# Exercise 6 hints

## Level 1
`pthread_cond_timedwait` takes an ABSOLUTE point in time (a `struct timespec`
with seconds + nanoseconds since the epoch), not a duration. That's why you
build it from `clock_gettime(CLOCK_REALTIME, ...)` plus your desired delay —
get "now" first, then add to it.

Remember `#include <errno.h>` for `ETIMEDOUT`.

## Level 2
`tv_nsec` must always stay in `[0, 999999999]`. If you do
`ts.tv_nsec += (ms % 1000) * 1000000` and that pushes it to, say,
1,300,000,000, you have an invalid timespec — behavior is undefined or
just wrong, don't rely on the OS to "fix it for you."

The fix: after adding, check `if (ts.tv_nsec >= 1000000000) { ts.tv_nsec -= 1000000000; ts.tv_sec += 1; }`

## Level 3
```c
void build_deadline(struct timespec *ts, int ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += (ms % 1000) * 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_nsec -= 1000000000;
		ts->tv_sec += 1;
	}
}
```
Loop with `while`, same reasoning as exercise 4 — a timedwait can also
return early (spurious wakeup) without being ETIMEDOUT and without the
condition being true, so re-check both.

This exact function is what your `time_utils.c`'s `ms_to_abstime` needs to
do, and it's what powers BOTH your dongle cooldown wait and (if you choose
to implement it this way) your burnout deadline check. Get the overflow
handling right here once, reuse it everywhere.
