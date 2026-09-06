# Exercise 1 hints

## Level 1
`pthread_create` takes a `void *arg`. Whatever you pass, the thread function
gets that SAME pointer value back. If you pass `&i` where `i` is your loop
counter, every thread receives a pointer to the *same* memory address — and
that memory keeps changing after `pthread_create` returns, because the loop
keeps running.

## Level 2
Two common fixes:
1. Allocate one `int` per thread (`malloc`), pass that pointer, free it inside
   the thread function once you're done with it.
2. Cast the integer itself to `void *` directly: `(void *)(long)i`, and cast
   it back inside the thread with `(int)(long)arg`. No allocation needed.
   This works because on this platform a pointer and a `long` are the same
   size — it's  a common trick specifically for passing small integers, not a
   general pattern for passing real data.

For Codexion you'll actually want option 1's spirit: an array of coder
structs, and you pass `&coders[i]` — each element has a stable, unique
address for the whole program's life, so this exact bug can't happen.

## Level 3
```c
long idx = i;
pthread_create(&threads[i], NULL, thread_func, (void *)idx);
```
and in the thread function: `int id = (int)(long)arg;`

Also acceptable and closer to what you'll do in Codexion:
```c
int *ids = malloc(sizeof(int) * NUM_THREADS);
ids[i] = i;
pthread_create(&threads[i], NULL, thread_func, &ids[i]);
```
