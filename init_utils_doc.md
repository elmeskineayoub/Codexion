# Codexion — Init and Utils Module Documentation

`init.c` and `utils.c` — construction of the simulation, and the primitives
every other file depends on.

Neither file contains the interesting concurrency logic. Both are where subtle
bugs hide when they're wrong: a wrongly initialised timestamp makes every coder
burn out instantly; a wrongly typed `now_ms` silently truncates every timestamp;
an unguarded `log_state` prints lines after the burnout announcement. These two
files are the foundation the protocol stands on.

---

## 1. `init.c` — purpose

One job: **turn a `t_sim` holding validated raw numbers into a fully constructed
simulation, ready for threads to launch against.**

`parse_args` delivers numbers. `main` needs to call `pthread_create`. Between
those two points something has to allocate the dongle and coder arrays,
initialise every mutex and condition variable, create each dongle's waiter
heap, wire the circular left/right topology, and capture t=0. That is `init.c`.

It is the only place in the program where shared state is *constructed*, which
is exactly what makes cleanup reasonable to write: every resource has one
origin.

```
init.c   (3 functions written; join_all and cleanup_all still to be wired)
  init_sim        top-level: mutexes, start time, delegate to the two below
  init_dongles    allocate and initialise the dongle array
  init_coders     allocate and initialise the coder array, wire topology
```

---

## 2. `init.c` — line by line

### `init_sim`

```c
int	init_sim(t_sim *sim)
{
	sim->stop = 0;
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (1);
	sim->start_time = now_ms();
	if (init_dongles(sim) != 0)
		return (1);
	if (init_coders(sim) != 0)
		return (1);
	sim->coder_threads = malloc(sim->num_coders * sizeof(pthread_t));
	if (!sim->coder_threads)
		return (1);
	return (0);
}
```

**Precondition:** `parse_args` has already filled every numeric field
(`num_coders`, the five timings, `compiles_required`, `dongle_cooldown`,
`scheduler`). `init_sim` never reads `argv`; it only builds on what parse
delivered. An earlier version took `argc`/`argv` as parameters and never used
them — with `-Werror`, unused parameters are a hard error.

**Line by line:**

- `sim->stop = 0;` — the simulation is running. This flag is the single source
  of truth for "should every thread wind down?" It is only ever set to 1 by the
  monitor, and only ever read through `sim_stopped`.
- `pthread_mutex_init(&sim->stop_mutex, NULL)` — guards `sim->stop`. Every read
  and write goes through this mutex; otherwise the monitor writing while a coder
  reads is a data race (exercise 2's `counter++`, in miniature).
- `pthread_mutex_init(&sim->log_mutex, NULL)` — guards stdout. The subject:
  *"Logging must be serialized so that two messages never interleave on a single
  line (use a mutex to protect output)."*
- Both `pthread_mutex_init` calls are checked. They can fail (resource
  exhaustion, `EAGAIN`). An uninitialised mutex used later is undefined
  behaviour. Checking costs one line each.
- `sim->start_time = now_ms();` — **t=0 for the whole simulation.** Every log
  timestamp is `now_ms() - start_time`. Captured once, here, before any thread
  exists, so every thread measures against the same origin. Captured *after*
  the mutexes and *before* the dongles/coders because the dongles and coders
  are initialised relative to it (see below).
- `init_dongles(sim)` / `init_coders(sim)` — delegate. Order matters slightly:
  coders reference dongles by index, and both need `start_time`.
- `sim->coder_threads = malloc(...)` — the `pthread_t` array `launch_threads`
  will fill and `join_all` will iterate. Allocated last because it's the only
  thing here that nothing else depends on.
- Returns 0 on success, 1 on any failure — the project-wide convention.

**On the early returns:** each `return (1)` leaves whatever was already built
in place — it does *not* clean up. This is deliberate and safe **only because
`main` calls `cleanup_all(&sim)` on failure**, and `cleanup_all` is written to
tolerate partial construction (see section 5). If `init_sim` tried to clean up
itself on each path, it would have to know exactly how far it got, duplicating
the logic that already lives in `cleanup_all`.

### `init_dongles`

```c
int	init_dongles(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->num_coders;
	sim->dongles = malloc(n * sizeof(t_dongle));
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < n)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].taken = 0;
		sim->dongles[i].available_at = sim->start_time;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		sim->dongles[i].waiters = heap_create(n, sim->scheduler);
		if (!sim->dongles[i].waiters)
			return (1);
		i++;
	}
	return (0);
}
```

**Line by line:**

- `n = sim->num_coders;` — there are exactly as many dongles as coders. The
  subject: *"There are as many dongles as coders."* One dongle sits between each
  adjacent pair; in a circle of `n` coders there are `n` gaps.
- `sim->dongles = malloc(n * sizeof(t_dongle)); if (!sim->dongles) return (1);`
  — allocate the array. An earlier version was missing this line entirely and
  checked a pointer that had never been set — since `main` memsets `sim` to
  zero, `sim->dongles` was `NULL`, the check fired every run, and `init_sim`
  failed silently. The `if (!ptr)` guard being *present* made the missing
  `malloc` easy to overlook on re-reading.
- Per dongle:
  - `.id = i;` — its index, for logging and debugging.
  - `.taken = 0;` — nobody holds it.
  - `.available_at = sim->start_time;` — usable immediately at t=0. A dongle
    nobody has touched has no cooldown pending. Using `start_time` rather than
    `0` keeps every timestamp on the same reference frame; either would work
    since any `now >= available_at` comparison passes, but consistency matters
    when reading logs.
  - `pthread_mutex_init` / `pthread_cond_init` — the per-dongle lock and the
    wait queue's wake-up mechanism. **Return values are not checked here.** This
    is a known gap, defensible under time pressure but worth naming if asked:
    *"Mutex init failure inside the loop is unchecked; a production version
    would record the count of successful inits so cleanup knows how many to
    destroy."*
  - `.waiters = heap_create(n, sim->scheduler);` — this dongle's request queue.
    Capacity `n` is a safe upper bound (only two coders ever reference any given
    dongle, but `n` costs nothing and needs no justification). The scheduler is
    baked into the heap so `cmp_request` knows which policy to apply.
  - `if (!sim->dongles[i].waiters) return (1);` — `heap_create` returns `NULL`
    on allocation failure. Checked.

**The n=1 case:** the loop runs once, creates exactly one dongle. The subject:
*"If there is only one coder, there should be only one dongle on the table."*
No special-casing required — the general loop produces it.

### `init_coders`

```c
int	init_coders(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->num_coders;
	sim->coders = malloc(n * sizeof(t_coder));
	if (!sim->coders)
		return (1);
	i = 0;
	while (i < n)
	{
		sim->coders[i].id = i;
		sim->coders[i].left_dongle = i;
		sim->coders[i].right_dongle = (i + 1) % n;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].sim = sim;
		pthread_mutex_init(&sim->coders[i].state_mutex, NULL);
		i++;
	}
	return (0);
}
```

**Line by line:**

- `sim->coders = malloc(...); if (!sim->coders) return (1);` — allocate,
  check.
- Per coder:
  - `.id = i;` — 0-based internally. The subject's logs are 1-based (`1 has
    taken a dongle`), so `log_state` or its callers must print `id + 1`. Decide
    once, apply everywhere.
  - `.left_dongle = i;` and `.right_dongle = (i + 1) % n;` — **the circular
    topology.** Coder `i` sits between dongle `i` (its left) and dongle `i+1`
    (its right). The `% n` wraps the last coder's right dongle back to dongle 0,
    closing the circle. The subject: *"Coder number 1 sits next to coder number
    number_of_coders."* Modular arithmetic handles this without branching, for
    any `n`.
  - `.compile_count = 0;` — hasn't compiled yet.
  - `.last_compile_start = sim->start_time;` — **not zero.** The subject defines
    burnout from *"the beginning of their last compile or the beginning of the
    simulation."* Before any compile, the reference point is simulation start.
    If this were left at 0 (the Unix epoch, 1970), the monitor's first check
    would compute `now - 0 ≈ 1.7 trillion ms`, compare it to `time_to_burnout`,
    and declare every coder burned out instantly.
  - `.sim = sim;` — **the back-pointer.** `pthread_create` hands the thread
    function exactly one `void *`. The coder thread needs its own state *and*
    shared state (timings, stop flag, log mutex, the dongle array). Storing a
    pointer to `sim` inside each coder means passing `&sim->coders[i]` gives the
    thread everything through one argument.
  - `pthread_mutex_init(&sim->coders[i].state_mutex, NULL);` — guards
    `last_compile_start` and `compile_count`. The coder thread writes them (in
    `record_compile`); the monitor thread reads them (in `check_burnout` and
    `check_all_done`). Concurrent write and read without a lock is a data race.
    Per-coder rather than one global lock so coders never contend with each
    other on state updates.

**The n=1 case:** `right_dongle = (0 + 1) % 1 = 0`. Both hands need dongle 0.
Physically impossible to hold once dongle twice; the coder can never compile;
it burns out. This is the subject's stated n=1 behaviour, produced by the
general formula with no special case here — the *consequence* is handled in
`request_dongles`.

---

## 3. `init.c` — what's not written yet

**`launch_threads`** (planned for `main.c`, since `init.c` is at capacity):
loop `pthread_create` over `num_coders`, passing `&sim->coders[i]` to
`coder_routine`; then create the monitor thread passing `sim` to
`monitor_routine`. Return 1 on any failure.

**`join_all`**: `pthread_join` every coder thread, then the monitor. Every
thread must be joined before any mutex is destroyed, because a thread might
still be inside a locked section.

**`cleanup_all`**: the reverse of init, tolerant of partial construction. See
section 5.

---

## 4. `utils.c` — purpose

Five primitives every other file calls. Getting them right once here means
timing and logging behave identically everywhere, instead of being reinvented
with slight differences in `coder.c`, `dongle.c`, and `monitor.c`.

```
utils.c   (5 functions — at Norm cap)
  now_ms          current time in milliseconds
  precise_sleep   sleep without overshooting, interruptible by shutdown
  ms_to_abstime   duration → absolute timespec for timedwait
  log_state       one serialised, stop-guarded log line
  sim_stopped     the only way to read the stop flag
```

---

## 5. `utils.c` — line by line

### `now_ms`

```c
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
```

Returns the current wall-clock time in milliseconds since the Unix epoch.

**Line by line:**

- `struct timeval tv;` — two fields: `tv_sec` (whole seconds) and `tv_usec`
  (microseconds within the current second, 0–999999).
- `gettimeofday(&tv, NULL);` — fill it. The second argument is a timezone
  pointer, obsolete, always `NULL`. The subject explicitly recommends this call:
  *"real-time measurements using gettimeofday() are acceptable and recommended
  for simplicity."*
- `tv.tv_sec * 1000` — seconds to milliseconds.
- `tv.tv_usec / 1000` — microseconds to milliseconds, truncating.
- Sum them.

**Why the return type is `long`, not `int`:** the current epoch time in
milliseconds is around 1.7 × 10¹² — far beyond `INT_MAX` (≈ 2.1 × 10⁹). An
`int` return would silently truncate every call. An earlier version had this
bug. `tv.tv_sec` is itself a `long` on 64-bit Linux, so the multiplication
stays in `long` arithmetic.

**Why `(void)` in the parameter list:** in C, `int f()` means "takes an
unspecified number of arguments," which is different from "takes none." `(void)`
says explicitly: no arguments. The Norm requires it.

**On the return value of `gettimeofday`:** it returns 0 on success, -1 on
error. An earlier version stored this in an unused variable — dead code. It's
now ignored, which is standard: `gettimeofday` effectively cannot fail with a
valid pointer.

### `precise_sleep`

```c
void	precise_sleep(t_sim *sim, long ms)
{
	long	target;

	target = now_ms() + ms;
	while (now_ms() < target)
	{
		if (sim_stopped(sim))
			return ;
		usleep(300);
	}
}
```

Sleeps for `ms` milliseconds without overshooting, and returns early if the
simulation stops.

**Why not just `usleep(ms * 1000)`:** two reasons.

1. **Overshoot.** `usleep` guarantees you sleep *at least* the requested time;
   the OS wakes you when it gets around to it. A single long sleep can overshoot
   by milliseconds, which compounds across every phase of every coder and erodes
   the 10 ms burnout precision budget. Short sleeps in a loop, checking the real
   clock, keep the overshoot to one `usleep` granularity (~300 µs).

2. **Interruptibility.** A coder mid-way through a 500 ms compile sleep needs to
   notice that the monitor stopped the simulation. With one long `usleep`, it
   wouldn't notice until the sleep ended, delaying `pthread_join` and the
   program's exit. Checking `sim_stopped` every 300 µs means every thread winds
   down within a fraction of a millisecond of the stop flag being set.

**Line by line:**

- `target = now_ms() + ms;` — compute the absolute wake-up time once, up front.
  Recomputing it inside the loop would drift.
- `while (now_ms() < target)` — loop until the clock reaches the target.
- `if (sim_stopped(sim)) return ;` — bail out on shutdown.
- `usleep(300);` — 300 microseconds. Fine enough for precision, coarse enough
  not to spin the CPU.

**Why `ms` is `long`:** every duration in `t_sim` is `long`. An `int` parameter
would silently narrow when a caller passes `sim->time_to_compile`. The header
prototype and this definition must agree.

**The `return ;` with a space:** Norm requires a space before the semicolon on an
empty return.

### `ms_to_abstime`

```c
void	ms_to_abstime(struct timespec *ts, long ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += (long)(ms % 1000) * 1000000L;
	if (ts->tv_nsec >= 1000000000L)
	{
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000L;
	}
}
```

Fills `ts` with the absolute time `ms` milliseconds from now. This is exercise
6's `build_deadline`, renamed.

**Why it exists:** `pthread_cond_timedwait` takes an **absolute** deadline (a
`struct timespec` meaning "wake at this moment"), not a duration. Every timed
wait in the program needs "now + some ms" converted into that form.

**Line by line:**

- `clock_gettime(CLOCK_REALTIME, ts);` — fill `ts` with now. `CLOCK_REALTIME`
  is the wall clock, which is what the default cond-var clock uses. (Using
  `CLOCK_MONOTONIC` would be immune to wall-clock adjustments, but requires
  `pthread_condattr_setclock` on every cond var — not in the authorized list,
  and unnecessary for a short-running simulation.)
- `ts->tv_sec += ms / 1000;` — whole seconds. Integer division.
- `ts->tv_nsec += (long)(ms % 1000) * 1000000L;` — the leftover milliseconds,
  converted to nanoseconds (1 ms = 1 000 000 ns). An earlier version had `+`
  instead of `*` here — a bug that would have made every timed wait ~1 ms long
  regardless of the requested duration. It wouldn't crash, wouldn't warn, and
  would have manifested as dongle cooldowns that don't wait and deadlines that
  fire instantly.
- The carry:
  - `if (ts->tv_nsec >= 1000000000L)` — `tv_nsec` must stay in `[0,
    999 999 999]`. `clock_gettime` could have returned, say, 800 000 000 ns; adding
    300 000 000 more gives 1 100 000 000 — invalid.
  - `ts->tv_sec += 1; ts->tv_nsec -= 1000000000L;` — move the overflow into
    the seconds field. Exactly like carrying a digit.

**Why an invalid `timespec` matters:** `pthread_cond_timedwait` on a `timespec`
with `tv_nsec` out of range is undefined behaviour — some implementations
return `EINVAL`, some misbehave. The carry is a correctness requirement, not
tidiness.

**Why negative `ms` is not handled here:** C's `%` keeps the sign of the
dividend, so `-300 % 1000 == -300`, giving a negative `tv_nsec` contribution.
The callers (`wait_for_dongle`) clamp `ms` to zero before calling. Handling it
here too would be defensive duplication; the contract is "callers pass
non-negative."

### `log_state`

```c
void	log_state(t_sim *sim, int coder_id, char *state)
{
	if (sim_stopped(sim))
		return ;
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", now_ms() - sim->start_time, coder_id, state);
	pthread_mutex_unlock(&sim->log_mutex);
}
```

Prints one log line in the subject's format: `timestamp_in_ms X state`.

**Line by line:**

- `if (sim_stopped(sim)) return ;` — **the stop guard.** Once the simulation has
  stopped, nothing more is printed. This is how the burnout line becomes the
  last line of output, and how a coder that was mid-phase when burnout struck is
  prevented from logging "is debugging" afterwards.
- `pthread_mutex_lock(&sim->log_mutex);` — serialise. Without this, two coders
  calling `printf` simultaneously can interleave their output at the character
  level. The subject: *"A displayed state message should not be mixed up with
  another message."*
- `printf("%ld %d %s\n", now_ms() - sim->start_time, coder_id, state);` —
  timestamp relative to simulation start (so the first line is near 0, matching
  the subject's examples), coder id, the state string. `%ld` because the
  timestamp is `long`.
- `pthread_mutex_unlock(&sim->log_mutex);`

**Why `printf` and not `write`:** `printf` is in the authorized list, handles
the formatting, and — under the mutex — its buffering is harmless because the
whole line is assembled and flushed atomically from the caller's perspective.
The mutex is what guarantees atomicity, not the choice of function.

**On the `coder_id` parameter:** callers must pass the 1-based id the subject
expects. If internal ids are 0-based (they are), the call is
`log_state(sim, coder->id + 1, "is compiling")` — or the `+ 1` lives here.
Decide once. (Currently the `+ 1` is the caller's responsibility.)

**The ordering subtlety this creates — and how it was resolved:** the monitor
must log "burned out" and then set the stop flag. If it set the flag first, its
own burnout log call would hit this guard and print nothing — the one required
line would be silently swallowed. The correct sequence in `check_burnout` is:
`log_state(...burned out...)` **then** set `stop = 1`, back to back. The
subject's own precision box acknowledges that perfect ordering isn't
achievable: *"Allow a minimal tolerance when testing, as hardware and OS
scheduling may slightly affect measured timings."*

**Why the guard checks `sim_stopped` and not `sim->stop` directly:** `sim->stop`
is written by the monitor thread. Reading it without the mutex is a data race.
`sim_stopped` is the locked accessor.

### `sim_stopped`

```c
int	sim_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_mutex);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (val);
}
```

The only legal way to read the stop flag.

**Why a function for a one-field read:** `sim->stop` is written by the monitor
and read by every coder. Concurrent write and read of a plain `int` without
synchronisation is a data race — undefined behaviour, and exactly the pattern
Helgrind and ThreadSanitizer flag. Wrapping the read in the mutex makes it
well-defined.

**Why copy to `val` and return that, rather than `return (sim->stop)` inside the
lock:** the return statement would execute the unlock after evaluating
`sim->stop` — which is actually fine in this specific case. But the copy-out
pattern is the habit worth having: it makes it visually obvious that the lock
is released before the function returns, and it generalises to cases where the
returned expression is more complex.

**Cost:** this is called very frequently — every `precise_sleep` iteration
(every 300 µs per coder), every `acquire_one` loop iteration, every `log_state`.
Uncontended mutex lock/unlock on Linux is tens of nanoseconds. It's not a
bottleneck. Correctness first; this is not the place to optimise.

---

## 6. Cleanup — what `cleanup_all` must do, and why `memset` in `main` matters

`cleanup_all` is not yet written. When it is, it must be the exact reverse of
init, tolerant of partial construction:

```
cleanup_all(sim):
    for each dongle that exists:
        heap_destroy(dongle.waiters)      # handles NULL
        pthread_cond_destroy(&dongle.cond)
        pthread_mutex_destroy(&dongle.mutex)
    free(sim->dongles)                    # free(NULL) is a no-op
    for each coder that exists:
        pthread_mutex_destroy(&coder.state_mutex)
    free(sim->coders)
    free(sim->coder_threads)
    pthread_mutex_destroy(&sim->stop_mutex)
    pthread_mutex_destroy(&sim->log_mutex)
```

**The partial-init problem:** if `init_dongles` succeeds but `init_coders`'
`malloc` fails, `main` calls `cleanup_all`. `sim->coders` was never assigned.
Whatever garbage is in that stack slot gets passed to `free()` — a crash.

**The fix is in `main`, not here:** `memset(&sim, 0, sizeof(t_sim));` before
anything else. Now every pointer is `NULL` until genuinely allocated.
`free(NULL)` is defined as a no-op. `heap_destroy` checks for `NULL`. So
cleanup is safe at every point of partial construction.

**What `memset` does not solve:** `pthread_mutex_destroy` on a mutex that was
never initialised is undefined behaviour — it's not a pointer, so `NULL`
doesn't help. If `init_dongles` fails at dongle 3 of 5, dongles 0–2 have live
mutexes and 3–4 have zeroed bytes. Destroying 3–4 is UB. The clean solution is
to track how many succeeded (a counter in `t_sim`) and destroy only that many.
Under time pressure, a pragmatic alternative: `pthread_mutex_destroy` on
zeroed memory happens to be harmless on glibc (a zeroed mutex looks like a
statically initialised one), but that's an implementation detail, not a
guarantee. Know which one you're relying on.

**The `(t_sim){0}` alternative:** compound-literal assignment does the same
zeroing without `memset` or `<string.h>`. `memset` was kept because it's
explicitly in the subject's authorized list and needs no explanation in a
defense.

---

## 7. Anticipated defense questions

**"Why is `last_compile_start` initialised to `start_time` and not zero?"**
The subject counts burnout from the beginning of the last compile *or the
beginning of the simulation*. Zero would be the Unix epoch, and the monitor
would see ~1.7 trillion ms elapsed on its first check. Every coder would burn
out instantly.

**"Why `(i + 1) % n` for the right dongle?"**
It closes the circle. The last coder's right neighbour is coder 0; `% n` wraps
without an `if`. Works identically for any `n`, and for `n = 1` it correctly
gives `left == right`, which is the impossible-to-compile case the subject
describes.

**"Why does each coder have a pointer back to `sim`?"**
`pthread_create` passes one `void *`. The thread needs both its own coder
struct and the shared simulation. The back-pointer means `&sim->coders[i]` is
the only argument required.

**"Why is `now_ms` a `long`?"**
Epoch milliseconds exceed `INT_MAX` by three orders of magnitude. An `int`
would truncate every call.

**"Why loop `usleep(300)` instead of one `usleep`?"**
Precision and interruptibility. A single long sleep overshoots and can't
notice shutdown. Short sleeps against the real clock keep overshoot under a
millisecond and let every thread exit promptly.

**"Why does `log_state` check the stop flag?"**
So nothing prints after the burnout announcement. The monitor logs burnout
first, then sets the flag; every subsequent `log_state` call from any coder
hits the guard and prints nothing.

**"Why does `sim_stopped` take a lock for one integer?"**
Because the monitor writes it and coders read it concurrently. Unsynchronised
concurrent access to a plain `int` is a data race. The lock makes it defined.
Uncontended lock/unlock costs tens of nanoseconds; it's not a bottleneck.

**"What happens if `pthread_mutex_init` fails inside `init_dongles`?"**
Honest answer: it's unchecked in the loop. `init_sim`'s two mutexes are
checked; the per-dongle and per-coder ones are not. Under time constraints
that's a deliberate gap. A production version would check and track the
successful count so cleanup knows how many to destroy.

**"Why `memset` the sim struct in `main`?"**
So every pointer is `NULL` until allocated. That makes `cleanup_all` safe to
call after a partial `init_sim` failure — `free(NULL)` is a no-op, and
`heap_destroy` checks for `NULL`. Without the memset, cleanup would `free()`
whatever garbage was on the stack.

**"Why not use `CLOCK_MONOTONIC` in `ms_to_abstime`?"**
`pthread_cond_timedwait` uses `CLOCK_REALTIME` by default. Switching requires
`pthread_condattr_setclock`, which isn't in the authorized list. For a
simulation that runs for seconds, wall-clock adjustments aren't a practical
concern.