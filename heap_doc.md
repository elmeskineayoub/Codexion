# Codexion — Heap Module Documentation

`heap.c` and `heap_utils.c` — the priority queue behind dongle arbitration.

This module answers one question, over and over, for the whole simulation:
*given several coders waiting for the same dongle, who gets it next?* The
subject names the data structure explicitly, and the tie-breaking rules are one
of the few places it spells out a requirement in detail. It is also the only
module in the project that can be fully tested in isolation — which is why it
was, before a single thread touched it.

---

## 1. What the subject requires

From Chapter VI, mandatory part:

> *"You must implement a priority queue (heap) for FIFO/EDF scheduling (no
> standard library priority queue may be used)."*

> *"Fair arbitration is mandatory: when multiple coders request the same dongle,
> the dongle must grant access according to scheduler. With fifo, serve requests
> in arrival order. With edf, serve the coder with the earliest burnout deadline
> (i.e., last_compile_start + time_to_burnout)."*

And the note that shapes the comparator:

> *"Due to timestamp precision, equal deadlines may rarely occur in practice.
> The tie-breaker rule is required to ensure a fully deterministic EDF policy,
> even in edge cases."*

Three obligations: implement a heap by hand, order by the chosen policy, and
break ties deterministically.

---

## 2. Layout and why it's split

```
heap.c          (5 functions — at Norm cap)
  heap_push       insert a request, restore the heap property upward
  sift_up         bubble an element toward the root
  sift_down       bubble an element toward the leaves
  heap_pop        remove and return the highest-priority request
  heap_peek       read the highest-priority request without removing it

heap_utils.c    (5 functions — at Norm cap)
  swap            exchange two requests in place
  cmp_request     the scheduling policy — FIFO vs EDF, with tie-breaks
  heap_create     allocate and initialise a heap
  heap_destroy    free a heap and its backing array
```

**Why this boundary:** `heap.c` holds the algorithm — the operations that
manipulate tree structure. `heap_utils.c` holds construction, destruction, and
the two primitives those operations call (`swap` for moving elements,
`cmp_request` for deciding order). The Norm's 5-function cap forced a split;
this line keeps each file coherent.

**Why `swap` isn't in `heap.c` where it's used:** `heap.c` was already at five
functions. Both sift operations need it, so it moved to the utils file. Note it
is *not* `static` — a static function is invisible outside its own translation
unit, so it could not be called from `heap.c`. It needs an ordinary prototype in
`codexion.h`.

**Both files are at capacity.** Any further heap helper needs a third file.

---

## 3. The data structures

### `t_request` — one coder's claim on one dongle

```c
typedef struct s_request
{
	int		coder_id;
	long	deadline;
	long	arrival;
}	t_request;
```

- `coder_id` — whose request this is. Used by the waiting coder to recognise
  itself at the front of the queue, and by `cmp_request` as the final, always-
  unique tie-breaker.
- `deadline` — `last_compile_start + time_to_burnout`. EDF's primary key. The
  subject defines it in exactly these terms.
- `arrival` — `now_ms()` at the moment the request was queued. FIFO's primary
  key, EDF's secondary key.

All three participate in ordering. That `coder_id` is unique across the
simulation is what guarantees the comparator can never be ambiguous — see
section 5.

### `t_heap` — an array pretending to be a tree

```c
typedef struct s_heap
{
	t_request	*data;
	int			size;
	int			capacity;
	int			scheduler;
}	t_heap;
```

- `data` — flat array. The tree structure is *implied by indices*, not stored.
- `size` — number of live elements. Everything at index ≥ `size` is stale.
- `capacity` — allocated slots, fixed at `num_coders`, never resized.
- `scheduler` — `FIFO` or `EDF`, passed through to `cmp_request` on every
  comparison.

**Why an array and not nodes with pointers.** A binary heap is always a
*complete* tree: every level is full except possibly the last, which fills left
to right. That regularity means a node's relatives can be computed instead of
stored:

| Relationship | Formula |
|---|---|
| parent of `i` | `(i - 1) / 2` |
| left child of `i` | `2 * i + 1` |
| right child of `i` | `2 * i + 2` |

No per-node allocation, no pointer chasing, contiguous memory. These three
formulas are two views of the same edges: going *down* from a node gives its
children, going *up* gives its parent. A node at index 1 is both a child of
index 0 and the parent of indices 3 and 4 — the formulas describe different
directions, not conflicting facts.

**The heap property** — the single invariant every operation preserves:

> every parent has higher priority than both of its children, as judged by
> `cmp_request`.

Note what this does *not* say: the array is **not sorted**. Only the local
parent–child relation holds. Siblings have no defined order relative to each
other. That weaker guarantee is precisely why operations are logarithmic
instead of linear — and it is enough, because the only question ever asked is
"who is the winner?", and the winner is always at index 0.

**Why capacity is `num_coders` and never grows.** Each coder's left and right
dongles are assigned once in `init_coders` (`left = i`, `right = (i + 1) % n`)
and never change. Which coders can ever reference dongle `j`? Solving both:

- `coder[j].left_dongle = j` → coder `j`
- `coder[k].right_dongle = (k + 1) % n = j` → `k = (j - 1 + n) % n`

Exactly two coders, for the entire run. Real occupancy never exceeds 2.
`num_coders` is a generous upper bound that eliminates any need for
reallocation logic. This observation is worth stating in a defense — it shows
the topology is understood, not just implemented.

---

## 4. `heap_utils.c` — line by line

### `swap`

```c
void	swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
```

Exchanges two requests through a temporary.

**Why pointer parameters.** C passes by value. A version taking `t_request a,
t_request b` would receive *copies*, swap the copies, and leave the array
untouched — a silent no-op. Taking addresses means the function writes to the
real array slots. Callers pass `&heap->data[i]`, not `heap->data[i]`.

**Why a struct copy and not memcpy or bit tricks.** `t_request` is three
scalars; the compiler generates efficient register moves. Clarity wins, and
there is nothing to optimise here.

### `cmp_request`

```c
int	cmp_request(t_request *a, t_request *b, t_scheduler scheduler)
{
	if (scheduler == FIFO)
	{
		if (a->arrival != b->arrival)
			return (a->arrival < b->arrival);
	}
	if (scheduler == EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
		if (a->arrival != b->arrival)
			return (a->arrival < b->arrival);
	}
	return (a->coder_id < b->coder_id);
}
```

Returns non-zero if `a` should be served before `b`. This one function is the
entire scheduling policy.

**The two chains:**

| Scheduler | Order of comparison |
|---|---|
| FIFO | `arrival` → `coder_id` |
| EDF | `deadline` → `arrival` → `coder_id` |

**Line by line:**

- `if (scheduler == FIFO) { if (a->arrival != b->arrival) return (a->arrival <
  b->arrival); }` — whoever asked first wins. If arrivals differ, decided; fall
  through to the shared `coder_id` fallback otherwise.
- `if (scheduler == EDF) { ... }` — whoever burns out soonest wins. If deadlines
  differ, decided. If they tie, fall to `arrival`. If that ties too, fall
  through.
- `return (a->coder_id < b->coder_id);` — the shared final fallback for both
  policies. Always decides, because `coder_id` is unique.

**Why ties are not hypothetical.** `now_ms()` has millisecond resolution. Two
coders requesting the same dongle within the same millisecond produce identical
`arrival` values. Two coders that began compiling in the same millisecond carry
identical `deadline` values for their entire cycle. Both happen routinely at
small timing parameters.

**Why EDF needs `arrival` before falling to `coder_id`.** If EDF jumped
straight from tied deadlines to `coder_id`, the lower-numbered coder would win
*every* contention between that pair — not once, but permanently, for as long as
their deadlines keep tying. That is a systematic bias, and it threatens the
subject's liveness requirement: *"no coder should be starved of dongles and burn
out under edf scheduling, provided the parameters are feasible."* Falling back
to `arrival` first resolves ties by who actually asked earlier, which rotates
naturally as coders drift out of phase.

**Why a final unique fallback is mandatory, not defensive.** A comparator
driving a heap must define a **total, deterministic order**. If two distinct
requests can compare as "neither before the other," `sift_up` and `sift_down`
have no consistent relation to work with, and the heap's structure can genuinely
corrupt — not just produce a wrong winner. Since `coder_id` is unique by
construction, the cascade always terminates in a definite answer. An earlier
version of this function fell out of the FIFO branch with no return at all when
arrivals tied, which is undefined behaviour and would have been caught by
`-Werror` as `control reaches end of non-void function`.

**The design payoff.** Neither `sift_up` nor `sift_down` mentions FIFO or EDF
anywhere. They only ask "does this beat that?" Changing the scheduler changes
this function's answer and nothing else in the module. One heap implementation
serves both policies — which is why the subject's "FIFO/EDF" requirement didn't
need two data structures.

### `heap_create`

```c
t_heap	*heap_create(int capacity, int scheduler)
{
	t_heap	*heap;

	heap = malloc(sizeof(t_heap));
	if (!heap)
		return (NULL);
	heap->data = malloc(capacity * sizeof(t_request));
	if (!heap->data)
	{
		free(heap);
		return (NULL);
	}
	heap->size = 0;
	heap->capacity = capacity;
	heap->scheduler = scheduler;
	return (heap);
}
```

Allocates the heap struct and its backing array, initialises the bookkeeping
fields.

**Line by line:**

- `heap = malloc(sizeof(t_heap)); if (!heap) return (NULL);` — the struct.
  Checked, because the next line dereferences it — an unchecked failure here is
  an immediate segfault, and the subject treats segfaults as automatic zero.
- `heap->data = malloc(capacity * sizeof(t_request));` — the array.
- `if (!heap->data) { free(heap); return (NULL); }` — **the failure path is the
  interesting part.** If the second malloc fails, the first already succeeded.
  Returning `NULL` without freeing `heap` would leak. This is the same
  "unwind what already succeeded" discipline that partial-init cleanup requires
  elsewhere in the project, at its smallest possible scale.
- `heap->size = 0;` — empty.
- `capacity` and `scheduler` stored for later use by `heap_push` (bounds check)
  and `cmp_request` (policy).

**Return convention:** `NULL` on failure, valid pointer on success. This is the
standard for pointer-returning functions and matches `malloc` itself — it does
not contradict the project's `0 = success` convention for `int`-returning
functions, because there is no `int` to return.

Callers check it: `init_dongles` does `if (!sim->dongles[i].waiters) return (1);`

### `heap_destroy`

```c
void	heap_destroy(t_heap *heap)
{
	if (!heap)
		return ;
	free(heap->data);
	free(heap);
}
```

Frees the backing array, then the struct.

**Order matters:** `heap->data` must be freed *before* `heap`, because reading
`heap->data` after freeing `heap` is a use-after-free.

**The `if (!heap) return ;` guard is load-bearing, not padding.** Contrast with
the NULL check that was correctly *removed* from `ms_to_abstime` — there, the
parameter was always the address of a stack variable and could never be NULL.
Here, `cleanup_all` runs after a possible *partial* init in which some dongles
received heaps and others did not. Those others hold `NULL` (because `main`
memsets the sim struct). The guard is what makes cleanup safe on every failure
path. A NULL check earns its place when a NULL can actually arrive.

---

## 5. `heap.c` — line by line

### `heap_push`

```c
int	heap_push(t_heap *heap, t_request req)
{
	int	i;

	if (heap->size == heap->capacity)
		return (1);
	heap->data[heap->size] = req;
	i = heap->size;
	heap->size++;
	sift_up(heap, i);
	return (0);
}
```

Inserts a request and restores the heap property.

**Line by line:**

- `if (heap->size == heap->capacity) return (1);` — bounds check. Given the
  topology argument in section 3, overflow cannot occur; this is insurance
  against a bug elsewhere. Returning a status rather than failing silently means
  a dropped request would surface immediately instead of manifesting as an
  unexplained hang — a coder that thinks it's queued but isn't would wait
  forever for a grant that can never come.
- `heap->data[heap->size] = req;` — place the new element in the first free
  slot. That slot is always index `size`, because the tree is complete: elements
  fill left to right with no gaps.
- `i = heap->size;` — **remember the index before growing.** This ordering is
  not cosmetic. If `size++` came first, `i` would be one past the element just
  inserted and `sift_up` would bubble the wrong slot.
- `heap->size++;` — the element is now live.
- `sift_up(heap, i);` — the new element is at the bottom but may outrank its
  ancestors. Bubble it to its correct level.
- `return (0);` — success, per the project convention.

**On the return convention.** `0` means success here, matching `heap_pop`,
`heap_peek`, `init_sim`, `init_dongles`, `init_coders`, and every `pthread_*`
call in the program. The intuition that "1 should mean it worked" is natural but
locally scoped — the reason C settled on `0 = success` is that there is one way
to succeed and many ways to fail, so a single `!= 0` check catches every failure
mode regardless of which occurred. Mixing conventions within one module is how
`if (heap_push(...))` and `if (heap_pop(...))` end up meaning opposite things.

**Complexity:** O(log n) — `sift_up` walks at most the height of the tree.

### `sift_up`

```c
void	sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (cmp_request(&heap->data[i], &heap->data[parent], heap->scheduler))
		{
			swap(&heap->data[i], &heap->data[parent]);
			i = parent;
		}
		else
			break ;
	}
}
```

Moves an element toward the root until the heap property holds above it.

**Line by line:**

- `while (i > 0)` — index 0 is the root; it has no parent. Reaching it means the
  element has bubbled as far as it can go.
- `parent = (i - 1) / 2;` — who is above me. Integer division. For `i = 1` or
  `i = 2`, this gives 0 (both children of the root). For `i = 3` or `i = 4`, it
  gives 1.
- `if (cmp_request(&heap->data[i], &heap->data[parent], ...))` — do I outrank my
  parent? Note the `&` on both — `cmp_request` takes pointers.
- `swap(...); i = parent;` — exchange with the parent, then continue from the
  parent's position. The element has moved up one level.
- `else break ;` — the element lost to its parent. Since the parent already
  satisfied the heap property relative to *its* ancestors, everything above is
  correctly ordered too. Continuing would be wasted work. This early exit is
  what makes the average case faster than the worst case.

**Why this restores the property.** Before the push, the heap was valid. Adding
a leaf can only violate the property along the single path from that leaf to the
root — no sibling relationships changed. Walking that one path is sufficient.

### `sift_down`

```c
void	sift_down(t_heap *heap, int i)
{
	int	left;
	int	best;
	int	right;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < heap->size && cmp_request(&heap->data[left],
				&heap->data[best], heap->scheduler))
			best = left;
		if (right < heap->size && cmp_request(&heap->data[right],
				&heap->data[best], heap->scheduler))
			best = right;
		if (best == i)
			break ;
		swap(&heap->data[i], &heap->data[best]);
		i = best;
	}
}
```

Moves an element toward the leaves until the heap property holds below it.
Harder than `sift_up` because it compares against **two** children rather than
one parent.

**Line by line:**

- `while (1)` — the loop exits via `break` when nothing moves, not via a
  condition at the top. There is no simple index bound here the way `i > 0`
  worked for `sift_up`.
- `left = 2 * i + 1; right = 2 * i + 2;` — the two children.
- `best = i;` — assume the current node wins; the two checks below may override.
- `if (left < heap->size && ...)` — **the bounds check is mandatory, not
  defensive.** A node near the bottom may have one child or none at all. Without
  it, the code reads past `size` into stale data — elements that are still
  physically present in the array from earlier pops but logically removed. Those
  values would compare as real candidates and corrupt the heap.
- `if (right < heap->size && cmp_request(&heap->data[right], &heap->data[best],
  ...))` — **the second comparison targets `best`, not `i`.** If the left child
  already won, the right child must beat *the left child*, not the original
  parent. Comparing both against `i` is a classic bug: it can promote the right
  child over the parent while the left child outranks both, leaving a violated
  property that casual testing won't reveal.
- `if (best == i) break ;` — nothing outranks the current node. The property
  holds from here down; stop.
- `swap(&heap->data[i], &heap->data[best]); i = best;` — move down one level and
  repeat from the new position.

**Why only one path matters here too.** Swapping with the higher-priority child
guarantees that child's former subtree is the only place the property can now be
violated. The other subtree is untouched.

**On the wrapped lines:** the two `cmp_request` calls exceed 80 columns on one
line, so `c_formatter_42` split them. Norminette accepts the continuation
indentation it produced. An alternative would be local aliases (`t_request *d =
heap->data; int s = heap->scheduler;`) to shorten the expressions — but that
would bring the function to five local variables, exactly at the Norm's
declaration cap, with no room to spare.

### `heap_pop`

```c
int	heap_pop(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (1);
	*out = heap->data[0];
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	sift_down(heap, 0);
	return (0);
}
```

Removes and returns the highest-priority request.

**Line by line:**

- `if (heap->size == 0) return (1);` — empty heap, nothing to pop. The caller
  must check this before using `*out`.
- `*out = heap->data[0];` — **save the root before it is overwritten.** The root
  is the winner by the heap property. This must happen before the next lines
  clobber index 0.
- `heap->size--;` — **decrement before the assignment.** After decrementing,
  `heap->size` *is* the index of the last valid element. Doing the assignment
  first and decrementing after would read one slot past the end.
- `heap->data[0] = heap->data[heap->size];` — promote the last element into the
  vacated root slot.
- `sift_down(heap, 0);` — that promoted element is almost certainly in the wrong
  place (it was a leaf; leaves are typically low priority). Push it down to
  where it belongs.
- `return (0);` — success.

**Why the *last* element specifically, and not some other?** Removing the root
leaves a hole. Filling it with the last element is the only choice that
preserves the *complete tree* shape — every level full except the last, filling
left to right. Any other element would leave a gap mid-tree, and the index
formulas (`2i+1`, `2i+2`, `(i-1)/2`) depend entirely on that shape holding.
Break completeness and the arithmetic stops describing the structure.

**Complexity:** O(log n).

### `heap_peek`

```c
int	heap_peek(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (1);
	*out = heap->data[0];
	return (0);
}
```

Reads the highest-priority request without removing it.

**Its role in the protocol.** A waiting coder calls this inside its
`pthread_cond_timedwait` predicate loop in `acquire_one`, to ask *"am I
currently at the front of this dongle's queue?"*:

```c
heap_peek(dongle->waiters, &front);
if (is_available(dongle) && front.coder_id == coder->id)
```

The check must be **non-destructive**, because the coder may well find it is
*not* first and has to go back to sleep with the queue intact. Using `heap_pop`
here would remove whichever coder genuinely was first — every time any coder
woke up and checked.

**Complexity:** O(1) — a single array read.

---

## 6. The out-parameter pattern

`heap_pop` and `heap_peek` each need to return two things: a status (did it
succeed?) and a payload (which request?). A C function returns one value, so the
status becomes the return value and the payload is written through a
caller-supplied pointer:

```c
t_request	winner;

if (heap_pop(dongle->waiters, &winner) == 0)
{
	/* winner holds the highest-priority request */
}
```

The caller owns the storage (`winner` is on its stack), so there is no
allocation and nothing to free. The function only writes into it on success —
on failure `*out` is untouched, which is why the status must be checked before
reading it.

---

## 7. Conventions and complexity

**Return values:** `0` = success, non-zero = failure, for every `int`-returning
function in this module and across the project. Pointer-returning functions use
`NULL` for failure.

| Operation | Complexity | Notes |
|---|---|---|
| `heap_push` | O(log n) | one root-ward path |
| `heap_pop` | O(log n) | one leaf-ward path |
| `heap_peek` | O(1) | single read of index 0 |
| `cmp_request` | O(1) | at most three integer comparisons |
| `heap_create` | O(1) | two mallocs |
| `heap_destroy` | O(1) | two frees |

---

## 8. Verification performed

Compiled against `heap.c` and `heap_utils.c` only, with a throwaway `main` —
deliberately *before* integrating into threaded arbitration, because a heap bug
and a race condition produce similar-looking symptoms and separating them made
both tractable.

| Case | Expectation | Result |
|---|---|---|
| EDF, distinct deadlines (900, 400, 700, 200, 600) | popped ascending: 200, 400, 600, 700, 900 | pass |
| FIFO, distinct arrivals (48, 10, 70, 25, 33) | popped ascending: 10, 25, 33, 48, 70 | pass |
| EDF, tied deadlines, different arrivals | earlier `arrival` first | pass |
| EDF, tied deadline *and* arrival | lower `coder_id` first | pass |
| `heap_pop` / `heap_peek` on empty heap | return 1, no crash, `*out` untouched | pass |
| `heap_push` beyond capacity | third push returns 1, heap intact | pass |

**A testing lesson from this run.** The first capacity test printed
`push1=1 push2=0 push3=0` — apparently impossible, since the first two pushes
should succeed and only the third should fail. The cause was not a heap bug:
all three `heap_push` calls had been written as arguments to a single `printf`,
and **C does not define the evaluation order of function arguments.** The
compiler evaluated them right-to-left, so `push3` ran first against an empty
heap. Split into separate statements, the result was the expected `0 0 1`. If a
result ever looks impossible, check whether side-effecting calls share an
expression.

---

## 9. Anticipated defense questions

**"Why a heap and not a linked list or a sorted array?"**
Finding the minimum in an unsorted list is O(n) on every grant. Keeping a sorted
array costs O(n) per insertion from shifting elements. A heap is O(log n) for
both push and pop with no per-node allocation and good cache locality. The
subject also names the heap explicitly.

**"Only two coders can ever queue on one dongle — isn't a heap overkill?"**
Structurally, yes: dongle `j` is referenced only by coder `j` and coder
`(j - 1 + n) % n`, so occupancy never exceeds 2, and a two-slot array with one
comparison would suffice. The heap was implemented in general form because the
subject requires a priority queue, and because the general version costs little
more than a special-cased one while removing an entire class of edge cases.

**"What is the heap property, exactly?"**
Every parent outranks both children, as judged by `cmp_request`. The array is
*not* sorted — only that local relation holds. That is what makes push and pop
logarithmic rather than linear, while still guaranteeing the winner sits at
index 0.

**"What if two requests compare as completely equal?"**
They cannot. `coder_id` is unique per coder and terminates both comparison
chains, so any two distinct requests always have a defined order. This matters
beyond fairness: an ambiguous comparator can corrupt the heap's structure,
because the sift operations assume a consistent ordering relation.

**"Why does EDF check `arrival` before `coder_id`?"**
Without it, tied deadlines would always resolve in favour of the lower-numbered
coder — permanently, for as long as the tie persists. That is systematic
starvation risk, which the subject's liveness requirement forbids. `arrival`
resolves by who genuinely asked first, which rotates as coders drift.

**"Why does each dongle own a heap instead of one global queue?"**
Arbitration is scoped per dongle by the subject. A global queue would force
coders competing for entirely different dongles into a single ordering, which is
neither required nor correct.

**"Where is `heap_remove`?"**
There isn't one, deliberately. The only path where a waiter leaves a queue
without being granted is simulation shutdown, at which point every heap is
destroyed and no further arbitration occurs — an abandoned entry is harmless.
Arbitrary-position removal (locate index, swap with last, then sift in whichever
direction is needed) would be dead code with no other caller.

**"Why is `size--` before the array assignment in `heap_pop`?"**
After decrementing, `size` is the index of the last valid element. Assigning
first and decrementing after would read one slot past the live region.

**"Why does `sift_down` compare the right child against `best` instead of `i`?"**
Because if the left child already won, the right child must beat the left child
to take its place. Comparing both against the original parent can promote the
wrong element and leave the heap property violated.

**"Why is `swap` in `heap_utils.c` rather than next to its callers?"**
`heap.c` was at the Norm's five-function limit. It could not be `static` there
either, since a static function is invisible outside its own translation unit
and both sift operations live in `heap.c`.