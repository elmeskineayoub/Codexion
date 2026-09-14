# Codexion — Dongle Module Documentation

`dongle.c` and `dongle_utils.c` — the resource acquisition protocol.

This is the core of the project. Everything else (parsing, heaps, timing,
logging) exists to support what happens in these two files: coders competing
for shared dongles without deadlock, without starvation, with fair arbitration,
and with mandatory cooldown.

---

## 1. The problem these files solve

Restating the subject's constraints that this module must satisfy:

| Requirement | Where it's enforced |
|---|---|
| Each dongle protected by a mutex | `dongle->mutex`, locked in every access |
| Fair arbitration by FIFO or EDF | per-dongle heap, `cmp_request` |
| Dongle cooldown after release | `available_at`, checked in `is_available` |
| No deadlock | fixed-order acquisition in `request_dongles` |
| No starvation under EDF | EDF comparator + arrival tie-break |
| Coders don't communicate | each coder runs identical generic code, no cross-coder state |
| Clean shutdown on burnout | every wait loop checks `sim_stopped` |

---

## 2. The mental model

**A dongle is a shared resource with three pieces of state:**

```c
int   taken;         /* 0 = nobody holds it, 1 = someone holds it */
long  available_at;  /* timestamp: usable only once now >= this */
t_heap *waiters;     /* queue of coders waiting for THIS dongle */
```

plus a mutex guarding all three, and a condition variable for sleeping waiters.

**A coder needs two dongles simultaneously** — its left and its right, which are
shared with its two neighbours. This is the classic Dining Philosophers setup:
the risk is that every coder grabs one dongle and waits forever for the other.

**The protocol in one sentence:** a coder queues itself on each dongle it needs,
sleeps until it is both *front of that dongle's queue* and *the dongle is
available*, claims it, then does the same for the second dongle — always in a
fixed global order so a cycle of waiting can never form.

---

## 3. Layout and why it's split

```
dongle.c        (5 functions — at Norm cap)
  is_available       pure state query
  acquire_one        get ONE dongle, or abort on shutdown
  request_dongles    orchestrate getting BOTH, in safe order
  release_one        give back ONE dongle, start its cooldown, wake waiters
  release_dongles    give back BOTH

dongle_utils.c  (3 functions)
  build_request      construct a t_request for the heap
  wait_for_dongle    compute how long to sleep, then timedwait
  wait_single_dongle idle forever for the n=1 impossible case
```

The split exists because the Norm caps functions at 5 per file. The three helpers
are not `static` because they are called from `dongle.c` while living in
`dongle_utils.c` — a `static` function is invisible outside its own translation
unit, so it must be an ordinary externally-linked function with a prototype in
`codexion.h`.

---

## 4. `dongle_utils.c` — line by line

### `build_request`

```c
t_request	build_request(t_coder *coder, t_sim *sim)
{
	t_request	req;

	req.coder_id = coder->id;
	req.arrival = now_ms();
	req.deadline = coder->last_compile_start + sim->time_to_burnout;
	return (req);
}
```

**What it does:** packages the three fields the heap comparator needs into a
`t_request`.

**Line by line:**

- `req.coder_id = coder->id;` — identifies whose request this is. Used by the
  waiter to recognise itself at the front of the queue, and by `cmp_request` as
  the final unique tie-breaker.
- `req.arrival = now_ms();` — timestamp of the request. This is FIFO's primary
  key and EDF's secondary key.
- `req.deadline = coder->last_compile_start + sim->time_to_burnout;` — the
  moment this coder will burn out if it doesn't start compiling. This is EDF's
  primary key. The subject defines it exactly this way: *"deadline =
  last_compile_start + time_to_burnout."*
- `return (req);` — returned by value. `t_request` is three scalars; copying it
  is cheap, and the heap stores copies anyway.

**Why it exists as a separate function:** `acquire_one` was at 27 lines with the
three assignments inline. Extracting them brought it to exactly 25. That's the
honest reason — it is a Norm-driven extraction, not a conceptual one, though it
does make `acquire_one` read more clearly.

**Note on `last_compile_start`:** this is read without locking the coder's
`state_mutex`. That is acceptable here because the only writer of
`last_compile_start` is the coder's own thread (in `record_compile`), and
`build_request` is called from that same thread. A thread reading a value it
alone writes needs no lock. The monitor thread *also* reads this field, and
*that* read is locked — see `monitor.c`.

### `wait_for_dongle`

```c
void	wait_for_dongle(t_dongle *dongle, long deadline)
{
	long			wait_ms;
	struct timespec	ts;

	if (dongle->taken == 0)
		wait_ms = dongle->available_at - now_ms();
	else
		wait_ms = deadline - now_ms();
	if (wait_ms < 0)
		wait_ms = 0;
	ms_to_abstime(&ts, wait_ms);
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
}
```

**What it does:** decides how long the current coder should sleep, then sleeps
that long on the dongle's condition variable — waking early if signalled.

**Precondition:** the caller holds `dongle->mutex`. `pthread_cond_timedwait`
requires this; it atomically releases the mutex while sleeping and reacquires it
before returning.

**Line by line:**

- `if (dongle->taken == 0)` — the dongle is free but the coder still couldn't
  take it. There are two possible reasons: cooldown hasn't elapsed, or someone
  else is ahead in the queue. Either way, the natural wake-up target is the
  cooldown expiry:
  - `wait_ms = dongle->available_at - now_ms();` — sleep until the cooldown
    ends. If it's already ended (someone else is simply ahead in the queue),
    this is ≤ 0.
- `else` — the dongle is physically held by another coder. There's no
  cooldown target to wait for; the holder will broadcast on release. But a
  plain `cond_wait` with no timeout is dangerous (if the signal is missed, the
  coder sleeps forever), so bound the wait by something meaningful:
  - `wait_ms = deadline - now_ms();` — sleep at most until this coder's own
    burnout deadline. If it wakes up then and still has nothing, the monitor is
    about to catch it anyway.
- `if (wait_ms < 0) wait_ms = 0;` — **this clamp is a correctness requirement,
  not a nicety.** A negative `ms` fed to `ms_to_abstime` produces a negative
  `tv_nsec` (C's `%` keeps the dividend's sign), which is an invalid `timespec`.
  `pthread_cond_timedwait` on an invalid timespec is undefined behaviour; some
  implementations return `EINVAL`, others misbehave. Clamping to zero produces
  "wake immediately," which is exactly right — the target is already in the
  past, so re-check now.
- `ms_to_abstime(&ts, wait_ms);` — convert the *duration* into an *absolute
  deadline*. `pthread_cond_timedwait` takes absolute time, not relative.
- `pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);` — sleep. Wakes
  on: signal/broadcast, timeout, or spurious wakeup. The return value is
  deliberately ignored — whatever the reason for waking, the caller's `while`
  loop re-checks the real condition, so the reason doesn't matter.

**Why the wait is bounded rather than indefinite:** a bounded wait means a
missed broadcast, a scheduling hiccup, or a logic error elsewhere degrades to
"checks slightly late" instead of "hangs forever." Every sleep in this program
has a timeout for this reason.

**Why `wait_ms` might be zero when the dongle is free but someone else is
ahead:** the coder wakes immediately, re-checks, finds it's still not front,
and sleeps again with `wait_ms = 0`. This looks like a busy-loop, but it only
happens in the narrow window between "dongle became free" and "the true winner
claims it," which is microseconds — the winner was broadcast at the same
moment and is racing to lock the mutex. It resolves as soon as the winner sets
`taken = 1`, at which point the `else` branch applies and the wait becomes
deadline-bounded.

### `wait_single_dongle`

```c
int	wait_single_dongle(t_sim *sim)
{
	while (!sim_stopped(sim))
		usleep(300);
	return (1);
}
```

**What it does:** idles until the simulation stops. Used only for the n=1 case.

**Why it exists:** with one coder, `left_dongle == right_dongle == 0`. The coder
would need to hold the same single dongle in both hands simultaneously, which is
impossible. Rather than letting `acquire_one` run twice on the same dongle —
which would try to lock a mutex the thread already holds (self-deadlock on a
non-recursive mutex) and push the same coder twice onto one heap — the coder
simply waits. The monitor detects burnout after `time_to_burnout` ms and stops
the simulation. This is the subject's stated n=1 behaviour, produced without any
special-case code in `init_dongles` or `init_coders` — the general formula
`right = (i + 1) % n` yields it naturally, and this function handles the
consequence.

**Line by line:**

- `while (!sim_stopped(sim))` — poll the stop flag through the locked accessor.
- `usleep(300);` — 300 µs between checks. Fine-grained enough that the coder
  notices shutdown within a fraction of the 10 ms precision budget; coarse enough
  not to burn CPU.
- `return (1);` — returns "did not acquire," consistent with `acquire_one`'s
  failure return. The caller (`do_compile`) must treat this as "do not proceed
  to compile, do not release anything."

---

## 5. `dongle.c` — line by line

### `is_available`

```c
int	is_available(t_dongle *dongle)
{
	return (dongle->taken == 0 && now_ms() >= dongle->available_at);
}
```

**What it does:** answers "could this dongle be taken right now, ignoring the
queue?"

Two conditions, both required:
- `dongle->taken == 0` — nobody is holding it.
- `now_ms() >= dongle->available_at` — its cooldown has expired. This is where
  the subject's mandatory cooldown is enforced: *"after a coder releases a dongle,
  the dongle cannot be taken again until dongle_cooldown milliseconds have
  elapsed."*

**Precondition:** caller holds `dongle->mutex`. This function reads shared state
and does not lock, because its only call site (`acquire_one`) is already inside
the critical section.

**What it does NOT check:** whether the asking coder is front of the queue. That
is a separate condition combined at the call site. Keeping them separate makes
each predicate readable on its own.

### `acquire_one`

```c
int	acquire_one(t_dongle *dongle, t_coder *coder, t_sim *sim)
{
	t_request	req;
	t_request	front;

	pthread_mutex_lock(&dongle->mutex);
	req = build_request(coder, sim);
	heap_push(dongle->waiters, req);
	while (!sim_stopped(sim))
	{
		heap_peek(dongle->waiters, &front);
		if (is_available(dongle) && front.coder_id == coder->id)
		{
			heap_pop(dongle->waiters, &front);
			dongle->taken = 1;
			log_state(sim, coder->id, "has taken a dongle");
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		wait_for_dongle(dongle, req.deadline);
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}
```

**What it does:** obtains exactly one dongle for this coder, respecting the
scheduler's ordering and the cooldown. Returns 0 if acquired, 1 if the
simulation stopped before acquisition.

**Line by line:**

- `pthread_mutex_lock(&dongle->mutex);` — enter the critical section. Every
  read or write of `taken`, `available_at`, or the heap happens under this lock.
  This is the subject's *"you must protect each dongle's state with a mutex."*
- `req = build_request(coder, sim);` — construct this coder's claim.
- `heap_push(dongle->waiters, req);` — join the queue. From this moment the
  coder is a candidate; the heap orders it against other candidates by the
  scheduler's rule.
- `while (!sim_stopped(sim))` — **the predicate loop.** Every iteration re-checks
  the real condition. This is exercise 4's lesson: `pthread_cond_timedwait` can
  return for any reason (signal, timeout, spurious wakeup), so waking up proves
  nothing — only re-checking does. Using `if` instead of `while` would let a
  spurious wakeup or a broadcast meant for another coder grant a dongle the coder
  is not entitled to.
- `heap_peek(dongle->waiters, &front);` — non-destructively read the front of
  the queue. Peek, not pop: if the coder isn't the winner it must leave the
  queue intact.
- `if (is_available(dongle) && front.coder_id == coder->id)` — **the grant
  condition.** Both halves are necessary:
  - available alone isn't enough — a higher-priority coder may be ahead in the
    queue.
  - front-of-queue alone isn't enough — the dongle may still be held, or
    cooling down.
- Inside the grant branch:
  - `heap_pop(dongle->waiters, &front);` — remove self from the queue. Safe to
    use `pop` here (not an arbitrary remove) because the coder has just verified
    it *is* the front element. `front` is reused as the out-parameter purely to
    save a variable declaration; its contents are not needed afterwards.
  - `dongle->taken = 1;` — claim it. From this instant `is_available` returns
    false to everyone else.
  - `log_state(sim, coder->id, "has taken a dongle");` — the subject's log line.
    Placed here, at the moment of actual acquisition, not at request time. The
    subject's example output shows each "has taken a dongle" immediately before
    "is compiling," meaning it reflects a completed grant, not a queued request.
  - `pthread_mutex_unlock(&dongle->mutex);` then `return (0);` — leave the
    critical section holding the dongle.
- `wait_for_dongle(dongle, req.deadline);` — not granted; sleep. The mutex is
  released inside `timedwait` and reacquired before it returns, so the loop
  re-checks under the lock.
- After the loop: `pthread_mutex_unlock(&dongle->mutex); return (1);` — the
  simulation stopped while waiting. The coder's entry is left in the heap
  deliberately: every heap is about to be destroyed by `cleanup_all`, and no
  further arbitration will occur, so removing it would be wasted work (and would
  require an arbitrary-position `heap_remove` that has no other use).

**Why `req.deadline` is safe to read from the local copy:** `heap_push` took
`req` by value, so the heap holds a copy. Nothing modifies the local `req` after
construction. The local and the heap's copy are identical.

### `request_dongles`

```c
int	request_dongles(t_coder *coder, t_sim *sim)
{
	int	first;
	int	second;

	if (coder->left_dongle == coder->right_dongle)
		return (wait_single_dongle(sim));
	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first > second)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (acquire_one(&sim->dongles[first], coder, sim) != 0)
		return (1);
	if (acquire_one(&sim->dongles[second], coder, sim) != 0)
	{
		release_one(&sim->dongles[first], sim);
		return (1);
	}
	return (0);
}
```

**What it does:** obtains both of the coder's dongles, or none. Returns 0 if both
acquired, 1 otherwise. This is the deadlock-prevention layer.

**Line by line:**

- `if (coder->left_dongle == coder->right_dongle) return (wait_single_dongle(sim));`
  — the n=1 case. Explained above under `wait_single_dongle`. Must be checked
  *before* any acquisition attempt.
- `first = coder->left_dongle; second = coder->right_dongle;` — start with the
  natural order.
- `if (first > second) { swap them }` — **this is the deadlock prevention.**
  Whatever the coder's left and right happen to be, it always acquires the
  lower-numbered dongle first. Details below.
- `if (acquire_one(first) != 0) return (1);` — try the first dongle. If the
  simulation stopped during the wait, the coder holds nothing; return failure.
- `if (acquire_one(second) != 0)` — try the second. If this one fails, the coder
  is holding `first` but will never get `second`:
  - `release_one(&sim->dongles[first], sim);` — give back what it holds. Without
    this, a dongle would stay `taken = 1` forever after shutdown. It doesn't
    matter functionally at shutdown, but it keeps the invariant "a coder that
    returns failure holds nothing" true, which is what `do_compile` relies on.
  - `return (1);`
- `return (0);` — holding both.

**The invariant this function guarantees:** *on return 0, the coder holds both
dongles; on return 1, the coder holds neither.* `do_compile` depends on this
absolutely — it must not call `release_dongles` after a return of 1, because
there is nothing to release, and releasing dongles the coder never held would
corrupt another coder's state.

#### Why fixed-order acquisition prevents deadlock

Deadlock requires all four Coffman conditions simultaneously:

1. **Mutual exclusion** — a dongle can be held by one coder. Inherent; can't
   break this.
2. **Hold and wait** — a coder holds one dongle while waiting for another. True
   here; the coder does hold `first` while waiting for `second`.
3. **No preemption** — a dongle can't be forcibly taken. True here.
4. **Circular wait** — a cycle of coders each waiting for the next one's dongle.
   **This is the one that's broken.**

With naive left-then-right acquisition, a circular wait is possible: coder 0
takes dongle 0 and waits for dongle 1; coder 1 takes dongle 1 and waits for
dongle 2; ... coder n-1 takes dongle n-1 and waits for dongle 0. Every coder
holds one and waits for one held by its neighbour. Nobody can proceed.

With lowest-id-first acquisition, that cycle cannot form. Consider the coder
whose two dongles are `{n-1, 0}` (the wrap-around coder). Naively it would take
`n-1` first; under fixed ordering it takes `0` first. So there are now two coders
(coder 0 and coder n-1) both trying to take dongle 0 *first*, and only one can.
The loser holds nothing and is not part of any cycle. The chain is broken at the
wrap point.

More formally: if every thread acquires resources in a globally consistent
order, any thread waiting is waiting for a resource with a higher id than any it
holds. Follow the chain of "waits for": ids strictly increase. A strictly
increasing sequence over a finite set cannot cycle. No cycle, no deadlock.

**This is the single most important thing to be able to explain in the defense.**
It is the subject's *"Blocking cases handled"* section's central entry.

### `release_one`

```c
void	release_one(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->taken = 0;
	dongle->available_at = now_ms() + sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
```

**What it does:** returns one dongle to the table, starts its cooldown, and wakes
every coder waiting for it.

**Line by line:**

- `pthread_mutex_lock(&dongle->mutex);` — critical section.
- `dongle->taken = 0;` — no longer held.
- `dongle->available_at = now_ms() + sim->dongle_cooldown;` — **cooldown
  starts now.** Until this timestamp, `is_available` returns false regardless of
  `taken`. This is the entire implementation of the subject's mandatory cooldown.
- `pthread_cond_broadcast(&dongle->cond);` — wake everyone sleeping on this
  dongle. Broadcast, not signal: multiple coders may be queued (up to 2 by the
  topology argument). Each wakes, re-runs its `while` predicate, and only the one
  that is both front-of-queue and finds the dongle available proceeds. The rest
  go back to sleep. Using `signal` would wake an arbitrary one — possibly the
  wrong one — and the right one would stay asleep until its timeout, wasting the
  cooldown window. This is exercise 5's lesson applied at the per-dongle level.
- `pthread_mutex_unlock(&dongle->mutex);`

**Why there is no explicit "grant" step:** `release_one` does not pop the heap or
hand the dongle to anyone. It makes the dongle available and wakes the waiters.
Each waiter decides for itself, inside `acquire_one`, whether it has won. The
grant logic lives in exactly one place. An explicit `grant_next` that popped the
winner and signalled it would duplicate the predicate and introduce a window
where the popped coder isn't yet holding the dongle but is no longer in the queue.

**Why the broadcast happens while still holding the mutex:** this is standard
practice and avoids a subtle race. If the mutex were released before
broadcasting, a waiter could wake from a timeout, check, find nothing, and go
back to sleep *between* the unlock and the broadcast — missing the wake-up.
Broadcasting under the lock guarantees every waiter that is currently asleep
will be woken, and any waiter about to check will see the new state.

### `release_dongles`

```c
void	release_dongles(t_coder *coder, t_sim *sim)
{
	release_one(&sim->dongles[coder->left_dongle], sim);
	release_one(&sim->dongles[coder->right_dongle], sim);
}
```

**What it does:** returns both dongles.

**Why order doesn't matter here:** release never blocks. There's no waiting
involved, so no deadlock risk. Left-then-right is fine.

**Precondition:** the coder holds both dongles — i.e. `request_dongles` returned
0. Calling this after a return of 1 would set `taken = 0` on dongles the coder
never held, potentially freeing a dongle that another coder is actively using.
`do_compile` is responsible for checking.

**On n=1:** `left == right`, so this would call `release_one` twice on the same
dongle. That is harmless (each call locks, updates, broadcasts, unlocks
independently) but should never happen, because `request_dongles` returns 1 for
n=1 and `do_compile` therefore never reaches `release_dongles`.

---

## 6. The full lifecycle of one compile, traced

Coder 3 with `left = 3`, `right = 4`, five coders total.

1. `do_compile` calls `request_dongles(coder3, sim)`.
2. `first = 3, second = 4` (already ordered).
3. `acquire_one(dongle 3)`: lock, push request, loop. Suppose dongle 3 is free
   and coder 3 is front. Pop, `taken = 1`, log "3 has taken a dongle", unlock,
   return 0.
4. `acquire_one(dongle 4)`: lock, push request. Suppose coder 4 holds dongle 4.
   `is_available` false. `wait_for_dongle` — `taken == 1`, so wait until coder
   3's own deadline. Sleep.
5. Coder 4 finishes compiling, calls `release_dongles` → `release_one(dongle 4)`:
   `taken = 0`, `available_at = now + cooldown`, broadcast.
6. Coder 3 wakes (broadcast). Re-checks: `taken == 0` but `now < available_at`.
   `is_available` false. `wait_for_dongle` — `taken == 0`, so wait until
   `available_at`. Sleep.
7. Cooldown expires; timedwait times out. Re-check: `is_available` true, front is
   coder 3. Pop, `taken = 1`, log "3 has taken a dongle", unlock, return 0.
8. `request_dongles` returns 0. `do_compile` logs "3 is compiling", sleeps
   `time_to_compile`, calls `release_dongles`, records the compile.

---

## 7. What is deliberately NOT here

**No `heap_remove`.** The only exit from a queue without a grant is shutdown, at
which point the heap is destroyed. An orphaned entry is harmless.

**No per-coder "which dongle am I waiting on" state.** The coder's thread is
blocked inside `acquire_one`; its stack knows. Nothing else needs to.

**No inter-coder communication.** Each coder runs this identical code with no
knowledge of any other coder's state. The subject requires this. The alternating
compile pattern that emerges with even `n` (coders 0, 2, 4 compile while 1, 3
wait) is a consequence of shared dongles and mutual exclusion — not something
any code orchestrates.

---

## 8. Anticipated defense questions

**"How do you prevent deadlock?"**
Fixed-order acquisition. Every coder takes its lower-numbered dongle first. This
breaks the circular-wait Coffman condition: a chain of waiting coders would need
strictly increasing dongle ids, which cannot cycle. See section 5,
`request_dongles`.

**"How do you prevent starvation under EDF?"**
The heap orders waiters by deadline, so the coder closest to burnout is always
served first. Ties fall back to arrival time (who asked first), then coder id, so
no coder is systematically deprioritised. Combined with fixed-order acquisition
preventing deadlock, every waiting coder is eventually front-of-queue.

**"How is cooldown enforced?"**
`release_one` sets `available_at = now + cooldown`. `is_available` refuses until
`now >= available_at`. A waiting coder's `wait_for_dongle` sleeps exactly until
`available_at` if the dongle is free-but-cooling, so it wakes at the right
moment rather than polling.

**"Why `while` and not `if` around the wait?"**
`pthread_cond_timedwait` returns on signal, timeout, or spurious wakeup. Waking
proves nothing. The predicate must be re-evaluated every time. With `if`, a
coder woken by a broadcast intended for a different waiter would take a dongle it
is not entitled to.

**"Why broadcast instead of signal on release?"**
Up to two coders can be queued on a dongle. `signal` wakes one, chosen
arbitrarily. If it's not the front-of-queue coder, the real winner stays asleep
until its timeout. `broadcast` wakes both; each re-checks; only the true winner
proceeds.

**"What happens with one coder?"**
`left == right`. The coder cannot hold the same dongle twice, so it waits in
`wait_single_dongle` until the monitor detects burnout. No special-casing in
init — the general formula produces this, and one guard in `request_dongles`
handles the consequence.

**"What if the simulation stops while a coder holds one dongle and is waiting
for the second?"**
`acquire_one` for the second returns 1. `request_dongles` calls `release_one`
on the first, then returns 1. The coder holds nothing. `do_compile` sees the 1
and does not attempt to release anything.

**"Why is `is_available` not locked?"**
Its only caller already holds the dongle's mutex. Locking again would either
deadlock (non-recursive mutex) or require a recursive mutex for no benefit.

**"Why bounded waits everywhere instead of `pthread_cond_wait`?"**
A missed signal, scheduler hiccup, or logic error elsewhere would leave an
unbounded wait hung forever. Bounded waits degrade to "checks slightly late."
Every sleep in the program has a timeout for this reason.

**"Where does the 'has taken a dongle' log fire, and why there?"**
At the moment of actual acquisition inside `acquire_one`, after `taken = 1`.
The subject's example shows both "has taken a dongle" lines immediately
preceding "is compiling," which means they reflect completed grants. Logging at
request time would produce timestamps that don't correspond to holding anything.

---

## 9. What `coder.c` must respect (tomorrow's checklist)

- `do_compile` calls `request_dongles`; if it returns non-zero, **stop** — no
  "is compiling" log, no compile sleep, no `release_dongles`.
- Only after `request_dongles` returns 0: log "is compiling", sleep, then
  `release_dongles`, then `record_compile`.
- `record_compile` updates `last_compile_start` and `compile_count` under
  `state_mutex`, because the monitor reads both.
- `last_compile_start` must be set to the time compiling **started**, not
  ended — the subject defines burnout from *"the beginning of their last
  compile."* Capture the timestamp before the compile sleep, write it in
  `record_compile` after.
- The coder loop is: request → compile → release → debug → refactor → repeat.
  Check `sim_stopped` between phases so the thread exits promptly on shutdown.