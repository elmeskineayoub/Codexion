# Chreh Mfessel dyal Projet Codexion (EXPLANATION.md)

---

## 1. Chno howa had l-projet (What the project is)

Had l-projet smito **Codexion**. Howa simulation multithreading mktouba b
l-langage **C**, mbenya 3la l-mochkil l-ma3rouf f l-informatique dyal
**Dining Philosophers**.

### L-Fikra l-3amma:
- Tkhayyel 3endna wahed l-group dyal les développeurs (**coders**) galsin f
  tabla dayra f un espace de co-working.
- Bach ay coder y9der ycompili l-code dyalo, khassou darori yakhod **jouj
  hardware USB dongles**: wahed 3la limn dyalo w wahed 3la lisr dyalo.
- 3adad d les dongles howa **bhal bhal** m3a 3adad d les coders. Kol dongle
  galess bin joj coders jiran.
- Hit kolla coder kaypartagi les dongles m3a l-jiran dyalo, joj coders las9in f
  ba3diyat-hom ma y9drouch ycompiliw f nefs l-wa9t.

### L-Boucle li kaydir kolla Coder:
1. **Compiling**: Kayakhod 2 dongles, w kaycompili pendant `time_to_compile`.
2. **Debugging**: Kaytele9 les 2 dongles bach ykhlli jirano ykhedmou, w kaygless
   ydebuggi (`time_to_debug`).
3. **Refactoring**: Kaygless y-refactori (`time_to_refactor`) w men be3d
   kayrje3 ytleb les dongles men jdid.

### L-Khatar (Burnout):
- Kolla coder 3ndo wahed l-wa9t maxi smito `time_to_burnout`.
- L-qa3ida b ddi99a: l-moudda bin **l-bedya d compilation wehda w l-bedya d
  compilation li men be3dha** ma khassha t-fout `time_to_burnout`.
- Ila fatet, kayw9e3 lih **burnout**, w l-simulation kamla katw9ef f l-blassa.
- L-coder li ma zal ma compilach hta merra, l-hissab kaybda men **l-bedya d
  l-simulation** (`start_time`).

### L-Hadaf dyal l-projet:
N-gériw les threads w les ressources partagées bla ma yw9e3:
- **Deadlock**: Kol wahed chad dongle w kaytsenna lakhor ytele9, w kolchi
  ytbloqua l l-abad.
- **Data Race**: Jouj threads ykhedmou 3la nefs la variable f nefs l-wa9t bla
  mutex.
- **Race Condition**: L-natija d l-programme katbeddel 7sab l-timing, hta ila
  kan kolchi m7mi b mutex.
- **Starvation**: Chi coder yb9a dima me7roum w maywessloch nobto.

---

## 2. Chno kaydir kolla fichier (What each file does)

| Fichier | L-Khedma dyalo |
|---|---|
| `codexion.h` | Header file fih ga3 les structures (`t_request`, `t_heap`, `t_dongle`, `t_coder`, `t_sim`), l-enum d l-scheduler, w les prototypes d ga3 les fonctions. |
| `main.c` | L-point d'entrée (`main`) w `launch_threads`. Kay-parsi, kay-initialisi, kay-lanci les threads, kaytsennahom, w kaynqqi f l-lakher. |
| `parse_utils.c` | Des fonctions d base 3la les strings w les nombres: `is_digit`, `ft_strlen`, `ft_strcmp`, `ft_atoi`, `is_number`. Ma 3endhom ta 3ela9a b Codexion, khddamin f ay programme C. |
| `parse.c` | L-verification d les 8 arguments li 3endhom 3ela9a b had l-projet: `print_error`, `get_scheduler`, `parse_numeric_args`, `fill_sim`, `parse_args`. |
| `init.c` | Bina d l-simulation: `init_sim`, `init_dongles`, `init_coders`, w `join_all`. |
| `init_utils.c` | Tenzil w tenqiya: `cleanup_dongles` w `cleanup_all`. |
| `utils.c` | Fonctions d l-moussa3ada: `now_ms` (l-wa9t b ms), `precise_sleep` (n3as dki), `log_state` (ktaba m7miya b mutex), `sim_stopped` (9raya m7miya d l-flag). |
| `heap.c` | L-algorithme d l-**Min-Heap**: `sift_up`, `heap_push`, `sift_down`, `heap_pop`, `heap_peek`. |
| `heap_utils.c` | Bina w hedm d l-heap w l-comparateur: `swap`, `cmp_request`, `heap_create`, `heap_destroy`. |
| `dongle.c` | L-qelb d l-projet: `order_dongles`, `can_take_both`, `take_both`, `request_dongles`, `release_dongles`. |
| `dongle_utils.c` | Les outils d l-acquisition: `build_request`, `lock_both`, `unlock_both`, `is_free`, `wait_single_dongle`. |
| `coder.c` | Routine d kolla coder (thread loop): `do_compile`, `do_phase`, `record_compile`, `coder_routine`. |
| `monitor.c` | Routine d l-3essas: `stop_sim`, `check_burnout`, `check_all_done`, `monitor_routine`. |

**Mlahada mouhimma**: 3endna 13 d les fichiers, w kol fichier fih **5 d les
fonctions ola 9ell**, hit la Norme dyal 42 ma kat-smehch b kter men 5.

---

## 3. Chno kadir kolla fonction b tafsil (What each function does)

---

### A. `main.c`

#### 1. `launch_threads(t_sim *sim)`
- **Chno kadir**: Kat-lanci thread dyal kolla coder wahed b wahed b
  `pthread_create`, w men be3d kat-lanci thread dyal l-monitor.
- **Chno kan-passiw l kol thread**: L coder kan-passiw `&sim->coders[i]`
  (l-adresse d dik l-structure b 3ayniha), w l l-monitor kan-passiw `sim`
  direct.
- **3lach les coders 9bel l-monitor**: Bach l-monitor ma ybdach y-tcheki
  l-burnout 3la threads ma zal ma t-creaw.
- **Chno katretoorner**: `0` f nja7, `1` ila fchel chi `pthread_create`.

#### 2. `main(int argc, char **argv)`
- **Chno kadir**:
  1. `memset(&sim, 0, sizeof(t_sim))` — kat-msseh la structure kamla b zero.
  2. `parse_args` — kat-verifi les arguments.
  3. `init_sim` — kat-bni kolchi (mutexes, dongles, coders).
  4. `launch_threads` — kat-lanci les threads.
  5. `join_all` — kat-tsennahom kamlin.
  6. `cleanup_all` — kat-nqqi kolchi.
- **3lach `memset` f l-bedya**: `t_sim sim` hiya variable f la pile (stack), w f
  C kat-3mer b **ghbra** (garbage) machi b zero. Ila fchel chi haja f
  `init_sim` w 3eyyetna l `cleanup_all`, ghadi ydir `free()` 3la pointeur fih
  ghbra -> **crash**. B l-`memset`, ga3 les pointeurs kaykounou `NULL`, w
  `free(NULL)` ma kaydir walou (safe).
- **3lach `t_sim` f la pile machi b `malloc`**: 3endna wahda ghir hiya, w
  kat3ich l l-programme kamel. `main` ma kaysalich hta ykmmel `join_all`,
  donc ga3 les threads kay-l9awha mawjouda.

---

### B. `parse_utils.c`

#### 1. `is_digit(char c)`
- **Chno kadir**: Kat-chouf wach l-7arf howa ra9m men `'0'` l `'9'` (ASCII 48 l 57).
- **Chno katretoorner**: `1` ila howa ra9m, `0` ila la.

#### 2. `ft_strlen(char *str)`
- **Chno kadir**: Kat-7sseb chhal d les caracteres f string hta l `'\0'`.
- **Fin kheddama**: Ghir f `print_error`, hit `write` khassha tsift l-3adad.

#### 3. `ft_strcmp(char *s1, char *s2)`
- **Chno kadir**: Kat-9aren joj strings 7arf b 7arf.
- **Chno katretoorner**: `0` ila bhal bhal, ra9m machi zero ila mkhtalfin.
- **3lach `(unsigned char)`**: F x86, `char` howa **signé**, donc ay byte kbir
  men 127 kaybqa negatif. B l-cast l `unsigned char` kanta2kdo bli l-natija
  3endha nefs l-ma3na bhal `strcmp` l-asli.

#### 4. `ft_atoi(char *str)`
- **Chno kadir**: Kat-7owel string l `int`, m3a protection men l-overflow.
- **3lach `result` howa `long` machi `int`**: Ila kan `int`, `result * 10` ghadi
  y-déborde **9bel** ma n9dro n-tchekiwh, w l-overflow f C howa undefined
  behavior. B `long` 3endna blassa kafya bach n-chouf l-mochkil 9bel ma yw9e3.
- **Chno katretoorner**: L-ra9m, ola `-1` ila kan overflow.
- **Mlahada s7i7a**: `-1` momkin tkoun natija sahiha d string `"-1"`. Walakin
  hna ma mochkil hit ga3 les arguments kayrefdou les nombres negatifs, donc f
  jouj l-halat kay-t-rejeta.

#### 5. `is_number(char *str)`
- **Chno kadir**: Kat-verifi wach l-string howa ra9m s7i7: signe optionnel, men
  be3d **3la l-a9al ra9m wahed**, w men be3d ghir les chiffres.
- **L-satr l-mouhim**: `if (!is_digit(str[i])) return (0);` li kayji 9bel
  l-boucle. Bla had l-satr, string `"-"` ghadi t3di! Hit ghadi n9fzo 3la
  l-signe, w l-boucle ma t-dor hta merra, w t-retoorner `1` — w hada ghalat.
- **Kat-refed**: `""`, `"-"`, `"+"`, `"abc"`, `"12abc"`, `"1.5"`, `"--5"`.

---

### C. `parse.c`

#### 1. `print_error(char *msg)`
- **Chno kadir**: Kat-tbe3 message d l-erreur f **fd 2** (stderr) b `write`.
- **3lach `write` machi `printf`**: `write` howa syscall direct bla buffer.
  `printf` kaykhbbi l-texte f buffer, w ila derna `exit(1)` men be3d, momkin
  l-message ydi3 9bel ma ykherj.
- **3lach fd 2 machi fd 1**: Bach les erreurs ma ykhelltouch m3a les logs d
  l-simulation f stdout.

#### 2. `get_scheduler(char *str)`
- **Chno kadir**: Kat-7owel `"fifo"` l `FIFO` w `"edf"` l `EDF`, w `-1` l ay
  haja akhra.
- **3lach `-1` khddama bhal sentinel**: Hit l-enum kaybda men `0`
  (`FIFO = 0`, `EDF = 1`), donc `-1` mostahil tkoun natija sahiha.
- **L-mou9arana daqiqa**: `"FIFO"` ola `"Fifo"` kayt-refdou. Le sujet gal
  *"exactly one of: fifo or edf"*.

#### 3. `parse_numeric_args(char **argv, int *values)`
- **Chno kadir**: Kat-dor men `argv[1]` hta `argv[7]` (7 arguments raqmiyin) w
  kat-7ott-hom f `values[0..6]`.
- **L-verification**:
  ```c
  if (i - 1 == 0)
      verified = (values[i - 1] > 0);   /* num_coders khassou > 0 */
  else
      verified = (values[i - 1] >= 0);  /* l-ba9i zero msmouh */
  ```
- **3lach zero msmouh**: Le sujet gal ghir *"Reject invalid inputs such as
  negative numbers, non-integers, or a scheduler other than fifo or edf."* —
  ma hderch 3la zero. Donc zero msmouh fin kaykoun 3ndo ma3na (mital
  `time_to_refactor = 0` ola `dongle_cooldown = 0`). Ghir `num_coders` li khassou
  ykoun kbir men zero, hit b zero coders ma kayn ta chi haja n-simuliw.
- **3lach `exit(1)` hna msmouh**: F had l-lahda ma zal ma dirna hta `malloc` w
  ta mutex, donc ma kayn ta chi haja n-libériw. Men be3d `init_sim`, hadchi
  kaytbeddel w khassna n-nqqiw 9bel ma nkherjou.

#### 4. `fill_sim(t_sim *sim, int *values, int sched)`
- **Chno kadir**: Kat-nqel les 7 valeurs men l-tableau l les champs b smiythom
  f `t_sim`, w kat-7ott l-scheduler.
- **3lach fonction bo7dha**: 8 d les assignations wst `parse_args` ghadi
  y-dépassiw 25 satr d la Norme. W zid, "n-copiw les valeurs" hiya khedma
  wehda w wadha.

#### 5. `parse_args(int argc, char **argv, t_sim *sim)`
- **Chno kadir**: 4 khtawat: `argc == 9`? -> `parse_numeric_args` ->
  `get_scheduler` -> `fill_sim`.
- **3lach `argc == 9`**: Smiyt l-programme + 8 arguments daroriyin.

---

### D. `init.c`

#### 1. `init_sim(t_sim *sim)`
- **Chno kadir**:
  1. `sim->stop = 0` — l-simulation kheddama.
  2. Kat-initialisi `stop_mutex` w `log_mutex` (b verification d l-erreur).
  3. `sim->start_time = now_ms()` — **l-lahda zero** d l-simulation kamla.
  4. `init_dongles` w men be3d `init_coders`.
  5. `malloc` d tableau `coder_threads`.
- **3lach `start_time` kayt-akhed hna b 3ayn-o**: Kayt-akhed **merra wehda**,
  9bel ma yt-crea ay thread, bach ga3 les threads y7esbou b nefs l-marje3. Kol
  timestamp f les logs howa `now_ms() - start_time`.
- **3lach les `return (1)` ma kay-nqqiwch**: `main` howa li kay-3eyyet l
  `cleanup_all` f l-erreur, w `cleanup_all` m-brmja bach tkhdem hta ila kant
  l-bina ness-ness. Ila derna l-tenqiya hna tani, ghadi n-douplikiw l-code.

#### 2. `init_dongles(t_sim *sim)`
- **Chno kadir**: `malloc` d tableau d `n` dongles, w l kol wahed: `id`,
  `taken = 0`, `available_at = start_time`, `pthread_mutex_init`, w
  `heap_create(n, scheduler)`.
- **3lach `n` dongles b ddabt**: Le sujet gal *"There are as many dongles as
  coders."* F dayra d `n` coders kayn `n` d les fjouj bin les jiran.
- **3lach `available_at = start_time`**: Dongle li ma chedddo hta wahed khassou
  ykoun mawjoud f l-blassa f `t = 0`. Ila 7ttina `0`, kaykhdem tani, walakin b
  `start_time` kolchi kaybqa f nefs l-marje3 d l-wa9t.

#### 3. `init_coders(t_sim *sim)`
- **Chno kadir**: `malloc` d tableau d `n` coders, w l kol wahed:
  ```c
  sim->coders[i].id = i;
  sim->coders[i].left_dongle = i;
  sim->coders[i].right_dongle = ((i + 1) % n);
  sim->coders[i].compile_count = 0;
  sim->coders[i].last_compile_start = sim->start_time;
  sim->coders[i].sim = sim;
  pthread_mutex_init(&sim->coders[i].state_mutex, NULL);
  ```
- **3lach `(i + 1) % n`**: Bach n-sddou d-dayra. Aakher coder, l-dongle d limn
  dyalo kayrje3 l dongle `0`. B l-modulo, ma khassnach ta `if`, w kaykhdem m3a
  ay 3adad d coders.
- **3lach `last_compile_start = start_time` machi `0`**: Le sujet kay7sseb
  l-burnout men *"the beginning of their last compile **or the beginning of the
  simulation**"*. Ila khellinah `0` (li howa 1970 f l-wa9t d Unix), l-monitor f
  awwel tcheki ghadi y-7sseb tqriban 1.7 trillion ms w y-3lan bli ga3 les
  coders matou f l-blassa!
- **3lach `sim` (l-back-pointer)**: `pthread_create` kat-3ti l thread ghir
  **wahed** `void *`. L-coder khassou l-7wayj dyalo (id, dongles) **w** l-7wayj
  l-mouchtaraka (les timings, l-flag, les dongles kollhom). B had l-pointeur,
  `&sim->coders[i]` wehdo kafi.
- **L-hala d coder wahed (`n = 1`)**: `right = (0 + 1) % 1 = 0`. Donc `left` w
  `right` homa **nefs l-dongle**! Coder khassou ychedd nefs l-dongle b joj
  yeddih — mostahil. Donc ma ghadi ycompili hta merra w ghadi ymout b l-burnout.
  Hadchi kharej **wahdo** men l-formule, bla ta `if` f l-init.

#### 4. `join_all(t_sim *sim)`
- **Chno kadir**: `pthread_join` l ga3 les coders, w men be3d l l-monitor.
- **3lach darori 9bel l-tenqiya**: Thread li ma zal kheddam momkin ykoun wst
  section m7miya b mutex. Ila hedemna l-mutex w howa kheddam biha = undefined
  behavior. `pthread_join` kay-dmen bli l-thread sala 100%.

---

### E. `init_utils.c`

#### 1. `cleanup_dongles(t_sim *sim)`
- **Chno kadir**: L kol dongle: `heap_destroy` w men be3d
  `pthread_mutex_destroy`, w f l-lakher `free(sim->dongles)`.
- **3lach `heap_destroy` 9bel l-mutex**: Kan-libériw l-mouhtawa 9bel ma n-hedmou
  s-sarout li kan kay7miih.
- **3lach `if (!sim->dongles) return ;`**: Ila fchel l-`malloc` f
  `init_dongles`, had l-pointeur howa `NULL` (b fadl l-`memset`), w l-boucle
  ghadi t-dereferenci NULL = crash.

#### 2. `cleanup_all(t_sim *sim)`
- **Chno kadir**: `cleanup_dongles`, men be3d les `state_mutex` d les coders w
  `free(sim->coders)`, men be3d `free(sim->coder_threads)`, w f l-lakher
  `stop_mutex` w `log_mutex`.
- **3lach `free(sim->coder_threads)` bla `if` walakin l-boucle b `if`**:
  `free(NULL)` f l-standard d C ma kaydir **walou** (safe). Walakin
  dereferencing d NULL f boucle = crash.

---

### F. `utils.c`

#### 1. `now_ms(void)`
- **Chno kadir**: Katjbed l-wa9t l-7ali b millisecondes b `gettimeofday`.
- **3lach `long` machi `int`**: L-wa9t d daba b ms howa tqriban
  **1,700,000,000,000** — kbir bzaf 3la `INT_MAX` (tqriban 2.1 milyar). B `int`
  kaydi3 l-hissab f kol appel. **Hada kan bug 7a9i9i f version 9dima**: b `int`,
  `precise_sleep` ma kanch kayn3ess ta merra, w simulation kamla kant kat-sali f
  4 millisecondes!
- **3lach `(void)` f les parametres**: F C, `int f()` ma kat3nich "bla
  arguments", kat3ni "arguments ghir m-7eddda". `(void)` hiya li kat9ol b
  wdouh: walo.

#### 2. `precise_sleep(t_sim *sim, long ms)`
```c
target = now_ms() + ms;
while (now_ms() < target)
{
    if (sim_stopped(sim))
        return ;
    usleep(300);
}
```
- **3lach machi `usleep(ms * 1000)` ghir merra wehda**: Joj asbab:
  1. **Precision**: `usleep` kat-dmen bli ghadi tn3ess **3la l-a9al** dik
     l-moudda; l-systeme kayfiy9ek fach kaybgha. Sleep tawil kayfout l-wa9t b
     millisecondes, w hadchi kaytjm3 3la kol phase d kol coder.
  2. **Ykoun mo9ata3**: Coder f west compile d 500ms khassou y3ref bli
     l-simulation w9fat. B sleep wahed tawil, ma ghadi y3ref hta ysali — w
     `pthread_join` ghadi ytsenna bla fayda.
- **Hala khassa**: ila `ms = 0`, `target == now_ms()`, l-condition khaybba men
  l-bedya, w l-fonction katrje3 bla n3as. W hada s7i7 — le sujet kaysmeh b
  phases b zero.

#### 3. `log_state(t_sim *sim, int coder_id, char *state)`
```c
if (sim_stopped(sim))
    return ;
pthread_mutex_lock(&sim->log_mutex);
printf("%ld %d %s\n", now_ms() - sim->start_time, coder_id, state);
pthread_mutex_unlock(&sim->log_mutex);
```
- **L-mutex**: Bla biha, joj coders kay-3eyyto l `printf` f nefs l-wa9t w
  les phrases kay-dkhlou f ba3diyat-hom 7arf b 7arf.
- **L-garde d l-stop**: Hiya li kat-dmen bli satr d `burned out` howa **aakher
  satr** f l-affichage. Men be3d ma yt-7ott l-flag, walou ma kayt-tbe3.
- **Natija mouhimma jiddan**: L-monitor **khassou y-tbe3 l-burnout 9bel** ma
  y-7ott l-flag. Ila 7ott l-flag lwel, had l-garde b 3ayn-o ghadi t-khebbi
  l-message l-a-ham f l-projet kamel!
- **`coder_id + 1`**: F l-code les ids kaybdaw men `0`, walakin f l-affichage
  le sujet kaybda men `1`.

#### 4. `sim_stopped(t_sim *sim)`
```c
pthread_mutex_lock(&sim->stop_mutex);
val = sim->stop;
pthread_mutex_unlock(&sim->stop_mutex);
return (val);
```
- **3lach mutex 3la `int` wahed?**: L-monitor kaykteb fih w ga3 les coders
  kay9raw fih f nefs l-wa9t. 9raya w ktaba m-t9at3in 3la nefs la mémoire bla
  synchronisation = **Data Race**, hta ila kan ghir `int`. Le standard d C ma
  kay-dmen walou 3la l-atomicité, w le compilateur momkin ykhebbi la valeur f
  registre w ma y3awed y9raha ta merra.
- **L-taman**: Lock/unlock bla concurrence kaykhod dqiqa d nanosecondes.
  Machi mochkil. S7a lwel.

---

### G. `heap.c` (L-algorithme)

#### 1. `sift_up(t_heap *heap, int i)`
- **Chno kadir**: Kat-tle3 element l fou9 hta ywsal blasto s7i7a.
- **Kifach**: `parent = (i - 1) / 2`. Ila l-element 3ndo asbaqiya kter men bah,
  kan-beddlou blasthom w kan-tel3o. Ila la, `break`.
- **3lach `break`**: Ila khsser l-element m3a bah, ma3na bli ga3 li fou9
  m-rettbin m3ah. Ma kaynch 3lach n-kemmlou.

#### 2. `heap_push(t_heap *heap, t_request req)`
```c
heap->data[heap->size] = req;
i = heap->size;
heap->size++;
sift_up(heap, i);
```
- **Tartib mouhim**: `i` kayt-akhed **9bel** `size++`. Ila 3kesna, `sift_up`
  ghadi ybda men index ghalat.
- **Ta3qid**: $O(\log N)$.

#### 3. `sift_down(t_heap *heap, int i)`
- **Chno kadir**: Kat-hbet element l teht hta ywsal blasto.
- **Tlata d les tafasil l-mouhimma**:
  1. **`left < heap->size` darori**: Element f teht momkin ykoun 3ndo wald
     wahed ola walou. Bla had l-verification, ghadi n9raw **ghbra 9dima** li
     ba9ya f l-tableau men les pops li fatou.
  2. **T-comparison t-tanya m3a `best` machi m3a `i`**: Ila l-wald d lisr rbe7,
     l-wald d limn khassou y-3eleb **l-wald d lisr**, machi l-bah l-asli.
  3. **`if (best == i) break;`**: Walo ma tbeddel, donc kolchi m-rettb men hna
     l teht.

#### 4. `heap_pop(t_heap *heap, t_request *out)`
```c
*out = heap->data[0];
heap->size--;
heap->data[0] = heap->data[heap->size];
sift_down(heap, 0);
```
- **3lach `size--` 9bel l-assignation**: Men be3d ma n-n9so, `heap->size` **howa
  b 3ayn-o** l-index d aakher element s7i7. B l-3aks ghadi n9raw barra men
  l-hdoud.
- **3lach aakher element b ddabt**: Fach kan-7iydo l-qimma, kat-tkhella 7offra.
  L-3ewd b aakher element howa l-ikhtiyar l-wahid li kayhfed chekl l-arbre
  **complet**, w les formules d les index kayt-3etmdou 3la dak chekl.

#### 5. `heap_peek(t_heap *heap, t_request *out)`
- **Chno kadir**: Kat9ra index 0 bla ma t-7iyyedo.
- **3lach darori tkoun bla hedm**: Coder kay-sta3mlha bach ysowwel "wach ana
  lwel f had l-file?" — w momkin l-jawab ykoun **la**, w f dik l-hala khassha
  l-file tbqa 3la hal-ha.
- **Ta3qid**: $O(1)$.

---

### H. `heap_utils.c`

#### 1. `swap(t_request *a, t_request *b)`
- **3lach b les pointeurs**: F C, les arguments kayt-passaw **b la valeur**. Ila
  khddina `t_request a, t_request b`, ghadi n-beddlou joj **copies** w
  l-tableau l-asli ma ytbeddel walou.

#### 2. `cmp_request(t_request *a, t_request *b, t_scheduler sched)`
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
- **Had la fonction wehdha hiya l-politique d l-scheduling kamla.**

| Scheduler | Tartib d l-mou9arana |
|---|---|
| FIFO | `arrival` -> `coder_id` |
| EDF | `deadline` -> `arrival` -> `coder_id` |

- **3lach l-égalité machi nadira**: `now_ms()` 3endha precision d millisecondes.
  Joj coders li tlbou f nefs l-milliseconde 3endhom nefs `arrival`. Joj coders
  li bdaw ycompiliw f nefs l-milliseconde 3endhom nefs `deadline`.
- **3lach EDF khassou `arrival` 9bel `coder_id`**: Ila 9fezna direct l
  `coder_id`, l-coder li 3ndo ra9m sgher ghadi yrbe7 **dima** m3a nefs l-jar —
  machi merra, walakin l l-abad hta t-fer9o les deadlines. Hadi **starvation
  systematique**, w le sujet kay-mne3ha.
- **3lach `coder_id` f l-lakher darori**: Comparateur d heap khassou ykoun
  **total w deterministe**. Ila joj demandes mkhtalfin momkin ykounou "la hada
  9bel hada w la 3keso", les sift ma 3endhom ta relation thabta w l-heap momkin
  **ytkhesser** f la structure dyalo. `coder_id` farid, donc l-jawab dima
  wadeh.
- **L-fayda d had t-tasmim**: `sift_up` w `sift_down` **ma fihom ta kelma** 3la
  FIFO ola EDF. Ila beddelti l-scheduler, ghir jawab had la fonction li
  kaytbeddel. 3la haqqach recode d LIFO howa **7arf wahed** (chouf section 9).

#### 3. `heap_create(int capacity, int scheduler)`
- **L-jouz l-mouhim howa l-erreur**:
  ```c
  heap->data = malloc(capacity * sizeof(t_request));
  if (!heap->data)
  {
      free(heap);        /* n-libériw li nja7 9bel */
      return (NULL);
  }
  ```
  Ila fchel t-tani `malloc`, l-awwel **nja7**. Ila rj3na `NULL` bla
  `free(heap)`, ghadi ykoun 3endna **fuite**.
- **3lach `capacity = num_coders`**: Kol coder 3endo dongles thabtin. Ila
  hellina: dongle `j` kay-sta3mlouh **ghir joj coders** — coder `j` (bhal lisr
  dyalo) w coder `(j-1+n) % n` (bhal limn dyalo). Donc l-file ma kat-fout jouj
  demandes. `num_coders` howa 7ed a3la bzaf w safi bla realloc.

#### 4. `heap_destroy(t_heap *heap)`
- **3lach `if (!heap) return ;` khddama b7e9**: `cleanup_all` momkin ykhdem men
  be3d bina ness-ness, fin chi dongles 3endhom heaps w chi akhrin `NULL`.
- **Tartib**: `free(heap->data)` **9bel** `free(heap)`, hit 9raya d
  `heap->data` men be3d ma n-libériw `heap` = use-after-free.

---

### I. `dongle_utils.c`

#### 1. `build_request(t_coder *coder, t_sim *sim, long arrival)`
```c
req.coder_id = coder->id;
req.arrival = arrival;
req.deadline = coder->last_compile_start + sim->time_to_burnout;
```
- **3lach `arrival` kayji bhal parametre machi `now_ms()` hna**: Hada kan **bug
  7a9i9i** f version 9dima. L-timestamp kan kayt-akhed **f kol dongle 3la
  hda**. Hit l-coder kaywsel l t-tani dongle men be3d l-awwel, l-arrival dyalo
  f t-tani dima kan m-t2akher men dyal l-jar li 3ndo dak l-dongle bhal
  **awwel** dongle. Donc f FIFO, l-jar kan kayrbe7 **kol merra**, w
  l-mochkil kan kaydour f d-dayra kamla. L-a3rad li chefna: coder kaycompili 10
  d les merrat w akhor 4 w men be3d ymout. Daba `request_dongles` kayakhod
  **timestamp wahed** w kay-passih l joj les files.
- **3lach 9raya d `last_compile_start` bla mutex hna machi mochkil**: L-wahid
  li kaykteb f had l-champ howa l-thread d l-coder b 3ayn-o (f
  `record_compile`), w `build_request` kayt-3eyyet men nefs dak l-thread.
  Thread li kay9ra haja howa li kaykteb-ha ma khassouch verrou. L-9raya d
  **l-monitor** hiya li m7miya b l-mutex.

#### 2. `lock_both(t_sim *sim, int a, int b)` w `unlock_both(...)`
```c
void	lock_both(t_sim *sim, int a, int b)
{
	pthread_mutex_lock(&sim->dongles[a].mutex);
	if (b != a)
		pthread_mutex_lock(&sim->dongles[b].mutex);
}
```
- **Chno kaydiro**: Kay-verrouillaw `a` w men be3d `b`, w kay-tel9o `b` w men
  be3d `a`.
- **L-qa3ida d-dahabiya**: Hit `a < b` **dima** (b fadl `order_dongles`), ga3
  les threads f l-programme kay-verrouillaw les mutexes b **nefs t-tartib
  l-3alami**. Hadchi howa l-asas d l-himaya men deadlock (chouf section 9, Q1).
- **`if (b != a)`**: Himaya men l-hala d `n = 1` fin joj les dongles homa
  wahed — bach ma n-locki-ch nefs l-mutex jouj merrat (self-deadlock).

#### 3. `is_free(t_dongle *d, long now)`
```c
return (d->taken == 0 && now >= d->available_at);
```
- **Joj chourout, bjouj daroriyin**:
  - `taken == 0`: ma kaych-do hta wahed.
  - `now >= available_at`: **l-cooldown sala**.
- Had s-satr wahdo howa t-tatbiq kamel d `dongle_cooldown` li matloub f
  le sujet.

#### 4. `wait_single_dongle(t_sim *sim)`
```c
while (!sim_stopped(sim))
    usleep(300);
return (1);
```
- **L-hala d `n = 1` ghir hiya**. Coder wahed 3endo `left == right == 0`. Khassou
  ychedd nefs l-dongle b joj yeddih — mostahil. F 3oud ma n-khelliwh y-dkhol f
  l-boucle l-3adiya (fin ghadi y-pushi nefs l-coder jouj merrat f nefs l-heap),
  kaygless ghir kaytsenna hta l-monitor yw9ef l-simulation.
- **Katretoorner `1`** = "ma khditch" — donc `do_compile` ghadi y-retoorner bla
  ma y-tbe3 "is compiling" w bla ma y-3eyyet l `release_dongles`.

---

### J. `dongle.c` (L-QELB D L-PROJET)

#### 1. `order_dongles(t_coder *coder, int *a, int *b)`
```c
*a = coder->left_dongle;
*b = coder->right_dongle;
if (*a > *b)
{
    *a = coder->right_dongle;
    *b = coder->left_dongle;
}
```
- **Chno kadir**: Kat-7ott l-id s-sgher f `a` w l-kbir f `b`. Walou akhor.
- **3lach mouhimma**: Hiya li kat-dmen bli `lock_both` dima kay-verrouilla men
  sgher l kbir.

#### 2. `can_take_both(t_coder *coder, t_sim *sim)`
```c
now = now_ms();
if (!is_free(&sim->dongles[coder->left_dongle], now))     return (0);
if (!is_free(&sim->dongles[coder->right_dongle], now))    return (0);
if (heap_peek(sim->dongles[coder->left_dongle].waiters, &f) != 0)  return (0);
if (f.coder_id != coder->id)                              return (0);
if (heap_peek(sim->dongles[coder->right_dongle].waiters, &f) != 0) return (0);
if (f.coder_id != coder->id)                              return (0);
return (1);
```
- **Arb3a d les chourout, kollhom daroriyin**: joj les dongles khawyin
  (w cooldown sala), **w** l-coder howa **lwel f joj les files**.
- **Kayt-3eyyet ghir w les 2 mutexes chddin**, donc l-jawab howa **snapshot
  thabet** — ta 7ed ma y9der ybeddel walou binathom.

#### 3. `take_both(t_coder *coder, t_sim *sim)`
```c
heap_pop(left.waiters, &f);
heap_pop(right.waiters, &f);
left.taken = 1;
right.taken = 1;
log_state(sim, coder->id + 1, "has taken a dongle");
log_state(sim, coder->id + 1, "has taken a dongle");
```
- **Kayt-3eyyet ghir men be3d ma `can_take_both` t-retoorner `1`**, w les 2
  mutexes ba9yin chddin.
- **3lach joj `taken = 1` wst nefs l-verrou**: Hadchi howa li kaykhlli l-zouj
  (pair) **atomique** men wejhat nadar d ga3 les coders l-akhrin. Hta wahed ma
  y9der ychouf hala fin had l-coder chadd dongle wahed w machi t-tani.
- **3lach joj satrat d l-log**: Format d le sujet howa `timestamp X has taken a
  dongle` bla smiyt l-dongle, w l-mital dyalo kaybiyyen **jouj** satrat 9bel
  kol `is compiling`. Joj dongles t-khdaw b se77, donc joj satrat.

#### 4. `request_dongles(t_coder *coder, t_sim *sim)` — **L-FONCTION L-A-HAM**
```c
if (coder->left_dongle == coder->right_dongle)
    return (wait_single_dongle(sim));
order_dongles(coder, &a, &b);
req = build_request(coder, sim, now_ms());
lock_both(sim, a, b);
heap_push(sim->dongles[a].waiters, req);
heap_push(sim->dongles[b].waiters, req);
while (!sim_stopped(sim))
{
    if (can_take_both(coder, sim))
    {
        take_both(coder, sim);
        unlock_both(sim, a, b);
        return (0);
    }
    unlock_both(sim, a, b);      /* kat-tele9 KOLCHI */
    usleep(200);                  /* kat-tsenna chwiya */
    lock_both(sim, a, b);         /* kat-3awed t-jerreb */
}
unlock_both(sim, a, b);
return (1);
```

**Kifach t-9raha b jomla wehda**: *"N-3lan 3la l-niya f joj les dongles, w men
be3d n-tsenna bla ma n-chedd ta wahed, hta n9der nakhod-hom bjouj f nefs
l-lahda."*

- **L-3aqd (contract) li had la fonction kat-dmno**: Ila rj3at `0`, l-coder
  chadd **joj** les dongles. Ila rj3at ghir zero, l-coder **ma chadd walou**.
  W hit l-akhd howa "kolchi ola walou", ma kaynch hala ness-ness li khassna
  n-fekkouha f t-tari9 d l-erreur.
- **L-`unlock_both` wst l-boucle hiya s-serr**: Fach ma n9derch nakhod, kan-tel9o
  **kolchi** w kan-tsennaw. B haka ma kanb9aw chaddin walou — w hadchi howa li
  kaymne3 **Hold and Wait**.
- **`while` machi `if`**: Kolla merra kan-3awdo n-verifiw l-hala men l-jdid,
  machi kan-3tmdo 3la mou3tayat 9dima.

#### 5. `release_dongles(t_coder *coder, t_sim *sim)`
```c
order_dongles(coder, &a, &b);
lock_both(sim, a, b);
now = now_ms();
sim->dongles[a].taken = 0;
sim->dongles[a].available_at = now + sim->dongle_cooldown;
sim->dongles[b].taken = 0;
sim->dongles[b].available_at = now + sim->dongle_cooldown;
unlock_both(sim, a, b);
```
- **`available_at = now + cooldown`** — hna fin kaybda l-cooldown. Hta t-wsel
  dik l-lahda, `is_free` kat-retoorner `0` hta ila kan `taken == 0`.
- **3lach `order_dongles` hna tani**: Bach l-tartib d l-verrouillage yb9a nefso
  f ga3 l-programme.

---

### K. `coder.c`

#### 1. `do_compile(t_coder *coder, t_sim *sim)`
```c
if (request_dongles(coder, sim) != 0)
    return ;
start = now_ms();
record_compile(coder, start);
log_state(sim, coder->id + 1, "is compiling");
precise_sleep(sim, sim->time_to_compile);
release_dongles(coder, sim);
```
- **L-verification d l-return hiya l-qa3ida li kolchi kay-3tmed 3liha**: Ghir
  zero = l-coder **ma chadd walou** (hala d `n=1`, ola l-simulation w9fat).
  Kan-retoornou f l-blassa: bla log, bla n3as, w **bla `release_dongles`** —
  hit ila tel9na dongles ma chddnahomch, ghadi n-fekkou dongles li chaddhom chi
  coder akhor kheddam bihom!
- **`record_compile` 9bel `precise_sleep`**: L-magana d l-burnout khassha
  t-reseta f l-lahda fash **kaybda** l-compile. Chouf section 9 Q5 3la l-bug
  l-7a9i9i li had t-tartib fekkah.

#### 2. `do_phase(t_coder *coder, t_sim *sim, long duration, char *state)`
```c
log_state(sim, coder->id + 1, state);
precise_sleep(sim, duration);
```
- **Fonction wehda l joj les phases** (debug w refactor): homa bhal bhal f
  l-binya, kay-tfer9o ghir f l-moudda w f smiya. Ta wehda ma kat-chedd dongle —
  hit `do_compile` deja 3eyyet l `release_dongles` 9bel ma trje3.

#### 3. `record_compile(t_coder *coder, long start)`
```c
pthread_mutex_lock(&coder->state_mutex);
coder->compile_count++;
coder->last_compile_start = start;
pthread_mutex_unlock(&coder->state_mutex);
```
- **3lach l-mutex darori**: L-thread d had l-coder kaykteb f had joj les champs;
  l-thread d l-monitor kay9rahom. Ktaba w 9raya m-t9at3in bla synchronisation =
  **Data Race**.
- **3lach joj les champs f nefs l-verrou**: Homa kay-wesfou 7adita wehda w
  l-monitor kay9rahom bjouj. Ila fer9nahom, l-monitor momkin ychouf `count`
  jdid m3a `start` 9dim.
- **3lach `sim` machi parametre**: La fonction kat-mess ghir champs d `coder`.
  F version 9dima kan `t_sim *sim` w ma kanch m-sta3mel — w m3a `-Werror`,
  parametre ma m-sta3melch howa **erreur**, machi warning.

#### 4. `coder_routine(void *arg)`
```c
coder = (t_coder *)arg;
sim = coder->sim;
while (!sim_stopped(sim))
{
    do_compile(coder, sim);
    if (sim_stopped(sim))  break ;
    do_phase(coder, sim, sim->time_to_debug, "is debugging");
    if (sim_stopped(sim))  break ;
    do_phase(coder, sim, sim->time_to_refactor, "is refactoring");
}
return (NULL);
```
- **`void *arg` mafrouda men `pthread_create`**: L-signature khassha tkoun b
  ddabt `void *(*)(void *)`.
- **L-verification men be3d `do_compile` machi zayda**: `do_compile` kat-retoorner
  f l-blassa bla ma tcompili ila fchel `request_dongles`. Bla had t-tcheki,
  l-coder ghadi y-tbe3 "is debugging" b7al ila compila — w hada ghalat f
  l-affichage.
- **3lach ma kaynch tcheki "wach compilit bezzaf"**: L-chart d l-nihaya howa
  **3alami** (ga3 les coders), w le sujet kaymne3 coder y3ref haja 3la
  l-akhrin. Ghir l-monitor li y9der y-7kem 3liha.

---

### L. `monitor.c`

#### 1. `stop_sim(t_sim *sim)`
```c
pthread_mutex_lock(&sim->stop_mutex);
sim->stop = 1;
pthread_mutex_unlock(&sim->stop_mutex);
```
- Kat-refed l-flag, safi. Ga3 les threads kay-l9awh f awwel merra kay-verifiw.

#### 2. `check_burnout(t_sim *sim)`
```c
pthread_mutex_lock(&sim->coders[i].state_mutex);
elapsed = now_ms() - sim->coders[i].last_compile_start;
pthread_mutex_unlock(&sim->coders[i].state_mutex);
if (elapsed > sim->time_to_burnout)
{
    log_state(sim, i + 1, "burned out");
    stop_sim(sim);
    return (1);
}
```
- **L-9raya m7miya, walakin l-mou9arana w l-log kayjiw men be3d l-unlock**: Ila
  bqina chaddin `state_mutex` w 3eyyetna l `log_state` (li kat-akhod
  `log_mutex`), ghadi n-3e99do les verrous bla fayda.
- **`>` machi `>=`**: Coder li wsel b ddabt l l-hed ma fetch l-hed.
- **T-TARTIB L-MOUHIM JIDDAN: log 9bel l-flag.** `log_state` kat-retoorner
  f l-blassa ila kan `sim_stopped()`. Ila 3eyyetna l `stop_sim` lwel, satr d
  `burned out` — li howa **a-ham satr f l-projet kamel** — ghadi ykhebbih dak
  l-garde b 3ayn-o!
- **F l-bedya**: `last_compile_start == start_time`, donc l-magana kat-bda men
  `t = 0` l coder li ma compilach hta merra — w hadchi howa l-jouz *"or the
  beginning of the simulation"* f le sujet.

#### 3. `check_all_done(t_sim *sim)`
- **Chno kadir**: Kat-dor 3la ga3 les coders; ila l9at wahed `compile_count <
  compiles_required`, kat-retoorner `0` f l-blassa. Ila ga3 salaw, kat-3eyyet l
  `stop_sim` w kat-retoorner `1`.
- **Bla ta log**: Le sujet ma 3ndo ta satr l l-nihaya l-3adiya. L-programme ghir
  kayw9ef w kaykherj b `0`.

#### 4. `monitor_routine(void *arg)`
```c
while (1)
{
    if (check_burnout(sim))     break ;
    if (check_all_done(sim))    break ;
    usleep(300);
}
```
- **L-burnout kayt-tcheka lwel**: Ila t-7eqqou joj les chourout f nefs
  l-lahda, l-burnout 3ndo l-asbaqiya — hit howa l-fachal w khassou y-t3lan.
- **`usleep(300)`**: 300 microsecondes. Le sujet kaytleb l-burnout yt-tbe3 f
  9ell men 10ms; 300µs kat-3tina tqriban **33 merra** hamech d l-aman.
  L-9yas l-7a9i9i: burnout f 800ms kayt-tbe3 f **801ms**.

---

## 4. Kifach t-lanci l-programme etape b etape

```bash
# 1. Compilation
make

# 2. Lancement (8 arguments, kollhom daroriyin)
./codexion <coders> <burnout> <compile> <debug> <refactor> <compiles> <cooldown> <scheduler>

# Mital
./codexion 5 2000 200 200 200 10 0 fifo

# 3. Tenqiya
make fclean
```

| Argument | Ma3nah | L-wehda |
|---|---|---|
| `number_of_coders` | 3adad d les coders w les dongles | 3adad (>= 1) |
| `time_to_burnout` | L-wa9t maxi bla compile | ms |
| `time_to_compile` | Mouddat l-compilation | ms |
| `time_to_debug` | Mouddat l-debugging | ms |
| `time_to_refactor` | Mouddat l-refactoring | ms |
| `number_of_compiles_required` | Quota d les compiles bach nsaliw | 3adad |
| `dongle_cooldown` | Cooldown d les dongles men be3d t-tlaq | ms |
| `scheduler` | `fifo` ola `edf` | texte |

**Bach t-tcheki les outils**:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./codexion 5 100000 200 200 200 3 0 fifo
valgrind --tool=helgrind ./codexion 5 100000 200 200 200 3 0 fifo
valgrind --tool=drd      ./codexion 5 100000 200 200 200 3 0 fifo
cc -fsanitize=thread -g -Wall -Wextra -pthread *.c -o cx_tsan && ./cx_tsan 5 100000 200 200 200 3 400 edf
norminette *.c *.h
```

**Mlahada mouhimma bzaf**: M3a Valgrind, sta3mel `time_to_burnout` **kbir**
(mital `100000`). Valgrind kaybetti l-programme b 10 hta 50 merra, donc b
`2000` kolchi ghadi ymout f l-blassa w l-outil ma ghadi ychouf walou.

---

## 5. Amtila d les inputs w les outputs

### Mital 1: Coder wahed (khassou ymout)
```bash
./codexion 1 800 200 200 200 10 0 fifo
```
**Output**:
```
801 1 burned out
```
**Chreh**: Coder wahed, dongle wahed. `left == right == 0`, donc khassou ychedd
nefs l-dongle b joj yeddih — mostahil. Ma ghadi ycompili hta merra, w l-monitor
kayt-3lan l-burnout f 800ms (+1ms d ta2khir 3adi).

### Mital 2: 5 coders b wa9t m-rekh (kolchi kaysali)
```bash
./codexion 5 2000 200 200 200 10 0 fifo
```
**Output** (l-bedya):
```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 1 is debugging
...
```
**Chreh**: Dawra wehda = 200 + 200 + 200 = 600ms, w l-burnout howa 2000ms. Ta
wahed ma ghadi ymout. L-programme kayw9ef wehdo fach ga3 les coders ywsslou l
10 compiles. Chouf kifach **coder 1 w coder 3** bdaw f nefs l-wa9t — hit ma
kay-partagiwch ta dongle (1 3endo 0 w 1, 3 3endo 2 w 3).

### Mital 3: Wa9t 9asseh (khassou ymout)
```bash
./codexion 5 500 200 200 200 10 0 fifo
```
**Output** (l-lakher):
```
501 4 burned out
```
**Chreh**: Dawra wehda = 600ms, w l-burnout = 500ms. **Riyadiyan mostahil** ay
coder ykemmel dawra 9bel l-ajal. L-burnout mdmoun, w satr dyalo howa **aakher
satr**.

### Mital 4: M3a cooldown
```bash
./codexion 5 3000 200 200 200 10 400 fifo
```
**Chreh**: Ta wahed ma kaymout, w kolchi kaysali. Kol dongle kaybqa mghlou9
200ms (l-compile) + 400ms (cooldown) = 600ms. Dawra kamla d 5 coders = 1800ms,
w l-burnout = 3000ms -> hamech d 40%.

### Mital 5: Arguments ghalatin
```bash
./codexion                                   # [ERROR] wrong number of arguments!
./codexion 5 800 200 200 100 3 0 sjf         # [ERROR] scheduler must be fifo or edf!
./codexion 0 800 200 200 100 3 0 fifo        # [ERROR] invalid argument value!
./codexion -5 800 200 200 100 3 0 fifo       # [ERROR] invalid argument value!
./codexion abc 800 200 200 100 3 0 fifo      # [ERROR] invalid numeric argument!
./codexion 99999999999 800 200 200 100 3 0 fifo  # [ERROR] invalid argument value!
```
Kollhom kayt-refdou b `exit code 1` w l-message f **stderr** machi f stdout.

---

## 6. Legend: Les Fonctions Standard d C li kheddamna bihom

### A. Les Fonctions d les Threads (`<pthread.h>`)

#### 1. `pthread_create`
- **Chno kadir**: Kat-crea thread jdid dakhel nefs l-processus.
- **Les 4 arguments**:
  - `pthread_t *thread`: **Sortie**. Kat-ktebb fih l-identifiant d l-thread
    l-jdid. 9bel l-appel, dik la variable fiha ghbra.
  - `const pthread_attr_t *attr`: Attributs (stack size, detached...). `NULL` =
    l-3adi (joinable, stack standard).
  - `void *(*start_routine)(void *)`: **Pointeur 3la fonction**. Howa l-code li
    ghadi y-lanci l-thread. L-signature khassha tkoun b ddabt haka.
  - `void *arg`: **L-argument l-wahid** li ghadi yt-3ta l dik la fonction.
- **F had l-projet**: `pthread_create(&sim->coder_threads[i], NULL,
  coder_routine, &sim->coders[i])`. Kan-passiw l kol coder **l-adresse d
  la structure dyalo howa**, machi `sim` kamla.
- **Noqta mouhimma**: L-argument r-rabe3 **ma howach "les variables
  partagées"**. Howa ghir pointeur wahed. L-partage kayji hit dak l-pointeur
  kaychir l **nefs la mémoire** l ga3 les threads — machi copie.

#### 2. `pthread_join`
- **Chno kadir**: Kat-bloqui l-thread li 3eyyet-ha hta l-thread l-akhor ysali.
- **Les arguments**: `pthread_t thread` (chkoun), `void **retval` (fin n-7ttou
  l-valeur d l-return — derna `NULL` hit ma me7tajinahach).
- **F had l-projet**: Darori f `join_all`. Bla biha, `main` momkin yhdem les
  mutexes w les threads ba9yin kheddamin bihom = undefined behavior.
- **Wach t-tartib mouhim?**: **La**. `pthread_join` ghir kat-tsenna thread
  m3ayyen; les joins ma kay-t-t2ateroch b ba3diyat-hom. Li mouhim howa bli
  **ga3** les threads yt-joinaw, machi mn hiya n-bdaw.

#### 3. `pthread_mutex_init` / `lock` / `unlock` / `destroy`
- **Mutex** = **Mut**ual **Ex**clusion. Sarout kaykhlli **ghir thread wahed** f
  nefs l-lahda ydkhol l zone critique.
- **`init`**: Kat-hiyye2 l-mutex. `NULL` bhal attributs = standard.
- **`lock`**: Ila khawi, kat-akhdo w kat-kemmel. Ila kan chi thread akhor
  chaddo, kat-bloqui w kat-tsenna.
- **`unlock`**: Kat-tele9 bach akhor y9der ydkhol.
- **`destroy`**: Kat-libéri les ressources d l-systeme.
- **F had l-projet**: 3endna 3 anwa3:
  - Mutex d kol dongle: kay7mi `taken`, `available_at`, w l-heap dyalo.
  - `state_mutex` d kol coder: kay7mi `compile_count` w `last_compile_start`.
  - `stop_mutex` w `log_mutex`: kay7miw l-flag w l-affichage.

### B. Les Fonctions d l-Wa9t w N3as

#### 1. `gettimeofday` (`<sys/time.h>`)
- **Chno kadir**: Katjbed l-wa9t b microsecondes.
- **Les arguments**: `struct timeval *tv` (fiha `tv_sec` w `tv_usec`),
  `struct timezone *tz` (derna `NULL`).
- **F had l-projet**: F `now_ms`:
  `(tv.tv_sec * 1000) + (tv.tv_usec / 1000)`.
  Le sujet b 3ayn-o kay-nsseh biha: *"real-time measurements using gettimeofday()
  are acceptable and recommended for simplicity."*

#### 2. `usleep` (`<unistd.h>`)
- **Chno kadir**: Kat-wa99ef l-thread b microsecondes.
- **F had l-projet**: `usleep(300)` f `precise_sleep` w f `monitor_routine`,
  w `usleep(200)` f l-boucle d `request_dongles`.

### C. Les Fonctions d La Mémoire (`<stdlib.h>`)

#### 1. `malloc`
- **Chno kadir**: Kat-reservi blassa f l-Heap w katretoorner `void *`.
- **F had l-projet**: Tableau d les coders, tableau d les dongles, tableau d
  `pthread_t`, w tableau d les nodes f kol heap.
- **Dima khassek t-tchekiha**: `if (!ptr) return (1);`

#### 2. `free`
- **Chno kadir**: Kat-libéri la mémoire.
- **Noqta mouhimma**: `free(NULL)` ma kaydir **walou**. Hadchi howa li
  kaykhlli `cleanup_all` safe men be3d l-`memset`.

#### 3. `memset` (`<string.h>`)
- **Chno kadir**: Kat-3emmer bloc d la mémoire b qima wehda (hna `0`).
- **F had l-projet**: `memset(&sim, 0, sizeof(t_sim))` f l-bedya d `main` —
  bach ga3 les pointeurs ykounou `NULL` 9bel ay haja.

### D. L-Affichage w les Strings

#### 1. `printf` (`<stdio.h>`)
- **F had l-projet**: Kayt-3eyyet **ghir merra wehda** f ga3 l-projet: wst
  `log_state`, w hiya m7miya b `log_mutex`.

#### 2. `write` (`<unistd.h>`)
- **Chno kadir**: Syscall direct l file descriptor, bla buffer.
- **F had l-projet**: `write(2, msg, ft_strlen(msg))` f `print_error`.

---

## 7. Chreh Mfessel d les Structures d Données (`codexion.h`)

### 1. `t_scheduler` (Enum d l-politique)
```c
typedef enum e_scheduler { FIFO, EDF } t_scheduler;
```
`FIFO = 0`, `EDF = 1`. Kayt-khezzen bhal `int` f `t_sim` w f kol heap.
**3lach enum machi string**: `cmp_request` kayt-3eyyet f **kol** mou9arana f
l-heap. Mou9aranat `int` rkhissa bzaf 3la `strcmp`.

### 2. `t_request` (Talab d dongle)
```c
typedef struct s_request
{
    int     coder_id;   /* chkoun kaytleb */
    long    deadline;   /* last_compile_start + time_to_burnout — miftah EDF */
    long    arrival;    /* fash bda kayjerreb — miftah FIFO */
}   t_request;
```
Les 3 champs kollhom kay-dkhlou f t-tartib. `coder_id` **farid** f ga3
l-simulation — w hadchi howa li kaykhlli l-comparateur **total**.

### 3. `t_heap` (Priority Queue / Min-Heap)
```c
typedef struct s_heap
{
    t_request   *data;      /* tableau mastah; chekl l-arbre men les index */
    int         size;       /* 3adad d les elements l-7ayyin */
    int         capacity;   /* thabet 3la num_coders */
    int         scheduler;  /* ay politique t-tbe3 */
}   t_heap;
```
**L-formules d l-index**:

| 3ela9a | Formule |
|---|---|
| Bah d `i` | `(i - 1) / 2` |
| Wald d lisr d `i` | `2 * i + 1` |
| Wald d limn d `i` | `2 * i + 2` |

**Khasso ykoun wadeh**: Had les 3 formules homa **joj ittijahat** 3la nefs
l-7roufa f l-arbre. Fach kat-tale3 (`sift_up`), kat-sowwel "chkoun fou9i?" —
l-jawab howa `(i-1)/2`. Fach kat-hbet (`sift_down`), kat-sowwel "chkoun teht?"
— l-jawab howa `2i+1` w `2i+2`. Element f index 1 howa f nefs l-wa9t **wald**
d index 0 **w bah** d index 3 w 4.

**L-khassiya d l-heap**: Kol bah 3ndo asbaqiya kter men wladou bjouj. **Machi**
t-tableau m-rettb! Ghir dik l-3ela9a l-mahalliya bin bah w wladou. W hadchi
kafi, hit s-sou2al l-wahid li kansowwlouh dima howa "chkoun f l-wejh?", w
l-jawab dima f **index 0**.

### 4. `t_dongle` (USB Dongle)
```c
typedef struct s_dongle
{
    int             id;
    int             taken;          /* 0 = khawi, 1 = chi wahed chaddo */
    long            available_at;   /* ma y9derch yt-akhed hta had l-wa9t */
    pthread_mutex_t mutex;          /* kay7mi taken + available_at */
    t_heap          *waiters;       /* file d les demandes 3la HAD l-dongle */
}   t_dongle;
```
**Mlahada**: **Ma kaynch condition variable**. F had l-version, l-intidar
kayt-dar b `usleep(200)` f boucle, machi b `pthread_cond_wait` (chouf section 9
Q6 3la l-hoja d had l-ikhtiyar).

### 5. `t_coder` (Développeur / Thread)
```c
typedef struct s_coder
{
    int             id;                  /* 0-based dakhel, kayt-tbe3 id+1 */
    int             left_dongle;         /* = i */
    int             right_dongle;        /* = (i + 1) % n */
    int             compile_count;
    long            last_compile_start;  /* l-magana d l-burnout */
    pthread_mutex_t state_mutex;         /* kay7mi joj li fou9 */
    struct s_sim    *sim;                /* back-pointer */
}   t_coder;
```
**3lach `struct s_sim *` machi `t_sim *`**: F dik l-lahda f l-header, `t_sim`
ma zal ma t3errf. W hadchi **msmouh** hit howa **pointeur** — le compilateur
khassou ghir y3ref bli n-no3 kayn, machi l-hajm dyalo.

### 6. `t_sim` (L-Moteur Principal)
```c
typedef struct s_sim
{
    int             num_coders;
    long            time_to_burnout;
    long            time_to_compile;
    long            time_to_debug;
    long            time_to_refactor;
    int             compiles_required;
    long            dongle_cooldown;
    int             scheduler;
    t_dongle        *dongles;
    t_coder         *coders;
    pthread_t       *coder_threads;
    pthread_t       monitor_thread;
    int             stop;
    pthread_mutex_t stop_mutex;      /* kay7mi stop */
    pthread_mutex_t log_mutex;       /* kay7mi printf */
    long            start_time;      /* t = 0 l kol timestamp */
}   t_sim;
```
**`t_sim` hiya variable f la pile f `main`**, kat-passa b l-pointeur f kol
blassa. **Ma kaynch ta variable globale f l-projet kamel** — le sujet
kaymne3-hom, w l-evaluation kat-w9ef ila l9a wahda.

---

## 8. Kifach kheddam l-Min-Heap w L-Algorithme d Scheduling

### A. 3lach Min-Heap f 3oud tableau 3adi?
- F tableau 3adi: bach t3ref chkoun mol l-asbaqiya, khassk t-boucli 3la ga3
  les elements -> $O(N)$ f kol merra.
- F tableau m-rettb: kol insertion khassha t-7ayyed les elements -> $O(N)$.
- **F Min-Heap**:
  - `heap_peek`: dima f **index 0** -> $O(1)$.
  - `heap_push` w `heap_pop`: $O(\log N)$, hit ghir wahed l-far3 f l-arbre li
    kayt-sle7.
- W bla ta allocation f kol node, w la mémoire m-tla99a (cache-friendly).

### B. Kifach kheddam `heap_push` (Bubble Up)
1. Kan-7ottou l-talab l-jdid f **awwel blassa khawya** (dima `index = size`,
   hit l-arbre **complet**).
2. Kan-7efdo dak l-index f `i`, w men be3d `size++`.
3. Kan-compariwh m3a bah dyalo `(i - 1) / 2`.
4. Ila 3ndo asbaqiya kter, swap w kan-tel3o. Ila la, `break`.

### C. Kifach kheddam `heap_pop` (Bubble Down)
1. Kan-7efdo l-element d index 0 (howa r-rabe7) f `*out`.
2. `size--`.
3. Kan-7ottou **aakher element** f index 0.
4. `sift_down(heap, 0)` — kan-hebtouh hta yl9a blasto.

**3lach `size--` 9bel l-assignation?**: Men be3d ma n-n9so, `heap->size` howa
b 3ayn-o l-index d aakher element s7i7. B l-3aks ghadi n9raw barra men l-hdoud.

**3lach aakher element b ddabt?**: Bach n-hefdo chekl l-arbre **complet** (kol
mostawa 3amer 7ta l-akhir, w kayt-3emmer men lisr l limn). Ay element akhor
ghadi ykhlli **7offra f west**, w les formules d les index ghadi ytkessrou.

### D. L-Ferq bin FIFO w EDF f `cmp_request`
- **F FIFO**:
  - Kanchoufo `arrival`: li tlba 9bel (timestamp sgher) howa li kayakhod.
  - Ila t3adlou: `coder_id` (li ra9mo sgher).
- **F EDF**:
  - Kanchoufo `deadline` (`last_compile_start + time_to_burnout`): li 9reb ymot
    howa li kayakhod lwel.
  - Ila t3adlou: kan-rje3o l `arrival` (chkoun tlba 9bel).
  - Ila t3adlou tani: `coder_id`.
- **3lach EDF khassou `arrival` f west?**: Ila 9fezna direct l `coder_id`,
  l-coder b ra9m sgher ghadi yrbe7 **l l-abad** m3a nefs l-jar. Hadi
  **starvation systematique**, w le sujet kay-mne3ha b ddabt.

---

## 9. As'ila w Ajwiba dyal Soutenance (Peer-Evaluation Q&A)

### Q1: Kifach hresti l-Deadlock? Chno derti m3a les 4 Conditions d Coffman?

**L-Jawab**:
L-Deadlock ma y9der yw9e3 illa ila t-7eqqo **4** d les conditions f nefs
l-wa9t:
1. **Mutual Exclusion**: Dongle ma y9derch ykoun 3nd joj coders. **Ma n9dro
   n-hresso-ha** — hiya tabi3t l-mochkil.
2. **No Preemption**: Ma kan-fekkouch l-dongle b zzez men coder kheddam.
3. **Hold and Wait**: Coder kayched wahed w kaytsenna l-akhor.
4. **Circular Wait**: Dayra d les coders kol wahed kaytsenna li men be3do.

**F had l-projet, hressna JOUJ men had les conditions, machi wehda ghir hiya:**

**(a) Hressna "Hold and Wait" b l-akhd "kolchi ola walou"**:
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
    lock_both(sim, a, b);
}
```
Coder **3omro ma kayched dongle wahed w howa kaytsenna l-akhor**. `take_both`
kat-7ott `taken = 1` f joj ola f ta wahed, wst nefs section critique. Hit
ma kaynch "hold and wait", coder **mostahil** ykoun 7ol9a f selsla d
"chadd X w kaytsenna Y" -> ma kayn ta dayra.

**(b) Hressna "Circular Wait" b t-tartib t-thabet d les verrous**:
```c
void	order_dongles(t_coder *coder, int *a, int *b)
{
    *a = coder->left_dongle;
    *b = coder->right_dongle;
    if (*a > *b)   /* dima l-id s-sgher f 'a' */
    {
        *a = coder->right_dongle;
        *b = coder->left_dongle;
    }
}
```
**Mital b 5 coders**:
- Coder 1 khassou dongles 0 w 1 -> kay-locki **0 3ad 1**.
- Coder 5 khassou dongles 4 w 0 -> f 3oud ma y-locki 4 3ad 0, kay-locki
  **0 3ad 4**!

Hit ga3 les threads kay-verrouillaw f **nefs l-ittijah** (men l-id s-sgher l
l-kbir), selsla d "kaytsenna" khassha les ids ytle3ou dima — w selsla
kat-tle3 dima f majmou3a mahdouda **mostahil** treje3 3la rasha.

**Jawab 9asser**: *"Kol wahda men had les tekniktayn wehdha kafya. Ana
3endi joj, mosta9illin."*

---

### Q2: Chno kayw9e3 b coder wahed (`./codexion 1 800 200 200 200 10 0 fifo`)?

**L-Jawab**:
- F `init_coders`: `left = 0`, `right = (0 + 1) % 1 = 0`. Donc **joj homa nefs
  l-dongle**.
- Coder khassou ychedd nefs l-dongle b joj yeddih — **mostahil fizyaiyan**.
- **F l-code**:
  ```c
  if (coder->left_dongle == coder->right_dongle)
      return (wait_single_dongle(sim));
  ```
  Kanchoufo l-hala f l-bedya, **9bel** ay verrou. `wait_single_dongle` ghir
  kaygless kaytsenna hta l-flag.
- **3lach machi ghir n-khelliwh y-dkhol l-boucle l-3adiya?**: Ghadi y-pushi
  **nefs l-coder jouj merrat f nefs l-heap**, w `lock_both` ghadi t-jerreb
  t-locki nefs l-mutex jouj merrat = **self-deadlock** (mutex POSIX machi
  récursif b l-défaut).
- **L-natija**: `801 1 burned out`, w l-programme kaysali **cleanly** bla crash
  w bla freeze.
- **Noqta hlwa l l-soutenance**: Had s-solouk kharej **wehdo** men l-formule
  `(i + 1) % n`. Ma dernach ta `if` f `init_dongles` ola `init_coders`.

---

### Q3: Kifach kheddam l-Dongle Cooldown b ddi99a?

**L-Jawab**:
- **Fach coder kaysali** (`release_dongles`):
  ```c
  now = now_ms();
  sim->dongles[a].taken = 0;
  sim->dongles[a].available_at = now + sim->dongle_cooldown;
  ```
- **Fach chi coder kayji yakhod** (`is_free`):
  ```c
  return (d->taken == 0 && now >= d->available_at);
  ```
- **Joj chourout**: ma chaddo hta wahed **W** l-cooldown sala. Ila kan wahed
  khayeb, `can_take_both` kat-retoorner `0` w l-coder kay-tele9 kolchi w
  kay-3awed yjerreb men be3d 200µs.
- **L-cooldown kollo f satr wahed** — hada howa l-jawab l-basit.

---

### Q4: Kifach derti bach Burnout Detection tkoun f 9ell men 10ms?

**L-Jawab**:
- L-monitor f `monitor_routine` 3ndo boucle katdor b `usleep(300)` — **300
  microsecondes**, ya3ni 0.3 milliseconde.
- Kolla dawra, kay9ra `last_compile_start` d kol coder (m7mi b `state_mutex`)
  w kay-compari:
  ```c
  if (elapsed > sim->time_to_burnout)
  ```
- **L-hissab**: 300µs kat-3tina tqriban **33 merra** hamech d l-aman m9arna b
  10ms li matloubin.
- **L-9yas l-7a9i9i**: b `time_to_burnout = 800`, l-log kayt-tbe3 f **801ms**.
  B `500`, kayt-tbe3 f **501ms**. Dima 1ms d ta2khir.

---

### Q5: Chno l-ferq bin Data Race w Race Condition? W 3endek chi mital men
l-projet dyalek?

**L-Jawab** (had s-sou2al kaytsewwel bzaf):

- **Data Race** howa hala **tiqniya d la mémoire**: joj threads kay-dkhlou l
  nefs l-blassa f nefs l-wa9t, 3la l-a9al wahed kaykteb, w bla synchronisation.
  Hada howa li kay-l9awh Helgrind w ThreadSanitizer.
- **Race Condition** howa **a3amm**: l-natija d l-programme katbeddel 7sab
  l-timing w katwelli **ghalta**. W momkin ykoun 3endek race condition **bla ta
  data race** — kolchi m7mi b mutex, walakin l-mantiq ghalat.

**Mital 7a9i9i men l-projet dyali** (bug li l9itou w fekkitou):

F version 9dima, `record_compile` kan kayt-3eyyet **men be3d** n3as d
l-compile:
```c
start = now_ms();
precise_sleep(sim, sim->time_to_compile);   /* 200ms */
release_dongles(coder, sim);
record_compile(coder, start);    /* GHALAT — l-magana kat-reseta men be3d */
```
Kolchi kan m7mi b `state_mutex` -> **ma kanch data race**. Walakin
`last_compile_start` kan kaybqa b l-qima l-9dima **tul l-compile kamla**:

| L-wa9t | L-haqiqa | Chno kan kaychouf l-monitor |
|---|---|---|
| t=0 | coder 1 bda ycompili | `last_compile_start = 0` |
| t=200 | l-compile sala | `last_compile_start = 0` |
| t=400 | coder 1 bda ycompili men jdid | ba9i `0`! ma kay-tbeddelch hta t=600 |
| t=411 | coder 1 **kaycompili daba** | elapsed 411 > 410 -> **burnout kaddab** |

**L-hall**: n-7ttou `record_compile` **9bel** n3as:
```c
start = now_ms();
record_compile(coder, start);    /* l-magana kat-reseta f l-lahda fash bda */
log_state(sim, coder->id + 1, "is compiling");
precise_sleep(sim, sim->time_to_compile);
```

**L-3ibra**: "m7mi b mutex" **machi** nefs l-haja bhal "s7i7". T-tartib
kayhemm.

---

### Q6: 3lach ma kheddamtich `pthread_cond_t`?

**L-Jawab**:
- Le sujet gal: *"A condition variable (pthread_cond_t) **may** be used to
  manage waiting queues."* — **may**, machi **must**. Donc machi ijbari.
- **L-hoja t-tiqniya**: M3a l-akhd "kolchi ola walou", ma kaynch **7adita
  wehda** li n9der n-signaliw 3liha. L-coder kaytsenna 3la:
  - hala d **joj** dongles f nefs l-wa9t,
  - **w** joj magana d cooldown.

  W **intiha2 d cooldown machi 7adita** li chi thread y9der y-broadcasti-ha —
  hiya ghir **morour d l-wa9t**. Chi wahed khassou y3awed ychouf l-magana f ay
  hal.
- Donc l-polling b `usleep(200)` kaykhlli hadchi **wadeh**, w kay-7iyyed
  famila kamla d les bugs: missed wakeup, lost signal, spurious wakeup.
- **200 microsecondes** howa 50 merra sgher men 10ms d l-hamech.

**Jawab 9asser l l-correcteur**: *"Kheddemt les mutexes bach n7mi kol hala
mouchtaraka, w retry b backoff mahdoud l l-akhd. Khtart l-polling 3la 9sd hit
ma kaynch signal wahed n-signaliwh — l-coder kaytsenna 3la hala d joj dongles
zaid joj magana d cooldown, w intiha2 d cooldown machi 7adita, ghir wa9t li
kaydouz."*

---

### Q7: Kifach mna3ti Data Race bin Monitor w Coders?

**L-Jawab**:
Kol hala mouchtaraka 3endha **mutex wahed**, w **ga3** l-wosoul kaydouz menou:

| L-mou3tayat | Chkoun kaykteb | Chkoun kay9ra | M7miya b |
|---|---|---|---|
| `dongle->taken`, `available_at`, `waiters` | ay coder | ay coder | mutex d dak l-dongle |
| `coder->compile_count`, `last_compile_start` | dak l-coder | l-monitor | `state_mutex` d dak l-coder |
| `sim->stop` | l-monitor | ga3 les threads | `stop_mutex`, ghir b `sim_stopped()` |
| `stdout` | ga3 les threads | — | `log_mutex`, `printf` wahed f l-projet |

**L-indibat li kaykhlli hadchi yb9a s7i7**: Ma kaynch ta blassa f l-code fin
kay-t-mess had les champs **direct**:
- `sim->stop` kayt9ra **ghir** b `sim_stopped()`.
- `printf` kayt-3eyyet **merra wehda f ga3 l-projet**, wst `log_state`.

**L-ithbat**: ThreadSanitizer, Helgrind, w DRD kollhom kay-3tiw **0 errors**.

---

### Q8: Kifach kaysali l-programme cleanly bla ma yb9a chi thread mbloqui?

**L-Jawab** — had s-selsla hiya s-sabab 3lach `pthread_join` 3omrha ma
kat-bloqui:
1. L-monitor kaychouf burnout ola quota sala -> `stop_sim` kay-refed
   `sim->stop = 1` wst `stop_mutex`.
2. Kol coder f l-boucle d `request_dongles` kay-verifi `sim_stopped(sim)` f kol
   dawra (kol 200µs) -> kaykherj men l-boucle, kay-tele9 les mutexes,
   kay-retoorner `1`.
3. `do_compile` kaychouf `1` -> kay-retoorner bla ma ycompili w **bla ma
   y-tele9 walou** (hit ma chadd walou).
4. `coder_routine` kay-briki men l-boucle l-kbira.
5. Les coders li kano wst `precise_sleep` kaykherjo f 9ell men 300µs b l-verification
   dyalhom.
6. Ga3 les threads kay-retoornaw `NULL` -> `join_all` kaykemmel.
7. `cleanup_all` kayhdem ga3 les mutexes w kay-libéri la mémoire — **b aman**,
   hit ta wahed ma ba9i kheddam bihom.
8. Valgrind kay-3ti: `All heap blocks were freed -- no leaks are possible`.

---

### Q9: L-Recode: kifach t-beddel `fifo` l `lifo`?

**L-Jawab**: **7arf wahed.**

F `heap_utils.c`, f `cmp_request`, f l-far3 d FIFO:
```c
if (sched == FIFO)
{
    if (a->arrival != b->arrival)
        return (a->arrival < b->arrival);   /* FIFO: li ja 9bel kayrbe7 */
    return (a->coder_id < b->coder_id);
}
```
kaywelli:
```c
if (sched == FIFO)
{
    if (a->arrival != b->arrival)
        return (a->arrival > b->arrival);   /* LIFO: li ja L-AKHER kayrbe7 */
    return (a->coder_id < b->coder_id);
}
```
`<` -> `>`. W men be3d `make`. Safi.

**3lach hadi hiya l-blassa s7i7a?**: L-heap kayt-3eyyet l `cmp_request` f **kol**
`sift_up` w `sift_down`, w hadchi howa li kay-9errer chkoun kaygless f
**index 0** — ya3ni chkoun f wejh l-file. W `can_take_both` kay-sowwel
`heap_peek` bach ychouf chkoun lwel. Donc t-beddil d had la fonction
kaybeddel **chkoun kayakhod l-dongle b se77**, machi ghir chno kayt-tbe3.

**Kifach n-wrih l l-correcteur**:
```bash
./codexion 5 3000 200 200 200 4 400 fifo > before.log
# n-beddel < b > f heap_utils.c
make
./codexion 5 3000 200 200 200 4 400 fifo > after.log

awk '$3=="is" && $4=="compiling" {print $1, $2}' before.log | head -6
awk '$3=="is" && $4=="compiling" {print $1, $2}' after.log  | head -6
```

**L-natija l-7a9i9iya**:
```
FIFO:  0->1 | 600->2 | 1200->3 | 1800->4 | 2400->5 | 3000->1
LIFO:  0->1 | 600->1 | 1200->2 | 1800->1 | 1800->3 | 2400->1
```
F FIFO, d-dor kaydour 3la ga3 les 5 coders b tartib. F LIFO, li tlba
**l-akher** howa li kayrbe7 — coder 1 rbe7 4 men awwel 6 merrat. Hadi hiya
l-bassma d LIFO b ddabt.

**Jomla l l-correcteur**: *"`sift_up` w `sift_down` ma fihom **ta kelma** 3la
FIFO ola EDF. L-politique kamla galssa f `cmp_request` wehdha, w l-heap
kaysowwelha f kol i3adat tartib. Donc t-beddil howa f l-mantiq d l-arbitrage
b se77, machi f l-affichage."*

---

### Q10: 3lach `5 3000 200 200 200 10 800 fifo` chi merrat kaymout?

**L-Jawab** — hadi **riyadiyat**, machi bug. W khassek t-gouliha **9bel** ma
y-l9aha l-correcteur:

- Dayra d 5 coders kat-smeh b **jouj** compiles f nefs l-wa9t ghir hiya (ghir
  les coders li machi jiran).
- Kol compile kaykhelli 2 dongles machi mawjoudin: 200ms d l-compile + 800ms d
  l-cooldown = **1000ms**.
- Donc les dawrat kaykounou b3ad 1000ms 3la ba3diyat-hom, w kol dawra kat-khdem
  2 coders.
- Bach ga3 les 5 coders yakhdou dorhom: 3 dawrat (2+2+1) = **3000ms** l dawra
  kamla.
- `time_to_burnout` = **3000ms**. L-coder l-awwel khassou y3awed ycompili b
  ddabt f `t = 3000`. **Hamech = ZERO.**

Ta chi implementation ma t9der t-dmen had l-hala. M3a `cooldown = 400`:
dawrat kol 600ms, dawra kamla 1800ms m9abil 3000ms -> **hamech d 40%**, w
3la haqqach dik l-hala kat-nja7 dima.

**Mlahada mouhimma**: L-feuille d correction kat-tleb "bla burnout" **ghir l
l-hala d 400**. L-hala d 800, kat-tleb ghir **t9aren t-tartib d l-akhd bin
fifo w edf** — w hadchi kaykhdem b ay hal.

---

### Q11: 3lach 3adad d "is compiling" kbir 3la `n × compiles_required`?

**L-Jawab**:
- `check_all_done` kat-w9ef l-simulation ghir fach **ga3** les coders ywsslou l
  l-quota, machi fach l-awwel ywsel.
- L-monitor kay-tcheki kol 300µs, machi f l-lahda b ddabt. Bin l-lahda fash
  aakher coder wsel l-quota w l-lahda fash l-monitor lah, les coders li kano
  **f west compile** kaysaliw dik l-compile w kay-tbe3ouha.
- Donc l-3adad howa **3la l-a9al** `n × compiles_required`, w momkin yzid chwiya
  7sab l-timing. W hada s7i7 — le sujet kaytleb ghir *"at least"*.
- **Bach t-verifi**: `awk '{c[$2]++} END{for(k in c) print k, c[k]}'` — khass
  **kol** coder ykoun 3ndo >= l-quota.

---

### Q12: Wach t-tartib d `pthread_join` mouhim?

**L-Jawab**: **La.** `pthread_join` 3endha khedma wehda: t-bloqui hta l-thread
l-mahdoud ysali. Les joins **ma kay-t2ateroch b ba3diyat-hom** — kol wahda
intidar mosta9ill 3la thread mosta9ill.

Li **mouhim b se77** howa bli **ga3** les threads yt-joinaw, merra wehda l kol
wahed, **9bel** `cleanup_all`. Ila nsina wahed, `cleanup_all` momkin yhdem
mutex w dak l-thread ba9i kheddam biha = undefined behavior.

F l-code dyali kan-joinaw les coders 9bel l-monitor — machi hit darori,
walakin hit hadchi kay-3kess **s-sababiya**: l-monitor howa li kay-refed
l-flag, w les coders homa li kay-t-ra9bou 3lih.

---

### Q13: Wrini bli ma 3endekch ta variable globale.

**L-Jawab**: Ga3 l-hala mouchtaraka galssa f `t_sim`, w `t_sim` hiya variable f
**la pile** f `main`, kat-passa b l-pointeur. Kol coder kaywsel lha b
`coder->sim`.

```bash
grep -n "^[a-z_].*=" *.c | grep -v "^\s"    # walou
```

**3lach f la pile machi b `malloc`**: Wehda ghir hiya, w kat3ich l-mouddat
l-programme kamla. `main` ma kaysalich hta ykmmel `join_all`, donc ga3 les
threads kay-l9aw dik la mémoire mawjouda w sahiha.

---

### Q14: Chno kayw9e3 ila fchel `malloc` f west l-init?

**L-Jawab**:
- `main` kaybda b `memset(&sim, 0, sizeof(t_sim))`, donc **ga3 les pointeurs
  homa `NULL`** hta ma yt-allocaw b se77.
- `cleanup_all` kat-khdem b aman f ay no9ta: `free(NULL)` ma kaydir walou, w
  `heap_destroy` kat-verifi `NULL`.
- **L-noqta l-daifa (n-gouliha b sara7a)**: `pthread_mutex_destroy` 3la mutex
  li ma t-initialisach = undefined behavior — machi pointeur, donc `NULL` ma
  kat-3awen. Ila fchel `init_dongles` f dongle 3 men 5, dongles 0-2 3endhom
  mutexes 7ayyin w 3-4 3endhom bytes b zero. L-hall l-kamel howa 3addad d li
  nja7. F l-3amaliya 3la glibc, mutex b zero kaybane bhal wahed
  statically-initialised — walakin hadi tafsila d l-implementation machi
  dmana.
