# Exercise 5 hints

## Level 1
`pthread_cond_signal` wakes AT MOST one waiting thread — which one is
unspecified, it's whichever the OS scheduler picks. `pthread_cond_broadcast`
wakes ALL waiting threads.

## Level 2
With `signal`: only 1 of the 3 consumers wakes up. The other 2 stay asleep
forever (in this exercise, since nothing else ever signals again) — your
program will hang at `pthread_join` on those two. That hang IS the lesson,
don't "fix" it by adding more signals yet, just observe it and understand
why.

With `broadcast`: all 3 wake up, all 3 re-check `while (!ready)`, see it's
now true, and all 3 proceed.

## Level 3
Map this directly onto Codexion: your stop-flag (the "custom event" the
subject mentions) needs to wake up EVERY coder thread that might be
waiting on a dongle's condition variable when the simulation ends —
otherwise some coder threads never wake up, `pthread_join` on them hangs
forever, and your program never exits even after a burnout is correctly
detected and logged. That means when you set the stop flag, you broadcast,
not signal — on every cond var any coder could be waiting on.

Conversely, when a single dongle becomes available for exactly one waiting
coder, signal is usually the right (and more efficient) choice, since
broadcasting there would just wake everyone up to re-check and go back to
sleep. Knowing which situation needs which is the actual skill here.
