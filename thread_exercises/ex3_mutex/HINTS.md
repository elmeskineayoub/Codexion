# Exercise 3 hints

## Level 1
A mutex guarantees only one thread is between its `lock` and `unlock` at a
time. So put the lock immediately before the read-modify-write, and the
unlock immediately after — nothing else should be inside that critical
section (keep critical sections as short as possible, this matters a lot
once you have 5+ dongles all contending).

## Level 2
```c
pthread_mutex_lock(&counter_lock);
counter++;
pthread_mutex_unlock(&counter_lock);
```
inside the loop, INCREMENTS times.

For part B: if a thread locks and then returns/exits without unlocking, that
mutex is locked forever. Every other thread that later tries to lock it will
block forever too. Your program won't crash — it'll just hang. `ps` will
show the process still running, `gdb -p <pid>` then `thread apply all bt`
will show most threads sitting inside `pthread_mutex_lock`.

## Level 3
This exact failure mode — one thread holds a lock and never releases it
because of an early return, a missed code path, or an exception-like early
exit — is why in Codexion you should think carefully about EVERY exit path
out of a locked section, including the one where the simulation stop-flag
gets set while a coder is mid-acquisition. If you ever have more than one
`return` or early exit possible while a mutex is held, that's a red flag —
restructure so unlock always happens (or use fewer early exits).
