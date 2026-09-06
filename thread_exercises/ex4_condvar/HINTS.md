# Exercise 4 hints

## Level 1
`pthread_cond_wait(&cond, &lock)` does three things atomically: unlocks
`lock`, sleeps until signaled, then re-locks `lock` before returning. You
must hold the lock when you call it, and you'll hold it again when it
returns.

## Level 2
With only one producer and one consumer and no other threads touching
`ready`, using `if (!ready)` instead of `while (!ready)` will probably
still work correctly. That's exactly the trap — it LOOKS fine here.

The reason `while` is the only correct version: `pthread_cond_wait` can
return even when nobody called signal/broadcast (a "spurious wakeup" —
allowed by POSIX on some platforms/schedulers) AND, more importantly for
Codexion, when there are MULTIPLE waiters, a thread can wake up, find
`ready` already consumed/changed by a faster thread, and needs to re-check
the condition and go back to sleep if it's not actually true for it.

## Level 3
```c
pthread_mutex_lock(&lock);
while (!ready)
    pthread_cond_wait(&cond, &lock);
printf("consumer: got it\n");
pthread_mutex_unlock(&lock);
```

The rule to internalize: `pthread_cond_wait` is ALWAYS wrapped in a `while`
loop that re-checks the real condition, never an `if`. In Codexion, "the
condition" will be things like "am I actually granted this dongle now AND
has its cooldown expired" — re-checked every time you wake up, not assumed
true just because you were signaled.
