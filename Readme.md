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
2 1 has taken a dongle
2 1 is compiling
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
- `man` pages: `pthread_create`, `pthread_mutex_init`, `pthread_cond_wait`,
  `pthread_cond_timedwait`, `clock_gettime`, `gettimeofday`.
- Wikipedia — Earliest Deadline First scheduling; Coffman's conditions for
  deadlock.

**How AI was used**

Claude (Anthropic) was used throughout this project as a learning aid and
code reviewer, in a mentor-style workflow: I wrote the code myself, and Claude
questioned, reviewed, and corrected it rather than writing it from scratch.

## Blocking cases handled

**Deadlock prevention (Coffman's conditions).** Compiling requires two
dongles held simultaneously — a direct opportunity for the classic Dining
Philosophers deadlock, where every coder holds one dongle and waits forever
for its neighbour's. This is prevented by breaking the *circular wait*
condition: a coder always acquires its **lower-numbered** dongle first,
regardless of which one is its "left" or "right". If every thread acquires
shared resources in a single globally consistent order, the chain of "who is
waiting for whom" can only ever increase in dongle id, and a strictly
increasing sequence over a finite set cannot cycle back on itself. No cycle,
no deadlock.

*Known trade-off:* this acquisition order still allows **hold-and-wait** (a
coder holds its first dongle while waiting for its second), which under large
`dongle_cooldown` values can waste dongle availability and, in tightly
parameterised scenarios, cost a coder its burnout deadline. Eliminating it
would require all-or-nothing acquisition of both dongles under a single
decision, which introduces its own head-of-line-blocking trade-off. This
project favours guaranteed deadlock freedom via fixed ordering.

**Starvation prevention.** Each dongle owns its own hand-written priority
queue (a binary heap; no standard library structure is used). Under `fifo`,
requests are served by arrival time; under `edf`, by earliest burnout
deadline (`last_compile_start + time_to_burnout`). Both policies fall back to
arrival time and finally to a coder's unique id to break ties deterministically,
exactly as required for reproducible EDF behaviour when timestamps collide.
Because EDF always prioritises the coder closest to burnout, no coder can be
indefinitely outrun by others under a feasible parameter set.

**Cooldown handling.** Every dongle stores an `available_at` timestamp, set
to `now + dongle_cooldown` at the moment of release. A dongle is only
considered available once `now >= available_at`, so it cannot be re-acquired
before its cooldown elapses — enforced inside the same critical section that
checks whether the dongle is currently held.

**Precise burnout detection.** A dedicated monitor thread polls every coder's
state every 300 microseconds — roughly 33 times inside the subject's 10 ms
precision budget — comparing `now - last_compile_start` against
`time_to_burnout`. `last_compile_start` is reset at the **start** of a
compile, not its end, matching the subject's definition of the deadline.

**Log serialization.** A single mutex protects every `printf` call across all
threads, guaranteeing no two log lines can ever interleave. A guard inside
the logging function additionally refuses to print anything once the
simulation has stopped, which is what makes the burnout announcement
reliably the last line of output: the monitor logs it, then sets the stop
flag — never the reverse, since the guard would otherwise silently suppress
its own message.

## Thread synchronization mechanisms

**`pthread_mutex_t`, one per dongle**, protects that dongle's `taken` flag,
`available_at` cooldown timestamp, and its waiter heap. Every read or
modification of these fields happens inside this lock, preventing two coders
from ever believing they simultaneously hold the same dongle — the core race
this project must avoid.

**`pthread_cond_t`, one per dongle**, lets a waiting coder sleep instead of
busy-polling. Coders wait with `pthread_cond_timedwait`, bounded either by the
dongle's cooldown expiry or by the coder's own burnout deadline, so a missed
wake-up degrades to "checks slightly late" rather than hanging forever. Every
wait sits inside a `while` loop re-checking the real condition (dongle
available **and** this coder is at the front of its queue) rather than an
`if`, since a `pthread_cond_wait`/`timedwait` can return for reasons unrelated
to this specific coder's turn (a broadcast meant for another dongle's event,
or a spurious wakeup).

On release, `pthread_cond_broadcast` (not `signal`) wakes every coder queued
on that dongle, since more than one coder can be waiting on it; each
re-evaluates its own predicate and only the true front-of-queue winner
proceeds. The same reasoning, applied simulation-wide, is why the monitor's
shutdown routine broadcasts on **every** dongle's condition variable in turn —
each dongle has its own condition variable, so a coder asleep on dongle 3 is
never woken by an event on dongle 0.

**Per-coder `pthread_mutex_t` (`state_mutex`)** protects `compile_count` and
`last_compile_start`, written by the coder's own thread and read
concurrently by the monitor thread. Without this lock, the read/modify/write
sequence behind updating these fields would be a textbook data race — the
same class of bug as an unprotected `counter++` across threads, but with the
consequence of a missed or phantom burnout detection instead of a wrong
number.

**A dedicated `stop` flag protected by its own `pthread_mutex_t`** is the
project's "custom event": every thread — coder and monitor alike — checks it
through a single locked accessor before proceeding through any phase or wait.
Setting it and broadcasting on every dongle together form the shutdown
sequence: every coder, wherever it is sleeping, is guaranteed to wake, notice
the flag, and unwind cleanly, which is what allows `pthread_join` to complete
for every thread without ever hanging.

**A separate `pthread_mutex_t` around all logging** is the mechanism behind
log serialization described above — the only shared resource every thread in
the program touches, made safe by a single lock around the `printf` call
itself.