*This project has been created as part of the 42 curriculum by aelmeski.*

# Codexion
![Codexion Banner](./assets/Banner_Codexion.jpg)
## Description

Codexion is a C simulation of concurrent resource sharing under real-time
constraints — a variant of the classic Dining Philosophers problem. A fixed
number of **coders** sit in a circular co-working hub. Each coder cycles
through three phases: **compile**, **debug**, **refactor**. Compiling requires
holding two **USB dongles** simultaneously — the ones shared with the coder's
left and right neighbours — and there are exactly as many dongles as coders.

The goal is to let every coder compile regularly without ever **burning out**
(failing to start a compile within `time_to_burnout` milliseconds of its last
one), while enforcing:

- a mandatory **cooldown** after each dongle release,
- **fair arbitration** between competing coders, chosen at launch as either
  **FIFO** (first request served first) or **EDF** (earliest burnout deadline
  served first),
- a strict prohibition on any coder knowing about any other coder's state.

Each coder is a POSIX thread. A separate **monitor thread** watches for
burnout and for simulation completion (every coder having compiled at least
`number_of_compiles_required` times), and is responsible for a clean,
race-free shutdown of every thread.

## Instructions

### Compilation

```bash
make          # builds the `codexion` program
make clean    # removes object files
make fclean   # removes object files and the binary
make re       # fclean + all
```

Compiles with `-Wall -Wextra -Werror -pthread`, no warnings.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning | Unit |
|---|---|---|
| `number_of_coders` | number of coder threads and dongles | count (≥ 1) |
| `time_to_burnout` | max idle time before a coder burns out | ms |
| `time_to_compile` | duration of the compile phase | ms |
| `time_to_debug` | duration of the debug phase | ms |
| `time_to_refactor` | duration of the refactor phase | ms |
| `number_of_compiles_required` | compiles needed per coder to stop the sim | count |
| `dongle_cooldown` | unavailable time after a dongle is released | ms |
| `scheduler` | arbitration policy | `fifo` or `edf` |

Example:

```bash
./codexion 5 2000 200 200 200 10 0 fifo
```

All arguments are mandatory. Negative numbers, non-integers, and any
scheduler other than `fifo`/`edf` are rejected with an error on stderr and a
non-zero exit code. `number_of_coders` must be strictly positive; every other
numeric argument accepts zero, since the subject only requires rejecting
negatives, non-integers, and invalid scheduler strings.

### Output

Every state change is logged as `timestamp_in_ms coder_id action`, e.g.:

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
```

The simulation stops either when a coder burns out (its log line is
guaranteed to be the last one printed) or once every coder has reached
`number_of_compiles_required` compiles.

## Resources

**Concurrency theory and POSIX threads:**
-  *Unix Threads in C playlist* (CodeVault)
    https://youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&si=DJUQMvFtMTyuAWVV
- *Extreme_C Taking You To The Limit In Concurrency,OOP* (Kamran Amini)
- *Operating Systems: Three Easy Pieces* (Remzi Arpaci-Dusseau), chapters on
  concurrency, locks, and condition variables.
- *The Linux Programming Interface* (Michael KerrisK) — the Linux programming interface—the system calls, library
functions, and other low-level interfaces provided by Linux, a free implementation
of the UNIX operating system.
- `man` pages: `pthread_create`, `pthread_join`, `pthread_mutex_init`,
  `pthread_mutex_lock`, `gettimeofday`, `usleep`.
- Wikipedia — Earliest Deadline First scheduling; Coffman's conditions for
  deadlock.

**How AI was used**

Claude (Anthropic) was used throughout this project as a learning aid and
code reviewer, in a mentor-style workflow: I wrote the code myself, and Claude
questioned, reviewed, and corrected it rather than writing it from scratch.

## Blocking cases handled

**Deadlock prevention (Coffman's conditions).** Compiling requires two
dongles held simultaneously — a direct opportunity for the classic Dining
Philosophers deadlock, where every coder grabs one dongle and then waits
forever for the one its neighbour is holding. Deadlock requires all four
Coffman conditions at once (mutual exclusion, no preemption, hold-and-wait,
circular wait); breaking any single one makes it impossible. This
implementation breaks **two of them, independently**.

*First, hold-and-wait is eliminated by all-or-nothing acquisition.* A coder
queues its request on both of its dongles' waiter heaps, then loops: if it can
take **both** — both free, cooldown elapsed on both, and it is first in line
on both — it claims both inside a single critical section. If it cannot, it
**releases both mutexes**, backs off for 200 µs, and retries from scratch. A
coder therefore never holds one resource while blocked waiting for another, so
it can never be a link in a "holds X, waits for Y" chain, and no such chain can
form a cycle.

```c
while (!sim_stopped(sim))
{
    if (can_take_both(coder, sim))
    {
        take_both(coder, sim);        /* claims BOTH, or neither */
        unlock_both(sim, a, b);
        return (0);
    }
    unlock_both(sim, a, b);           /* could not -> release everything */
    usleep(200);
    lock_both(sim, a, b);             /* and try again */
}
```

*Second, circular wait is eliminated by a strict global lock order.*
`order_dongles` always places the lower dongle id in `a` and the higher in `b`,
and `lock_both` always locks `a` before `b`. Every thread in the program
therefore acquires dongle mutexes in the same ascending direction. A chain of
"who waits for whom" would require strictly increasing dongle ids, and a
strictly increasing sequence over a finite set cannot return to its start.
Concretely, with 5 coders: coder 1 needs dongles 0 and 1 and locks 0 first;
coder 5 needs dongles 4 and 0 and — instead of locking 4 first — also locks
**0 first**. The wrap-around that would close the ring never happens.

**Starvation prevention.** Each dongle owns its own hand-written priority
queue (a binary heap; no standard library structure is used). Under `fifo`,
requests are served by arrival time; under `edf`, by earliest burnout
deadline (`last_compile_start + time_to_burnout`). Both policies fall back to
arrival time and finally to a coder's unique id to break ties deterministically,
exactly as required for reproducible EDF behaviour when timestamps collide.
Because EDF always prioritises the coder closest to burnout, no coder can be
indefinitely outrun by others under a feasible parameter set. The arrival
timestamp used for both heaps is captured once, at the moment a coder begins
trying to compile — not separately per dongle — so a coder's queue position
reflects when it actually wanted to compile rather than an artifact of
acquisition order.

**Cooldown handling.** Every dongle stores an `available_at` timestamp, set
to `now + dongle_cooldown` at the moment of release. A dongle is only
considered free once `now >= available_at` **and** `taken == 0` — both checked
together in `is_free`, under that dongle's own mutex, as part of every
acquisition attempt. A dongle therefore cannot be re-taken before its cooldown
elapses, regardless of which coder is trying.

**Precise burnout detection.** A dedicated monitor thread polls every coder's
state every 300 microseconds — roughly 33 times inside the subject's 10 ms
precision budget — comparing `now - last_compile_start` against
`time_to_burnout`. `last_compile_start` is reset at the **start** of a
compile, not its end, matching the subject's definition of the deadline. A
burnout on an 800 ms deadline is reported at 801 ms.

**Log serialization.** A single mutex protects every `printf` call across all
threads, guaranteeing no two log lines can ever interleave. A guard inside
the logging function additionally refuses to print anything once the
simulation has stopped, which is what makes the burnout announcement
reliably the last line of output: the monitor logs it, then sets the stop
flag — never the reverse, since the guard would otherwise silently suppress
its own message.

## Thread synchronization mechanisms

**`pthread_mutex_t`, one per dongle**, protects that dongle's `taken` flag,
its `available_at` cooldown timestamp, and its waiter heap. Every read or write
of these fields happens inside this lock, so no coder can ever observe or
create an inconsistent view of a dongle's state. This is also the lock that
makes acquisition atomic: `lock_both` holds *both* of a coder's dongle mutexes
across the entire check-and-claim sequence, so `can_take_both` (the test) and
`take_both` (the action) cannot be interleaved by another coder's decision —
closing the classic check-then-act race.

**A strict global lock ordering** is what makes holding two mutexes at once
safe. `order_dongles` guarantees `a < b`, and `lock_both`/`unlock_both` always
lock `a` then `b` and unlock `b` then `a`. Because every thread follows the
same direction, the mutexes themselves can never deadlock against each other.
`lock_both` also guards the `n = 1` case with `if (b != a)`, since locking the
same non-recursive mutex twice from one thread would self-deadlock.

**Bounded backoff instead of condition variables.** This implementation uses
no `pthread_cond_t`. The subject permits one — *"a condition variable may be
used to manage waiting queues"* — but does not require it, and with
all-or-nothing acquisition there is no single event to signal: a waiting coder
depends on the combined state of **two** dongles plus **two** cooldown timers,
and a cooldown expiring is not an event any thread can broadcast — it is simply
the passage of time, which somebody has to re-check regardless. A coder that
cannot proceed therefore releases both mutexes, sleeps 200 µs, and re-evaluates
the full condition from scratch. This removes an entire class of missed-wakeup,
lost-signal and spurious-wakeup bugs, and 200 µs is fifty times finer than the
10 ms precision budget the subject sets.

**Per-coder `pthread_mutex_t` (`state_mutex`)** protects `compile_count` and
`last_compile_start`, written by the coder's own thread and read
concurrently by the monitor thread. Without this lock, the read/modify/write
sequence behind updating these fields would be a textbook data race — the
same class of bug as an unprotected `counter++` across threads, but with the
consequence of a missed or phantom burnout detection instead of a wrong
number. Both fields are updated inside the same critical section because the
monitor reads them together; splitting them would let it observe an
incremented count alongside a stale start time.

**A dedicated `stop` flag protected by its own `pthread_mutex_t`
(`stop_mutex`)** is the project's "custom event". Every thread — coder and
monitor alike — reads it exclusively through the locked accessor
`sim_stopped()`; the flag is never touched directly anywhere in the codebase.
Because no thread ever sleeps on an unbounded wait, no wake-up signal is
needed at shutdown: the monitor simply sets the flag, and every thread
observes it at its next check — within 200 µs inside the acquisition loop, and
within 300 µs inside `precise_sleep`, which re-tests the flag between
increments rather than sleeping through a phase in one call. A coder that exits
mid-acquisition holds nothing (all-or-nothing acquisition guarantees it never
completed a partial claim), so it simply releases its mutexes and returns. This
chain is what allows `pthread_join` to complete for every thread without ever
hanging.

**A separate `pthread_mutex_t` around all logging** is the mechanism behind
log serialization described above — the only shared resource every thread in
the program touches, made safe by a single lock around the `printf` call
itself, which appears exactly once in the entire project.
