# Codexion — L-Khitab dyal Soutenance (Speech l l-Correcteur)

Had l-ktab machi documentation. Hada **chno ghadi t-gol b fommek** fach
l-correcteur ygless 7dak. Mrettb b tartib tabi3i: men l-mochkil l l-hall.

---

# PART 0 — L-Jomla d l-Bedya (30 thaniya)

Fach l-correcteur yw9ef 3endek, bda b hadi:

> *"Codexion howa simulation d concurrency b C. 3endna coders galsin f dayra,
> w kol wahed bach ycompili khassou ychedd **jouj** dongles f nefs l-wa9t:
> wahed 3la lisr dyalo w wahed 3la limn. 3adad d les dongles bhal 3adad d les
> coders, w kol dongle m-partagi bin joj jiran.*
>
> *L-mochkil l-7a9i9i machi kifach n-simuliw hadchi — l-mochkil howa **les
> problemes d concurrency** li kaytle3o menno. Ana 7ell tlata: Deadlock, Data
> Race, w Race Condition. W b7al bonus, Starvation. Ghadi n-cherhom lik
> wahed b wahed b l-code."*

Hadi kat-3ti lih l-map d l-mo7adata. W kat-wrih bli 3aref chno l-mouhim.

---

# PART 1 — L-MOCHKIL L-AWWEL: DEADLOCK

## 1.1 Chno howa (cherho b l-mital, machi b t-ta3rif)

> *"Tkhayyel 5 coders galsin f dayra. Kol wahed 3endo dongle 3la lisr w wahed
> 3la limn. Daba ila kol wahed fihom mdd yeddo w chedd l-dongle d lisr dyalo f
> **nefs l-lahda**...*
>
> *Kolchi daba chadd wahed, w kolchi kaytsenna l-akhor li chaddo jaro. Coder 1
> kaytsenna coder 2, coder 2 kaytsenna coder 3... w coder 5 kaytsenna coder 1.*
>
> *Dayra kamla. **Hta wahed ma ghadi ytele9**, hit kol wahed kaytsenna. W
> l-programme ghadi yb9a m-3elle9 l l-abad — bla crash, bla erreur, ghir
> m-bloqui. Hada howa **Deadlock**."*

## 1.2 Les 4 Conditions d Coffman (goulhom men ras, hadchi kayt-sewwel)

> *"Deadlock ma y9der yw9e3 **illa** ila t-7eqqo arb3a d les chourout f nefs
> l-wa9t:*
>
> 1. ***Mutual Exclusion** — dongle ma y9derch ykoun 3nd joj coders. Hadi ma
>    n9dro n-hressou-ha, hiya tabi3t l-mochkil.*
> 2. ***No Preemption** — ma n9dro n-fekkou dongle b zzez men coder kheddam.*
> 3. ***Hold and Wait** — coder kayched wahed w kaytsenna l-akhor.*
> 4. ***Circular Wait** — dayra d les coders kol wahed kaytsenna li men be3do.*
>
> *L-fikra hiya: **ila hressti wehda men had l-arb3a, Deadlock mostahil**. Ana
> hressit **joj**, machi wehda."*

## 1.3 L-Hall l-Awwel: Hressna "Hold and Wait"

Hna wrih l-code f `dongle.c`, fonction `request_dongles`:

```c
while (!sim_stopped(sim))
{
    if (can_take_both(coder, sim))    /* joj khawyin W ana lwel f joj les files */
    {
        take_both(coder, sim);         /* kanakhod JOUJ f nefs l-lahda */
        unlock_both(sim, a, b);
        return (0);
    }
    unlock_both(sim, a, b);            /* ma n9dertch -> kan-tele9 KOLCHI */
    usleep(200);
    lock_both(sim, a, b);              /* w kan-3awed njerreb */
}
```

> *"Chouf had l-boucle mzyan. L-coder dyali **3omro ma kayched dongle wahed w
> howa kaytsenna l-akhor**.*
>
> *Ila ma9dertch nakhod bjouj — kan-tele9 **kolchi**, kan-tsenna 200
> microsecondes, w kan-3awed njerreb men l-bedya.*
>
> *`take_both` kat-7ott `taken = 1` f **joj** les dongles, ola f **ta wahed**,
> wst nefs section critique. Ma kaynch hala f west.*
>
> *Hit ma kaynch 'Hold and Wait', coder **mostahil** ykoun 7ol9a f selsla d
> 'ana chadd X w kaytsenna Y'. W bla selsla, ma kayn ta dayra. Donc Deadlock
> mostahil."*

## 1.4 L-Hall t-Tani: Hressna "Circular Wait"

Wrih `order_dongles` f `dongle.c`:

```c
void	order_dongles(t_coder *coder, int *a, int *b)
{
    *a = coder->left_dongle;
    *b = coder->right_dongle;
    if (*a > *b)              /* dima l-id s-sgher f 'a' */
    {
        *a = coder->right_dongle;
        *b = coder->left_dongle;
    }
}
```

> *"Had l-fonction ghir kat-rettb: l-id s-sgher f `a`, l-kbir f `b`. W
> `lock_both` dima kat-verrouilla `a` 9bel `b`.*
>
> *Mital b 5 coders:*
> - *Coder 1 khassou dongles 0 w 1 -> kay-locki **0 3ad 1**.*
> - *Coder 5 khassou dongles 4 w 0 -> f 3oud ma y-locki 4 3ad 0, kay-locki
>   **0 3ad 4**!*
>
> *Hit ga3 les threads kay-verrouillaw f **nefs l-ittijah** — men l-id s-sgher
> l l-kbir — selsla d 'chkoun kaytsenna chkoun' khassha les ids ytle3ou dima.
> W selsla li kat-tle3 dima f majmou3a mahdouda **mostahil** treje3 3la rasha.*
>
> *Ya3ni: ma kayn ta dayra f les verrous."*

## 1.5 L-Jomla d l-Khtam 3la Deadlock

> *"Donc 3endi joj himayat **mosta9illin**: wehda 3la mostawa d les mutexes
> (t-tartib t-thabet), w wehda 3la mostawa d les ressources (kolchi ola walou).
> Kol wehda wehdha kafya. Ana 3endi joj."*

---

# PART 2 — L-MOCHKIL T-TANI: DATA RACE

## 2.1 Chno howa

> *"Data Race howa fach **joj threads kay-dkhlou l nefs l-blassa f la mémoire
> f nefs l-wa9t**, w 3la l-a9al wahed fihom **kaykteb**, w ma kaynch ta
> synchronisation binathom.*
>
> *L-mital l-machhour: `counter++`. Kaybane bhal 3amaliya wehda, walakin
> b l-haqiqa homa **tlata**:*
> 1. *A9ra `counter` men la mémoire.*
> 2. *Zid 1.*
> 3. *Ktebb l-natija.*
>
> *Ila joj threads dazou f nefs l-wa9t: bjouj kay9raw `5`, bjouj kay-ziido
> w kaykteb `6`. L-natija khassha tkoun `7`, w kanl9aw `6`. **3amaliya kamla
> daa3t**."*

## 2.2 Fin kan momkin yw9e3 f had l-projet

> *"F l-projet dyali, l-khatar l-7a9i9i howa bin **l-monitor w les coders**:*
>
> - *L-coder kaykteb f `last_compile_start` w f `compile_count`.*
> - *L-monitor f nefs l-lahda kay9ra dik les qiyam bach ychouf wach chi wahed
>   mat.*
>
> *Ila ma dernach mutex, l-monitor momkin y9ra qima ness-ness w y-3lan burnout
> kaddab, ola ymissi wahed 7a9i9i."*

## 2.3 L-Hall: Kol Hala Mouchtaraka 3endha Mutex Wahed

Hna 3tih l-jadwal — tkellem 3lih mbacher:

| L-mou3tayat | Chkoun kaykteb | Chkoun kay9ra | M7miya b |
|---|---|---|---|
| `dongle->taken`, `available_at`, `waiters` | ay coder | ay coder | mutex d dak l-dongle |
| `coder->compile_count`, `last_compile_start` | dak l-coder | l-monitor | `state_mutex` d dak l-coder |
| `sim->stop` | l-monitor | ga3 les threads | `stop_mutex` |
| `stdout` | ga3 les threads | — | `log_mutex` |

Wrih `record_compile` f `coder.c`:
```c
pthread_mutex_lock(&coder->state_mutex);
coder->compile_count++;
coder->last_compile_start = start;
pthread_mutex_unlock(&coder->state_mutex);
```

> *"Joj les champs kayt-bedlou wst **nefs** l-verrou. 3lach machi kol wahed
> b verrou dyalo? Hit l-monitor kay9rahom **bjouj**. Ila fer9nahom, l-monitor
> momkin ychouf `count` jdid m3a `start` 9dim — w hadi hala mostahila f
> l-haqiqa."*

## 2.4 L-Indibat li kaykhlli hadchi yb9a s7i7

Hadi noqta qawiya bzaf, goulha:

> *"Li kaykhlli hadchi ykhdem machi ghir les mutexes — howa **l-indibat**: ma
> kayn ta blassa f l-code fin kay-t-mess had les champs **direct**.*
>
> - *`sim->stop` kayt9ra **ghir** b `sim_stopped()` — fonction wehda, m7miya.*
> - *`printf` kayt-3eyyet **merra wehda f ga3 l-projet**, wst `log_state`.*
>
> *Ila derna hadchi f 20 blassa, chi merra ghadi nnsaw l-verrou. B fonction
> wehda, mostahil."*

## 2.5 L-Ithbat (wrih b l-outils)

```bash
valgrind --tool=helgrind ./codexion 5 100000 200 200 200 3 0 fifo
valgrind --tool=drd      ./codexion 5 100000 200 200 200 3 0 fifo
cc -fsanitize=thread -g -Wall -Wextra -pthread *.c -o cx_tsan && ./cx_tsan 5 100000 200 200 200 3 400 edf
```

> *"Tlata d les outils, kollhom kay-3tiw **0 errors**.*
>
> *W noqta tiqniya: m3a Valgrind khassna `time_to_burnout` **kbir** (100000),
> hit Valgrind kaybetti l-programme b 10 hta 50 merra. B 2000, kolchi ghadi
> ymout f l-blassa w l-outil ma ghadi ychouf walou."*

---

# PART 3 — L-MOCHKIL T-TALET: RACE CONDITION

## 3.1 L-Farq (had s-sou2al kaytsewwel bzaf — 7fedh had l-jomla)

> *"Bezzaf d n-nass kay-khelltou binathom, walakin machi nefs l-haja:*
>
> - ***Data Race** howa hala **tiqniya d la mémoire**: wosoul m-t9ate3 bla
>   synchronisation. Hada howa li kay-l9awh Helgrind w ThreadSanitizer.*
>
> - ***Race Condition** howa **a3amm**: l-natija d l-programme katbeddel 7sab
>   l-timing w katwelli **ghalta**.*
>
> *W l-mouhim: **momkin ykoun 3endek Race Condition bla ta Data Race** —
> kolchi m7mi b mutex, ga3 les outils kay-3tiw 0 errors, w l-mantiq ba9i
> ghalat."*

## 3.2 L-Mital l-7a9i9i men l-projet dyali (hadi hiya l-qenbala)

Hada howa a7san chi 3endek. Bug 7a9i9i li l9iti w fektih.

> *"3endi mital 7a9i9i men had l-projet. F version 9dima, `record_compile`
> kan kayt-3eyyet **men be3d** n3as d l-compile:*

```c
start = now_ms();
precise_sleep(sim, sim->time_to_compile);   /* 200ms */
release_dongles(coder, sim);
record_compile(coder, start);    /* GHALAT — l-magana kat-reseta men be3d */
```

> *Kolchi kan m7mi b `state_mutex`. **Ma kanch data race**. ThreadSanitizer
> kan kay-3ti 0 errors.*
>
> *Walakin `last_compile_start` kan kaybqa b l-qima l-9dima **tul l-compile
> kamla** — 200 milliseconde."*

Hna rsem l-jadwal 3la wer9a ila 3endek stylo:

| L-wa9t | L-haqiqa | Chno kan kaychouf l-monitor |
|---|---|---|
| t=0 | coder 1 bda ycompili | `last_compile_start = 0` |
| t=200 | l-compile sala | `last_compile_start = 0` |
| t=400 | coder 1 bda ycompili **men jdid** | ba9i `0`! ma kayt-bddelch hta t=600 |
| t=411 | coder 1 **kaycompili daba** | elapsed 411 > 410 -> **burnout kaddab** |

> *"L-monitor 3lan bli coder mat, w howa **kaycompili dak l-wa9t b 3ayn-o**.*
>
> *L-hall ma kanch f l-verrou — kan f **t-tartib**:"*

```c
start = now_ms();
record_compile(coder, start);    /* l-magana kat-reseta f l-lahda fash BDA */
log_state(sim, coder->id + 1, "is compiling");
precise_sleep(sim, sim->time_to_compile);
```

> *"L-3ibra: 'm7mi b mutex' **machi** nefs l-haja bhal 's7i7'. T-tartib
> kayhemm."*

## 3.3 Joj Amtila akhrin d Race Condition li 7ellithom

### (a) Check-then-Act

> *"Ila chefna 'wach had l-dongle khawi?' f blassa, w men be3d 'ana ghadi
> nakhdo' f blassa akhra — bin joj, chi coder akhor momkin yakhdo.*
>
> *F l-code dyali, joj l-3amaliyat kaydouzo wst **nefs** hold d les mutexes:"*

```c
if (can_take_both(coder, sim))   /* t-tcheki */
{
    take_both(coder, sim);        /* l-akhd — nefs l-verrou, bla ta fejwa */
```

> *"Ta 7ed ma y9der ydkhol binathom."*

### (b) L-Boucle `while` machi `if`

> *"Fach l-coder kay-tele9 kolchi w kay-3awed yjerreb, kay-3awed **y-verifi
> l-hala men l-jdid** — ma kay-3tmedch 3la mou3tayat 9dima li chafhom 9bel
> ma yn3ess."*

---

# PART 4 — L-MOCHKIL R-RABE3: STARVATION

## 4.1 Chno howa

> *"Starvation machi Deadlock. F Deadlock, **kolchi** m-bloqui. F Starvation,
> l-programme kheddam mzyan — walakin **chi coder wahed** kaybqa dima
> me7roum, w kol merra chi wahed akhor kaysbe9. F l-akhir, dak l-coder
> kaymout b l-burnout hta ila kan l-programme kolo kheddam."*

## 4.2 L-Hall: L-Priority Queue (Min-Heap)

> *"Kol dongle 3endo **file d intidar** dyalo howa — Min-Heap mektoub b yeddi,
> machi men chi librairie (le sujet kaymne3).*
>
> *Chkoun kayakhod l-dongle? 7sab l-scheduler:*
>
> - ***FIFO**: li tlba **9bel** (arrival sgher).*
> - ***EDF**: li **9reb ymot** (deadline sgher), ya3ni
>   `last_compile_start + time_to_burnout`."*

## 4.3 L-Comparateur (wrih `cmp_request` f `heap_utils.c`)

```c
if (sched == FIFO)
{
    if (a->arrival != b->arrival)
        return (a->arrival < b->arrival);
    return (a->coder_id < b->coder_id);
}
if (a->deadline != b->deadline)
    return (a->deadline < b->deadline);
if (a->arrival != b->arrival)
    return (a->arrival < b->arrival);
return (a->coder_id < b->coder_id);
```

> *"Had la fonction **wehdha** hiya l-politique d l-scheduling kamla.*
>
> - *FIFO: `arrival` -> `coder_id`.*
> - *EDF: `deadline` -> `arrival` -> `coder_id`."*

## 4.4 3lach selsla d les tie-breakers (hadi kayt-sewwel)

> *"Joj asbab:*
>
> ***L-awwel — l-égalité machi nadira.** `now_ms()` 3endha precision d
> millisecondes. Joj coders li tlbou f nefs l-milliseconde 3endhom **nefs**
> `arrival`. Joj coders li bdaw ycompiliw f nefs l-milliseconde 3endhom
> **nefs** `deadline`. Le sujet b 3ayn-o kay7der men hadchi.*
>
> ***T-tani — 3lach EDF khassou `arrival` f west, 9bel `coder_id`?** Ila
> 9fezna direct l `coder_id`, l-coder b ra9m sgher ghadi yrbe7 **l l-abad**
> m3a nefs l-jar. Machi merra — dima, hta t-fer9o les deadlines. Hadi
> **starvation systematique**, w le sujet kaymne3-ha b ddabt.*
>
> ***W 3lach `coder_id` f l-lakher?** Hadi noqta tiqniya: comparateur d heap
> khassou ykoun **total w deterministe**. Ila joj demandes mkhtalfin momkin
> ykounou 'la hada 9bel hada w la 3keso', les operations d sift ma 3endhom ta
> 3ela9a thabta — w l-heap momkin **ytkhesser f la structure dyalo**. Machi
> ghir l-jawab ghalat, l-arbre b 3ayn-o kayt-kessar. `coder_id` farid, donc
> l-jawab dima wadeh."*

## 4.5 Bug d Starvation li l9itou b se77

> *"3endi mital 7a9i9i. F version 9dima, l-timestamp d `arrival` kan kayt-akhed
> **f kol dongle 3la hda**. Hit l-coder kaywsel l t-tani dongle men be3d
> l-awwel, l-arrival dyalo f t-tani dima kan m-t2akher men dyal l-jar li 3ndo
> dak l-dongle bhal **awwel** dongle.*
>
> *Donc f FIFO, l-jar kan kayrbe7 **kol merra**, w l-mochkil kan kaydour f
> d-dayra kamla.*
>
> ***L-a3rad li chefna**: coder kaycompili 10 d les merrat w akhor ghir 4 w men
> be3d kaymout.*
>
> ***L-hall**: `request_dongles` daba kayakhod **timestamp wahed** f l-bedya
> w kay-passih l joj les files. Blasst l-coder f la file walat kat-wesf **fash
> bgha ycompili**, machi fash wsel l dak l-dongle b ssodfa."*

---

# PART 5 — L-MOCHKIL L-KHAMES: L-PRECISION D L-BURNOUT

## 5.1 Chno matloub

> *"Le sujet kaytleb: satr d `burned out` khassou yt-tbe3 f **9ell men 10
> millisecondes** men l-lahda l-7a9i9iya d l-mot."*

## 5.2 L-Hall: Thread Mosta9ill (Monitor)

```c
while (1)
{
    if (check_burnout(sim))     break ;
    if (check_all_done(sim))    break ;
    usleep(300);
}
```

> *"3endi thread mosta9ill kaydor kol **300 microsecondes** — 0.3 milliseconde.
> Hadchi kat-3tina tqriban **33 merra** hamech m9arna b 10ms.*
>
> *L-9yas l-7a9i9i: b `time_to_burnout = 800`, l-log kayt-tbe3 f **801ms**.
> B `500` -> **501ms**. Dima 1ms."*

## 5.3 T-Tartib l-Mouhim: Log 9bel l-Flag

Hadi noqta d99i9a, w kat-wri bli 3aref l-code dyalek:

```c
if (elapsed > sim->time_to_burnout)
{
    log_state(sim, i + 1, "burned out");   /* LOG LWEL */
    stop_sim(sim);                          /* W MEN BE3D L-FLAG */
    return (1);
}
```

> *"3lach had t-tartib b ddabt? Hit `log_state` kaybda b:"*

```c
if (sim_stopped(sim))
    return ;
```

> *"Had l-garde hiya li kat-dmen bli satr d `burned out` howa **aakher satr**
> f l-affichage — men be3d ma yt-7ott l-flag, walou ma kayt-tbe3.*
>
> *Walakin ila 3eyyetna l `stop_sim` **lwel**, satr d `burned out` — li howa
> **a-ham satr f l-projet kamel** — ghadi ykhebbih dak l-garde b 3ayn-o!*
>
> *Donc: n-tbe3 lwel, w men be3d n-refed l-flag."*

---

# PART 6 — L-MOCHKIL S-SADES: LES HALAT L-KHASSA

## 6.1 Coder Wahed (`n = 1`)

> *"Had l-hala hiya li kay-testiw biha lwel.*
>
> *F `init_coders`: `left = i`, `right = (i + 1) % n`. Ila `n = 1`:
> `right = (0 + 1) % 1 = 0`. Donc **joj homa nefs l-dongle**.*
>
> *Coder khassou ychedd nefs l-dongle b joj yeddih — **mostahil fizyaiyan**.
> Donc ma ghadi ycompili hta merra, w ghadi ymout f `time_to_burnout`.*
>
> ***L-jouz l-hlou**: hadchi kharej **wehdo** men l-formule. Ma dernach ta `if`
> f l-init."*

Walakin wrih l-himaya f `request_dongles`:
```c
if (coder->left_dongle == coder->right_dongle)
    return (wait_single_dongle(sim));
```

> *"3lach had t-tcheki darori? Ila khellinah y-dkhol l-boucle l-3adiya, ghadi
> y-pushi **nefs l-coder jouj merrat f nefs l-heap**, w `lock_both` ghadi
> t-jerreb t-locki **nefs l-mutex jouj merrat** = **self-deadlock**, hit
> les mutexes d POSIX machi récursifs b l-défaut.*
>
> *L-natija: `801 1 burned out`, w l-programme kaysali cleanly bla crash w bla
> freeze."*

## 6.2 L-Cooldown

> *"Le sujet kaytleb: men be3d ma coder ytele9 dongle, dak l-dongle ma y9derch
> yt-akhed hta t-douz `dongle_cooldown` millisecondes."*

```c
/* f release_dongles */
sim->dongles[a].available_at = now + sim->dongle_cooldown;

/* f is_free */
return (d->taken == 0 && now >= d->available_at);
```

> *"Joj chourout, bjouj daroriyin: ma chaddo hta wahed **W** l-cooldown sala.*
>
> *L-cooldown kollo f **satr wahed**. Ila kan wahed men joj khayeb,
> `can_take_both` kat-retoorner `0`, l-coder kay-tele9 kolchi w kay-3awed
> yjerreb men be3d 200 microsecondes."*

## 6.3 L-Khrouj n-Ndif (Clean Shutdown)

Had s-selsla goulha b tartib — hiya s-sabab 3lach `pthread_join` 3omrha ma
kat-bloqui:

> *"Fach l-monitor y-9errer l-w9ouf:*
>
> 1. *`stop_sim` kay-refed `sim->stop = 1` wst `stop_mutex`.*
> 2. *Kol coder f l-boucle d `request_dongles` kay-verifi `sim_stopped()` f
>    **kol dawra** — kol 200 microsecondes. Kaykherj, kay-tele9 les mutexes,
>    kay-retoorner `1`.*
> 3. *`do_compile` kaychouf `1` -> kay-retoorner **bla ma ycompili** w **bla ma
>    y-tele9 walou** — hit ma chadd walou.*
> 4. *`coder_routine` kay-briki men l-boucle l-kbira.*
> 5. *Les coders li kano wst `precise_sleep` kaykherjo f 9ell men 300µs, hit
>    homa tani kay-verifiw l-flag f kol dawra.*
> 6. *Ga3 les threads kay-retoornaw `NULL` -> `join_all` kaykemmel.*
> 7. *`cleanup_all` kayhdem ga3 les mutexes w kay-libéri la mémoire — **b
>    aman**, hit ta wahed ma ba9i kheddam bihom.*
>
> *Valgrind kay-3ti: `All heap blocks were freed — no leaks are possible`."*

---

# PART 7 — AS2ILA S3IBA W L-AJWIBA DYALHOM

## Q: "3lach ma kheddamtich `pthread_cond_t`?"

> *"Le sujet gal: 'A condition variable **may** be used' — **may**, machi
> **must**. Donc machi ijbari.*
>
> *W 3endi hoja tiqniya: m3a l-akhd 'kolchi ola walou', ma kaynch **7adita
> wehda** li n9der n-signali 3liha. L-coder kaytsenna 3la:*
> - *hala d **joj** dongles f nefs l-wa9t,*
> - ***w** joj magana d cooldown.*
>
> *W intiha2 d cooldown **machi 7adita** li chi thread y9der y-broadcasti-ha —
> hiya ghir **morour d l-wa9t**. Chi wahed khassou y3awed ychouf l-magana f ay
> hal.*
>
> *Donc l-polling b `usleep(200)` kaykhlli hadchi **wadeh**, w kay-7iyyed
> famila kamla d les bugs: missed wakeup, lost signal, spurious wakeup.*
>
> *200 microsecondes homa 50 merra sgher men l-hamech d 10ms."*

## Q: "3lach Min-Heap w machi tableau 3adi?"

> *"F tableau 3adi, bach n3ref chkoun mol l-asbaqiya khassni n-boucli 3la ga3
> les elements — $O(N)$ f kol merra. F tableau m-rettb, kol insertion khassha
> t-7ayyed les elements — tani $O(N)$.*
>
> *F Min-Heap:*
> - *`heap_peek`: dima f **index 0** -> $O(1)$.*
> - *`heap_push` w `heap_pop`: $O(\log N)$.*
>
> *W zid, le sujet kaytleb **explicitement** priority queue mektouba b l-yedd."*

## Q: "Ghir joj coders kay-sta3mlou kol dongle — wach l-heap machi ziyada?"

Had s-sou2al dki. Jaweb b sara7a:

> *"S7i7 — dongle `j` kay-sta3mlouh ghir coder `j` w coder `(j-1+n) % n`. Donc
> l-file ma kat-fout **jouj** demandes.*
>
> *Walakin le sujet kaytleb priority queue b l-3ibara, w l-version l-3amma ma
> kat-swa kter men version khassa b joj blayess — w kat-7iyyed famila kamla d
> les halat l-khassa."*

## Q: "Chnahiya l-khassiya d l-heap b ddabt?"

> *"Kol **bah** 3ndo asbaqiya kter men **wladou bjouj**.*
>
> ***Mlahada mouhimma**: t-tableau **machi m-rettb**! Ghir dik l-3ela9a
> l-mahalliya bin bah w wladou. W hadchi kafi — hit s-sou2al l-wahid li
> kansowwlouh dima howa 'chkoun f l-wejh?', w l-jawab dima f **index 0**.*
>
> *W 3la haqqach les 3amaliyat homa $O(\log N)$ w machi $O(N)$ — kan-rettbo
> b9ell bzaf w 3endna daba li khassna."*

## Q: "Kifach t-beddel `fifo` l `lifo`?" (L-Recode)

> *"**7arf wahed.**"*

F `heap_utils.c`, `cmp_request`:
```c
return (a->arrival < b->arrival);   /* FIFO */
return (a->arrival > b->arrival);   /* LIFO */
```

> *"`<` -> `>`, w men be3d `make`.*
>
> ***3lach hadi hiya l-blassa s7i7a?** L-heap kayt-3eyyet l `cmp_request` f
> **kol** `sift_up` w `sift_down` — w hadchi howa li kay-9errer chkoun kaygless
> f **index 0**, ya3ni chkoun f wejh l-file. W `can_take_both` kay-sowwel
> `heap_peek` bach ychouf chkoun lwel.*
>
> *Donc t-beddil kaybeddel **chkoun kayakhod l-dongle b se77**, machi ghir chno
> kayt-tbe3.*
>
> ***Jomla mouhimma**: `sift_up` w `sift_down` ma fihom **ta kelma** 3la FIFO
> ola EDF. L-politique kamla galssa f `cmp_request` wehdha."*

## Q: "3lach chi merrat kaymout m3a cooldown 800?"

Goul hadi **9bel** ma y-l9aha howa:

> *"Hadi **riyadiyat**, machi bug. Khellini n-wrih lik l-hissab:*
>
> - *Dayra d 5 coders kat-smeh b **jouj** compiles f nefs l-wa9t ghir hiya
>   (ghir les coders li machi jiran).*
> - *Kol compile kaykhelli 2 dongles machi mawjoudin: 200ms d l-compile + 800ms
>   d l-cooldown = **1000ms**.*
> - *Donc les dawrat kaykounou b3ad 1000ms, w kol dawra kat-khdem 2 coders.*
> - *Bach ga3 les 5 yakhdou dorhom: 3 dawrat (2+2+1) = **3000ms**.*
> - *`time_to_burnout` = **3000ms**. **Hamech = ZERO.***
>
> *Ta chi implementation ma t9der t-dmen had l-hala.*
>
> *M9arna m3a cooldown 400: dawrat kol 600ms, dawra kamla 1800ms m9abil
> 3000ms -> **hamech d 40%** — w 3la haqqach dik l-hala kat-nja7 dima.*
>
> *W l-feuille d correction b 3ayn-ha kat-tleb 'bla burnout' **ghir** l l-hala
> d 400. L-hala d 800 kat-tleb ghir t9aren t-tartib bin fifo w edf."*

## Q: "3lach 3adad d 'is compiling' kbir 3la l-quota?"

> *"`check_all_done` kat-w9ef ghir fach **ga3** les coders ywsslou, machi fach
> l-awwel ywsel. W l-monitor kay-tcheki kol 300µs, machi f l-lahda b ddabt.*
>
> *Bin l-lahda fash aakher coder wsel w l-lahda fash l-monitor lah, les coders
> li kano **f west compile** kaysaliw dik l-compile w kay-tbe3ouha.*
>
> *Donc l-3adad howa **3la l-a9al** `n × compiles_required`. W hada s7i7 — le
> sujet kaytleb ghir 'at least'."*

## Q: "Wrini bli ma 3endekch variables globales."

> *"Ga3 l-hala mouchtaraka galssa f `t_sim`, w `t_sim` hiya variable f **la
> pile** f `main`, kat-passa b l-pointeur. Kol coder kaywsel lha b
> `coder->sim`.*
>
> ***3lach f la pile machi b `malloc`?** Wehda ghir hiya, w kat3ich l-mouddat
> l-programme kamla. `main` ma kaysalich hta ykmmel `join_all`, donc ga3 les
> threads kay-l9aw dik la mémoire mawjouda w sahiha."*

## Q: "Chno kayw9e3 ila fchel `malloc` f west l-init?"

Jaweb b sara7a — w goul l-noqta d-daifa b rasek:

> *"`main` kaybda b `memset(&sim, 0, sizeof(t_sim))`, donc ga3 les pointeurs
> homa `NULL` hta ma yt-allocaw. `cleanup_all` kat-khdem b aman f ay no9ta:
> `free(NULL)` ma kaydir walou, w `heap_destroy` kat-verifi `NULL`.*
>
> ***L-noqta d-daifa — w n-gouliha b sara7a**: `pthread_mutex_destroy` 3la
> mutex li ma t-initialisach howa undefined behavior. Machi pointeur, donc
> `NULL` ma kat-3awen. L-hall l-kamel howa 3addad d li nja7. F l-3amaliya 3la
> glibc, mutex b zero kaybane bhal wahed statically-initialised — walakin hadi
> tafsila d l-implementation machi dmana."*

---

# PART 8 — T-TARTIB LI GHADI T-TBE3 F S-SOUTENANCE

Hada howa l-plan d l-mo3ada tabi3i:

1. **Compilation**: `make` — 0 warnings b `-Wall -Wextra -Werror -pthread`.
2. **Norminette**: `norminette *.c *.h` — 13/13 OK.
3. **Variables globales**: wrih `t_sim` f `main`.
4. **Les 3 tests l-3adiyin**:
   - `./codexion 1 800 200 200 200 10 0 fifo` -> `801 1 burned out`
   - `./codexion 5 2000 200 200 200 10 0 fifo` -> bla burnout
   - `./codexion 5 2000 200 200 200 7 0 edf` -> bla burnout
5. **T-test d l-burnout**: `./codexion 5 500 200 200 200 10 0 fifo` -> `501`,
   w howa **aakher satr**.
6. **L-cooldown**: `./codexion 5 3000 200 200 200 10 400 fifo` -> bla burnout.
7. **L-outils**: Valgrind, Helgrind, DRD, ThreadSanitizer.
8. **Les arguments ghalatin**: 6 d les halat, kollhom kayt-refdou.
9. **L-Recode**: FIFO -> LIFO, 7arf wahed, w wrih l-farq f les logs.

---

# PART 9 — TLATA D L-QAWA3ID L L-MO3ADA

**1. Ma t-3eyyetch — wrih l-code.**
Fach y-sowlek, hell l-fichier w dker l-satr. "Hadi hiya f `dongle.c`, f
`request_dongles`, chouf had l-boucle." Kter men chi jawab m-7foud.

**2. Ila ma 3reftich — goul ma 3reftch.**
"Hadi ma tfe99ertch fiha mzyan, walakin l-fikra li 3endi hiya..." — hadchi
a7san bzaf men jawab mkhrbe9. L-correcteurs kay-3erfou l-farq.

**3. Goul les no9at d-daifa 9bel ma y-l9ahom howa.**
L-hala d cooldown 800 (riyadiyat), w
`pthread_mutex_destroy` f l-bina n-ness-ness. Fach t-goulhom b rasek,
kaybano **fahm**, machi **ghafla**. Fach howa li y-l9ahom, kaybano **bug**.
