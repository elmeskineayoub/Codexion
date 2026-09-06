# Exercise 2 hints

## Level 1
`counter++` looks like one instruction. It isn't. It's three:
1. read `counter` from memory into a register
2. add 1 to the register
3. write the register back to memory

If two threads interleave those three steps, one thread's write can get
silently overwritten by the other's stale read. That's why the final total
is usually LESS than 4,000,000, and different every run.

## Level 2
This is exactly what will happen to a coder's `compile_count` or a dongle's
`state` field in Codexion if two threads touch it without a mutex around the
whole read-modify-write sequence — not just around the write.

Try compiling with `cc -fsanitize=thread -pthread ex2.c -o ex2_tsan` and
running `./ex2_tsan`. ThreadSanitizer will print a data race report pointing
at the exact line. Read that report closely — this is the same tool and the
same kind of report you'll be debugging in Codexion.

## Level 3
No code fix needed for exercise 2 itself — the whole point is to observe the
bug and explain it. The fix (adding a mutex) is exercise 3. If you're stuck
understanding WHY it's wrong rather than fixing it: run it under
`-fsanitize=thread` and paste the first race report to the mentor session,
we'll read it together.
