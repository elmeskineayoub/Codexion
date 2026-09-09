# Codexion — 5-Day Sprint Plan (9h/day deep work, 45h total)

**Starting point assumed:** theory done, `parse.c` done, pthread exercises done.
**Remaining scope:** structs, init, utils/time, heap, dongle protocol, coder,
monitor, main wiring, cleanup, adversarial testing, norm compliance, README,
defense prep.

Each task lists: **time box**, **purpose** (why this task exists, not just
what to type), and **resources** you may need. Tick each box only when the
"done when" condition is true — not when you feel like moving on.

---

## DAY 1 — Foundations (9h)

### 1.1 Finalize struct design on paper — 0:45
**Purpose:** every bug in days 2–3 traces back to an ambiguous struct. Locking
this down first means you're never redesigning mid-implementation.
**Do:** write final field lists for `t_dongle`, `t_coder`, `t_sim`. Decide
where the per-dongle request heap lives (should be per-dongle, see prior
discussion). Decide exactly what `t_sim` needs to hold the stop flag and its
mutex.
**Resources:** your notes from the earlier design-question session.
**Done when:** you can draw all three structs from memory without checking your notes.

### 1.2 Write `codexion.h` — 1:00
**Purpose:** turn the paper design into compilable types + prototypes for
every function you named across your 7 files. Doing this now means every
later file just fills in bodies — no more design decisions while coding.
**Do:** structs, enums (`e_state` for compile/debug/refactor, `e_scheduler`
for fifo/edf), all function prototypes.
**Done when:** it compiles standalone with `-Wall -Wextra` and no warnings.

### 1.3 `init.c` — 2:00
**Purpose:** get from parsed args to a fully-formed, ready-to-run `t_sim` —
this is the bridge between "I understand the problem" and "I have a program."
**Do:** `init_sim`, `init_dongles` (mutex/cond/state per dongle, circular
left/right assignment), `init_coders`, `join_all`, `cleanup_all` stub (fill
in fully on day 3).
**Resources:** `man pthread_mutex_init`, `man pthread_cond_init`.
**Done when:** you can print every coder's left/right dongle index and it
matches the circular rule for `n=1`, `n=2`, and `n=5`.

### 1.4 `utils.c` — 2:00
**Purpose:** these are the load-bearing primitives everything else calls —
get timing and logging bulletproof before anything threaded depends on them.
**Do:** `now_ms`, `precise_sleep` (loop in small increments, don't oversleep),
`ms_to_abstime` (reuse your exercise 6 solution, overflow-safe), `log_state`
(mutex-protected single `printf`), `sim_stopped` (mutex-protected read of the
stop flag).
**Resources:** exercise 6 hints file, `man clock_gettime`.
**Done when:** a standalone test calling `precise_sleep(500)` five times in a
row measures within ~1–2ms of 500ms each time.

### 1.5 `heap.c` — 2:00
**Purpose:** the arbitration mechanism the whole project depends on for
fairness — must be correct in isolation before you ever trust it inside
threads, because debugging a heap bug *and* a race bug simultaneously is how
people lose entire days.
**Do:** `heap_push`, `heap_pop`, `sift_up`, `sift_down`, `cmp_request` (branch
on fifo vs edf, deterministic tie-break for edf).
**Done when:** written — testing happens next task.

### 1.6 Standalone heap test harness — 1:00
**Purpose:** prove correctness now, cheaply, instead of discovering a heap bug
on day 8 while also debugging a hang.
**Do:** small `main()` (not submitted / kept outside repo or in `.gitignore`)
that pushes 1000+ random-priority requests and pops them, asserting sorted
order for both fifo and edf comparators.
**Done when:** zero assertion failures across several randomized runs.

### 1.7 Wrap-up / commit — 0:15
Commit. Write one sentence per file on what it does — you'll want this for
the README later.

---

## DAY 2 — Dongle protocol (9h) — the hard day, protect this time

### 2.1 Re-derive the acquisition protocol on paper — 0:30
**Purpose:** you already reasoned through deadlock-avoidance in the design
session — re-derive it fresh now, right before coding, so it's active in
your head instead of half-remembered.
**Do:** write, in plain language, the exact sequence of states a coder goes
through from "wants to compile" to "holding both dongles." Confirm your
chosen approach breaks at least one of Coffman's four conditions (hold-and-wait
is the natural one to target here).
**Resources:** OSTEP ch. 32 (Coffman's conditions), your notes.

### 2.2 `dongle.c` state functions — 1:30
**Purpose:** the dongle's own bookkeeping (free/taken, cooldown expiry) has to
be airtight before you build the coordination logic on top of it.
**Do:** availability check (accounts for cooldown), mark-taken, release +
set `available_at`.
**Done when:** each function only ever touches its own dongle's mutex-protected fields.

### 2.3 `acquire.c`-equivalent logic inside `dongle.c` — 3:00
**Purpose:** this is the actual concurrency core of the whole project —
request-enqueue, wait-until-granted, take-both-or-wait-for-both. Budget the
most careful, unhurried time of the whole sprint here.
**Do:** `request_dongles` (push onto both dongles' heaps), `wait_for_grant`
(cond_wait loop, re-checking real state, not just being signaled),
`take_dongles`, `grant_next` (called on release, pops the heap, signals the
winner).
**Resources:** exercises 4 and 5 (while-loop + broadcast vs signal reasoning
applies directly here).
**Done when:** you can explain out loud why two coders can never both believe
they hold the same dongle.

### 2.4 Cooldown + timedwait wiring — 1:30
**Purpose:** cooldown is a mandatory grading requirement and it's exactly the
`pthread_cond_timedwait` pattern from exercise 6 — wire it in deliberately,
don't bolt it on after.
**Done when:** a coder waiting on a dongle still in cooldown wakes up at (or
just after) `available_at`, not before, not indefinitely late.

### 2.5 Standalone dongle test harness (no coder threads yet) — 1:30
**Purpose:** isolate dongle-protocol bugs from coder-lifecycle bugs — same
logic as the heap harness on day 1.
**Do:** spawn a handful of raw threads that just request/hold/release dongles
in a tight loop with fake timings, print state changes, watch for anything
that looks wrong (never-granted requests, double-grants).
**Done when:** 60 seconds of chaotic concurrent requests produces no
impossible state in the log.

### 2.6 Debug / iterate buffer — 1:00
This day rarely goes exactly to plan. Use whatever's left here.

---

## DAY 3 — Coders, monitor, first full run (9h)

### 3.1 `coder.c` — 1:30
**Purpose:** wire the state machine (compile → debug → refactor → compile)
around the dongle protocol you already trust from day 2.
**Do:** `coder_routine`, `do_compile` (acquire → sleep time_to_compile →
release → record compile, update `last_compile_start`), `do_phase` (generic
sleep+log for debug/refactor), `record_compile` (mutex-protected).
**Done when:** compiles.

### 3.2 `monitor.c` — 1:30
**Purpose:** the subject's precision requirement (burnout logged within 10ms)
lives entirely in this file — get the polling interval right.
**Do:** `monitor_routine` (poll loop, ~200–500µs), `check_burnout` (mutex-
protected read of each coder's `last_compile_start`), `check_all_done`,
`stop_sim` (set flag + broadcast to every cond var coders might be waiting on).
**Resources:** exercise 5's broadcast reasoning — this is the exact case it
was training you for.
**Done when:** compiles.

### 3.3 `main.c` wiring — 1:00
**Purpose:** the glue — thread creation order and join order matter (monitor
usually starts last, joins... think about whether it should join before or
after coder threads, and why).
**Do:** `launch_threads`, tie parse → init → launch → join → cleanup together.

### 3.4 `cleanup.c` fully — 1:00
**Purpose:** partial-init-safe teardown, no leaks on either exit path
(burnout vs. all-compiles-reached).
**Do:** destroy every mutex/cond, free every allocation, guard against
destroying things that were never initialized.

### 3.5 First smoke test: trivial case — 1:00
**Purpose:** get *a* full run working before optimizing for correctness under
stress — confirms your wiring, not your edge cases, first.
**Do:** run `1 800 200 200 200 5 0 fifo`. It should burn out (1 coder can
never get 2 dongles — wait, careful: subject says with n=1 there's only ONE
dongle on the table total, so 1 coder truly cannot compile — confirm your
init handles n=1 correctly per the subject's special case).
**Done when:** clean burnout log within 10ms, program exits.

### 3.6 Multi-coder run + fix crashes/hangs — 2:00
**Purpose:** this is where day 1–2's isolated-testing investment pays off —
most bugs left should be integration bugs, not logic bugs.
**Do:** `5 800 200 200 100 5 0 fifo`, expect nobody burns out. Debug whatever
breaks.
**Resources:** `gdb` (`info threads`, `thread apply all bt`) for hangs.

### 3.7 Commit + log review — 1:00
Read your own log output line by line for one run. Does it match the
subject's expected format exactly? Any interleaved lines?

---

## DAY 4 — Hardening & the test matrix (9h)

### 4.1 Adversarial test matrix — 2:00
**Purpose:** prove correctness across the input space, not just the happy path.
**Do:** run each of these 5+ times:
- `1 800 200 200 200 5 0 fifo` (must burn out, n=1 special case)
- `5 800 200 200 100 5 0 fifo` (nobody dies)
- `4 410 200 200 0 10 0 edf` (tight — nobody dies under edf)
- `2 400 200 200 200 3 0 fifo` (burnout expected, verify 10ms precision)
- large `dongle_cooldown` (burnout is *correct*, verify it's still caught on time)
- `number_of_compiles_required` reached → clean self-exit
- garbage args: `0`, `-5`, `abc`, wrong arg count, `sjf` as scheduler

### 4.2 Valgrind leak-check — 1:00
**Purpose:** the subject explicitly fails non-freed memory — catch it now,
not during evaluation.
**Do:** `valgrind --leak-check=full --show-leak-kinds=all ./codexion ...` on
both a burnout exit and a compiles-reached exit.
**Done when:** "no leaks are possible" on both exit paths.

### 4.3 Helgrind + DRD — 1:30
**Purpose:** these catch race conditions your functional tests can't —
a program that "looks right" 50 times can still have a race.
**Do:** `valgrind --tool=helgrind` and `--tool=drd` on a mid-size run.
**Resources:** expect these to be slow — that's normal, they instrument every
memory access.

### 4.4 ThreadSanitizer — 1:00
**Purpose:** catches some races Helgrind misses, and is fast enough to run on
bigger, longer scenarios.
**Do:** build with `-fsanitize=thread -g` as a separate non-submitted target,
run your test matrix through it.

### 4.5 Fix whatever 4.2–4.4 found — 2:00
Budget this seriously — findings here are often the real bugs.

### 4.6 EDF tie-break + starvation demo — 1:00
**Purpose:** you need a concrete example for your README and defense showing
FIFO can starve someone while EDF doesn't (or your specific fairness
argument) — build it now while the system is fresh in your head.
**Do:** find/construct one parameter set demonstrating this, save the exact
command + log excerpt.

### 4.7 20x repeat of the tight cases — 0:30
**Purpose:** concurrency bugs are probabilistic — one clean run proves
nothing.
**Done when:** 20/20 clean on your tightest edf and fifo cases.

---

## DAY 5 — Norm, README, defense, buffer (9h)

### 5.1 Norm compliance pass — 1:30
**Purpose:** a norm error zeroes bonus AND mandatory — this is a hard gate,
not a nice-to-have.
**Do:** run norminette (or your campus's checker), fix violations (25-line
functions, 5-per-file, no globals, header guards, etc.).

### 5.2 README.md — 2:00
**Purpose:** required sections are graded content, not decoration — write
them from your day-1/day-2 design notes so they're accurate, not reconstructed.
**Do:** italic first line with logins, Description, Instructions, Resources
(classic refs + honest AI-usage description), **Blocking cases handled**
(deadlock prevention/Coffman, starvation prevention, cooldown handling,
precise burnout detection, log serialization), **Thread synchronization
mechanisms** (which primitive protects what, with a concrete race-prevented example).

### 5.3 Mock defense — explain design out loud — 1:00
**Purpose:** if you can't explain it verbally under mild pressure, you're not
ready — find the gaps now, alone or with a peer, not during evaluation.
**Do:** explain the acquisition protocol, why it avoids deadlock, how cooldown
works, how the monitor achieves 10ms precision, without looking at your code.

### 5.4 Recode drill practice — 1:00
**Purpose:** the subject explicitly warns a live modification may be
requested — rehearse making a small, contained change under time pressure.
**Do:** pick a plausible ask ("add a new state", "change the tie-breaker
rule", "log something extra") and actually do it against your real code in
under 10 minutes.

### 5.5 Final regression pass — 1:30
**Purpose:** last full sweep before you consider this done — catches anything
your recode drill or README pass might have disturbed.
**Do:** re-run the full day-4 test matrix once more end to end.

### 5.6 Buffer — 2:00
Reserved. Something in days 1–4 will have slipped into here — that's normal,
not failure. If everything went to plan, use this for extra 20x-repeat runs
or extra defense rehearsal.

---

## If you're falling behind

Cut scope in this order, never the reverse:
1. README polish (write it functional, not beautiful)
2. Number of repeat test runs (drop to 10x instead of 20x)
3. Extra defense rehearsal time

**Never cut:** valgrind/helgrind/tsan passes, or the n=1 and tight-timing test
cases. A race condition or a leak you didn't catch costs you far more than a
plain README does.
