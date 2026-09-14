# Codexion — Parsing Module Documentation

`parse.c` and `parse_utils.c` — argument validation and simulation configuration.

This module is the program's front door. Every value the simulation runs on
passes through here, and the subject grades input rejection explicitly. It's
also the module most likely to be probed with adversarial inputs during
evaluation ("what if I pass `abc`?", "what about `-5`?", "what about
99999999999?").

---

## 1. What the subject requires

From Chapter V, the only stated rule on validation:

> *"Reminder: All arguments are mandatory. Reject invalid inputs such as
> negative numbers, non-integers, or a scheduler other than fifo or edf."*

Three things to reject: negatives, non-integers, bad scheduler strings.
**Nothing is said about zero.** That silence shapes the zero-handling decision
in section 4.

From Chapter VI, the argument list, in order:

```
number_of_coders time_to_burnout time_to_compile time_to_debug
time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Eight arguments, all mandatory. With the program name, `argc == 9`.

---

## 2. Layout and why it's split

```
parse_utils.c   (5 functions — at Norm cap)
  is_digit          character classification
  ft_strlen         string length
  ft_strcmp         string comparison
  ft_atoi           string → int with overflow detection
  is_number         "is this string a well-formed integer?"

parse.c         (5 functions — at Norm cap)
  print_error       write a message to stderr
  get_scheduler     "fifo" / "edf" → enum, or -1
  parse_numeric_args  validate and convert argv[1..7]
  fill_sim          copy validated values into t_sim
  parse_args        orchestrator
```

**Why the boundary is where it is:** the five functions in `parse_utils.c` know
nothing about Codexion — they are generic string and number primitives that
would work unchanged in any C program. The five in `parse.c` are specific to
this program's argument list. The Norm's 5-function cap forced the split; the
split point was chosen so each file has a coherent purpose rather than an
arbitrary one.

**Why `libft` isn't used:** the subject says *"Libft authorized: No."* Every
string helper is written here from scratch. `strcmp`, `strlen`, and `atoi` are
in the authorized function list, but writing them is trivial and avoids any
question about which functions were used where.

---

## 3. `parse_utils.c` — line by line

### `is_digit`

```c
int	is_digit(char c)
{
	if (c >= 48 && c <= 57)
		return (1);
	return (0);
}
```

Returns 1 if `c` is `'0'` through `'9'`, else 0.

`48` and `57` are the ASCII codes for `'0'` and `'9'`. Writing `c >= '0' && c
<= '9'` would be equivalent and arguably more readable; the numeric form works
because character literals *are* integers in C. Either is defensible.

### `ft_strlen`

```c
int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}
```

Counts characters until the null terminator. `while (str[i])` is `while
(str[i] != '\0')` — the null byte is zero, which is false.

Used only by `print_error`, because `write` needs an explicit byte count.

### `ft_strcmp`

```c
int	ft_strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
```

Walks both strings while three things hold: `s1` hasn't ended, `s2` hasn't
ended, and the current characters match. Stops at the first difference or the
first terminator. Returns the difference between the characters at that
position — 0 if the strings are identical.

**Why the `unsigned char` casts matter:** on x86, plain `char` is signed. A byte
above 127 is negative as `char`. Subtracting two signed chars where one is
"negative" gives a result with the wrong sign compared to the standard
`strcmp`. Casting to `unsigned char` before subtracting matches libc's
behaviour. This program only ever compares ASCII, so it wouldn't bite here —
but it's the correct general form and a fair thing to be asked about.

**Why the loop condition needs both `s1[i]` and `s2[i]`:** if only `s1[i]` were
checked and `s2` were shorter, the loop would read past `s2`'s terminator.
Checking both stops at whichever ends first. The final subtraction then
correctly yields non-zero when one string is a prefix of the other.

### `ft_atoi`

```c
int	ft_atoi(char *str)
{
	int		i;
	int		sign;
	long	result;

	i = 0;
	sign = 1;
	result = 0;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i])
	{
		result = result * 10 + (str[i] - '0');
		if (result * sign > INT_MAX || result * sign < INT_MIN)
			return (-1);
		i++;
	}
	return ((int)(result * sign));
}
```

Converts a string of digits (optionally signed) into an `int`.

**Line by line:**

- `sign = 1;` — default positive.
- `if (str[i] == '-' || str[i] == '+')` — consume an optional sign. `-` flips
  `sign`; `+` is consumed but changes nothing.
- `while (str[i])` — for each remaining character:
  - `result = result * 10 + (str[i] - '0');` — shift the accumulated value one
    decimal place left and add the new digit. `str[i] - '0'` converts the
    character `'7'` (ASCII 55) to the integer 7 by subtracting `'0'` (ASCII 48).
  - `if (result * sign > INT_MAX || result * sign < INT_MIN) return (-1);` —
    **overflow guard.** `result` is a `long` so it can hold values beyond `int`
    range without wrapping; the check catches the moment the accumulated value
    would no longer fit in an `int`.
- `return ((int)(result * sign));` — apply the sign and narrow to `int`.

**Why `result` is `long`:** if it were `int`, `result * 10` on a large value
would overflow *before* the check could see it — signed overflow is undefined
behaviour in C, so the check would be testing a value that has already gone
wrong. `long` (64-bit on the target platform) gives headroom to detect the
overflow before it happens.

**On the `-1` sentinel — the honest caveat:** returning `-1` for overflow is
ambiguous, because `"-1"` legitimately parses to `-1` too. This works here
because every argument rejects negatives downstream, so both an overflowed
input and a literal `-1` end up rejected. A cleaner design would use a separate
error flag or an `int *` out-parameter. If asked, say exactly that: it's sound
given the validation that follows, and here is what a more general version
would do.

**What `ft_atoi` does NOT do:** it does not validate that the input is
well-formed. It assumes every character after the sign is a digit. That is
`is_number`'s job, and `is_number` is always called first.

### `is_number`

```c
int	is_number(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '-' || str[i] == '+')
		i++;
	if (!is_digit(str[i]))
		return (0);
	while (str[i])
	{
		if (!is_digit(str[i]))
			return (0);
		i++;
	}
	return (1);
}
```

Returns 1 if the string is a well-formed integer literal: optional sign, then
one or more digits, then nothing else.

**Line by line:**

- `if (str[i] == '-' || str[i] == '+') i++;` — skip an optional leading sign.
- `if (!is_digit(str[i])) return (0);` — **there must be at least one digit.**
  This rejects `""`, `"-"`, `"+"`, and `"abc"`. Without this line, a lone `"-"`
  would skip the sign, enter the loop with `str[i] == '\0'`, never execute the
  body, and return 1 — a false positive.
- `while (str[i]) { if (!is_digit(str[i])) return (0); i++; }` — every
  remaining character must be a digit. Rejects `"12abc"`, `"1.5"`, `"--5"`,
  `"1 2"`.
- `return (1);` — passed every check.

**Inputs this correctly rejects:**

| Input | Why |
|---|---|
| `""` | no digit after (absent) sign |
| `"-"` | no digit after sign |
| `"abc"` | first char not a digit |
| `"12abc"` | `a` fails the loop |
| `"1.5"` | `.` fails the loop |
| `"--5"` | second `-` fails the loop |
| `" 12"` | leading space fails the first check |

**Inputs this accepts that then get rejected later:**

| Input | Accepted by `is_number`? | Rejected where |
|---|---|---|
| `"-5"` | yes | value check in `parse_numeric_args` |
| `"99999999999"` | yes | overflow → `-1` → value check |
| `"0"` for coder count | yes | value check (index 0 requires > 0) |

Separating "is it syntactically a number?" from "is the value acceptable?" keeps
each function doing one thing.

---

## 4. `parse.c` — line by line

### `print_error`

```c
void	print_error(char *msg)
{
	write(2, msg, ft_strlen(msg));
}
```

Writes the message to file descriptor 2 — standard error.

**Why `write` and not `printf`:** `write` is an unbuffered system call — the
bytes leave the process immediately. `printf` buffers, and a following
`exit(1)` can terminate the process before the buffer flushes, losing the
message. `write` also needs no format string and is in the subject's authorized
list.

**Why fd 2 and not fd 1:** errors go to stderr so they never mix into the
timestamped simulation log on stdout. An evaluator piping stdout to a file to
check log format should never find error text in it.

### `get_scheduler`

```c
int	get_scheduler(char *str)
{
	if (ft_strcmp(str, "fifo") == 0)
		return (FIFO);
	if (ft_strcmp(str, "edf") == 0)
		return (EDF);
	return (-1);
}
```

Resolves the scheduler string to the enum value, or `-1` if invalid.

**Why one function does both validate and resolve:** an earlier version had
`valid_scheduler` returning a bool, and `fill_sim` comparing the string a second
time to pick the enum. That compared the same string twice for no reason. This
version does it once — the original observation that motivated the change was
correct.

**The implicit dependency to know about:** `-1` works as an error sentinel only
because the enum starts at 0 (`FIFO = 0`, `EDF = 1`), so `-1` can never be a
legitimate return. That's true by construction, but if the enum were ever
reordered to include a negative value, this would break silently. Naming that
assumption is better than pretending there isn't one.

**Why the comparison is exact:** `"FIFO"`, `"Fifo"`, `"fifo "` are all rejected.
The subject says *"The value must be exactly one of: fifo or edf."* Exact means
exact.

### `parse_numeric_args`

```c
int	parse_numeric_args(char **argv, int *values)
{
	int	i;
	int	ok;

	i = 1;
	while (i <= 7)
	{
		if (!is_number(argv[i]))
		{
			print_error("[ERROR] invalid numeric argument!\n");
			exit(1);
		}
		values[i - 1] = ft_atoi(argv[i]);
		if (i - 1 == 0)
			ok = (values[i - 1] > 0);
		else
			ok = (values[i - 1] >= 0);
		if (!ok)
		{
			print_error("[ERROR] invalid argument value!\n");
			exit(1);
		}
		i++;
	}
	return (0);
}
```

Validates and converts `argv[1]` through `argv[7]` into `values[0..6]`.

**Line by line:**

- `i = 1; while (i <= 7)` — `argv[0]` is the program name; the seven numeric
  arguments are `argv[1]` through `argv[7]`. `argv[8]` is the scheduler string,
  handled separately.
- `if (!is_number(argv[i])) { error; exit }` — syntactic check first. If the
  string isn't a well-formed integer, stop.
- `values[i - 1] = ft_atoi(argv[i]);` — convert. `argv[1]` → `values[0]`,
  etc. The `- 1` is the offset between argv indexing and the values array.
- The zero-handling decision:
  - `if (i - 1 == 0) ok = (values[i - 1] > 0);` — index 0 is
    `number_of_coders`. Must be strictly positive.
  - `else ok = (values[i - 1] >= 0);` — everything else accepts zero, rejects
    negatives.
- `if (!ok) { error; exit }` — value check failed.
- `return (0);` — success.

**Why the loop runs to 7, not 6:** an earlier version stopped at `argv[6]` and
parsed `dongle_cooldown` separately in `parse_args`. That separate block pushed
`parse_args` to 27 lines — over the Norm's 25. Since cooldown is just another
numeric argument, folding it in as index 6 removed the block and brought
`parse_args` to ~16 lines. The change was Norm-driven but produced cleaner code.

**The zero decision, and how to defend it:** the subject lists three things to
reject: negatives, non-integers, invalid schedulers. It says nothing about zero.
So zero is accepted wherever it is meaningful:

| Argument | Zero accepted? | Reasoning |
|---|---|---|
| `number_of_coders` | **No** | zero coders means no threads, no dongles, nothing to simulate |
| `time_to_burnout` | Yes | subject doesn't forbid it; burnout would be immediate |
| `time_to_compile` | Yes | instantaneous compile is a valid (degenerate) scenario |
| `time_to_debug` | Yes | same |
| `time_to_refactor` | Yes | a coder finishing debug and immediately requesting dongles is realistic |
| `number_of_compiles_required` | Yes | "all coders compiled ≥ 0 times" is true at t=0; sim exits immediately |
| `dongle_cooldown` | Yes | zero cooldown = instantly reusable; the natural baseline |

Defense phrasing: *"The subject specifies rejecting negatives, non-integers, and
invalid schedulers. It doesn't mention zero, so I accept zero everywhere it's
meaningful. The one exception is coder count, since zero coders means there's
nothing to simulate."*

**Why `exit(1)` here is acceptable:** nothing has been allocated at parse time —
no `malloc`, no mutexes. There is nothing to leak. Once `init_sim` runs, this
stops being true, and error paths there must clean up rather than exit
abruptly. Be explicit about that distinction if asked; it shows the shortcut
was taken knowingly.

### `fill_sim`

```c
void	fill_sim(t_sim *sim, int *values, int sched)
{
	sim->num_coders = values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->compiles_required = values[5];
	sim->dongle_cooldown = values[6];
	sim->scheduler = sched;
}
```

Copies the seven validated values into named fields, plus the resolved
scheduler.

**Why a separate function:** eight assignment lines inside `parse_args` would
push it back over 25. It is also a coherent single responsibility — no
validation, no decisions, pure transfer of already-validated data.

**The bug this replaced:** an earlier version had `sim->scheduler = 0;`
hardcoded. Every run behaved as FIFO regardless of input. Every EDF test would
have silently executed as FIFO, and the tie-breaker logic would have appeared
to do nothing. Silent wrong-behaviour bugs are far more expensive than crashes,
because nothing tells you they're there.

**Why the scheduler is an enum, not the string:** `cmp_request` runs on every
heap comparison during arbitration, which happens constantly. Comparing an
`int` is far cheaper than `strcmp`. It also makes invalid states unrepresentable
after parsing — once past `parse_args`, `sim->scheduler` is provably one of two
values.

**Note on types:** `values[]` is `int`, but `time_to_burnout` etc. are `long` in
`t_sim`. The assignment widens implicitly. That's safe — every `int` fits in a
`long`. The `long` fields exist because they're added to millisecond timestamps
(which are `long`) elsewhere, and keeping types consistent avoids implicit
narrowing later.

### `parse_args`

```c
int	parse_args(int argc, char **argv, t_sim *sim)
{
	int	values[7];
	int	sched;

	if (argc != 9)
	{
		print_error("[ERROR] wrong number of arguments!\n");
		exit(1);
	}
	parse_numeric_args(argv, values);
	sched = get_scheduler(argv[8]);
	if (sched == -1)
	{
		print_error("[ERROR] scheduler must be fifo or edf!\n");
		exit(1);
	}
	fill_sim(sim, values, sched);
	return (0);
}
```

The orchestrator. Four steps, no logic of its own.

**Line by line:**

- `if (argc != 9)` — program name plus 8 mandatory arguments. Too few or too
  many are both rejected; the subject says *all arguments are mandatory*, which
  implies neither omission nor extras.
- `parse_numeric_args(argv, values);` — the seven numeric arguments. On any
  failure it exits internally, so no return check is needed here.
- `sched = get_scheduler(argv[8]);` — resolve the eighth argument.
- `if (sched == -1) { error; exit }` — invalid scheduler string.
- `fill_sim(sim, values, sched);` — copy everything into the struct.
- `return (0);` — success. Currently always 0, since every failure exits. If
  the design were changed to return error codes instead of exiting, `main`
  would check this. Be ready to say the simpler path was chosen deliberately at
  parse stage, where nothing is allocated.

**Why the scheduler is validated here rather than inside `parse_numeric_args`:**
it isn't numeric. `parse_numeric_args` has one job — parse and validate
integers. Making its loop bounds or validation conditional on argument position
is the kind of thing that silently breaks when the argument list changes.

**On the `argc` check being here and not earlier:** `main` calls `parse_args`
before anything else, so this is effectively the first check the program makes.
Any later code can assume `argv[1]` through `argv[8]` exist.

---

## 5. The full flow, traced

`./codexion 5 800 200 200 100 7 50 edf`

1. `main` zeroes `sim`, calls `parse_args(9, argv, &sim)`.
2. `argc == 9` — passes.
3. `parse_numeric_args`: for `i = 1..7`:
   - `"5"` → `is_number` yes → `ft_atoi` 5 → index 0, `5 > 0` yes → `values[0] = 5`
   - `"800"` → 800 → index 1, `>= 0` yes → `values[1] = 800`
   - ... through `"50"` → `values[6] = 50`
4. `get_scheduler("edf")` → `EDF`.
5. `fill_sim` copies everything; `sim.scheduler = EDF`.
6. Returns 0. `main` proceeds to `init_sim`.

`./codexion 5 800 200 200 100 7 50 sjf`

Steps 1–3 as above. Step 4: `get_scheduler("sjf")` → `-1`. Error printed to
stderr, `exit(1)`. Nothing allocated, nothing leaked.

`./codexion 0 800 200 200 100 7 50 fifo`

Step 3, first iteration: `"0"` → `is_number` yes → `ft_atoi` 0 → index 0
requires `> 0` → fails. Error, exit.

`./codexion 5 800 200 200 0 7 0 fifo`

Step 3: index 4 (`time_to_refactor`) is 0 → `>= 0` yes → accepted. Index 6
(`dongle_cooldown`) is 0 → accepted. Valid configuration.

---

## 6. Anticipated defense questions

**"Why do you reject zero coders but accept zero everywhere else?"**
The subject only says to reject negatives, non-integers, and invalid schedulers.
Zero isn't on that list. Zero coders is rejected because it makes the
simulation meaningless — no threads, no dongles. Every other zero is a
legitimate degenerate case: zero cooldown, instant compile, immediate
completion.

**"What happens with `number_of_compiles_required = 0`?"**
"All coders have compiled at least 0 times" is true before any thread starts.
The monitor's `check_all_done` should detect this on its first poll and stop the
simulation immediately with no output. That is consistent behaviour, and the
monitor must handle it rather than assuming at least one compile will happen.

**"What if someone passes `99999999999`?"**
`is_number` accepts it (all digits). `ft_atoi` detects the overflow via the
`long` accumulator and returns `-1`. The value check rejects `-1` as negative.
Rejected with an error, not silently wrapped.

**"Why `exit(1)` instead of returning error codes?"**
At parse time nothing has been allocated. There is nothing to clean up, so an
immediate exit is safe and simpler. After `init_sim`, this would no longer be
acceptable, and those paths do clean up.

**"Why isn't `ft_atoi` validating its input?"**
Separation of concerns. `is_number` answers "is this syntactically an integer?"
`ft_atoi` assumes yes and converts. `parse_numeric_args` always calls them in
that order. Each function does one thing.

**"Why write `ft_strcmp` instead of using `strcmp`?"**
`strcmp` is authorized, so either would be fine. Writing it removes any
ambiguity about which library functions were used, and it's five lines.

**"What does `unsigned char` do in `ft_strcmp`?"**
Matches libc's behaviour for bytes above 127, where signed `char` would give the
wrong sign. Not needed for ASCII-only input, but it's the correct general form.

**"Why is `values` an array of 7 and not a struct?"**
The seven arguments are positional and processed in a loop. An array indexed by
position matches that naturally. `fill_sim` then maps positions to named struct
fields, so the rest of the program never touches the array.