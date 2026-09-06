# Thread Exercises — pthread warm-up before Codexion

Do these in order: 1 -> 2 -> 3 -> 4 -> 5 -> 6.
Don't skip ahead even if one looks trivial — the point of each is to see a specific
failure with your own eyes, not just to make it compile.

## How to use each folder

Each folder has:
- `exN.c` — a skeleton with `// TODO` markers and a `PREDICT:` comment where you
  must write down what you expect BEFORE running.
- `HINTS.md` — read only after you've tried for real. It has 3 levels:
  level 1 (nudge) -> level 2 (bigger nudge) -> level 3 (near-answer).
  Stop at the first level that unblocks you.

## Compiling

All of these need `-pthread`:

```
cc -Wall -Wextra -pthread exN.c -o exN
./exN
```

Run each one **at least 5 times** before moving on — nondeterministic bugs don't
show up on the first run. That inconsistency IS the lesson.

## What to bring back to the mentor session

For each exercise, be ready to explain, in your own words:
- ex1: why passing `&i` from a loop is broken
- ex2: why `counter++` is not atomic, and what the actual wrong output looked like
- ex3: what a mutex left locked (forgotten unlock) looks like from the outside
- ex4: why `while (!ready)` is required instead of `if (!ready)`
- ex5: signal vs broadcast — how many threads woke up in each case, and why that
  matters for Codexion's stop-flag
- ex6: what happens if you get the abstime math wrong (tv_nsec overflow)

Don't just paste working code — paste your reasoning. That's what's being tested.
