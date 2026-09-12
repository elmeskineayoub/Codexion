#include <pthread.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

int is_digit(char c);
int is_number(char *str);
int ft_atoi(char *str);
int ft_strlen(char *str);
int ft_strcmp(char *s1, char *s2);
int valid_scheduler(char *str);
void print_error(char *msg);


typedef struct s_dongle
{
    int id;
    int taken; // 0=free ; 1=taken
    long availablle_at; //ms timestamp; usable once now >= this
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    t_heap *waiters; //requests queued for This dongle only
}   t_dongle;


typedef struct s_coder
{
    int id;
    int left_dongle;
    int right_dongle;
    int compile_count;
    long last_compile_start;
    pthread_mutex_t state_mutex;
} t_coder;


typedef struct s_sim
{
    int num_coders;
    long time_to_burnout;
    long time_to_compile;
    long time_to_debug;
    long time_to_refactor;
    int compiles_required;
    long dongle_cooldown;
    int scheduler;
    t_dongle *dongles;
    t_coder *coders;
    pthread_t *coder_threads;
    pthread_t monitor_thread;
    int stop;
    pthread_mutex_t stop_mutex;
    pthread_mutex_t log_mutex;
    long start_time;
} t_sim;


typedef struct s_request
{
    int coder_id;
    long deadline;
    long arrival;
} t_request;


typedef struct s_heap
{
    t_request *data;
    int size;
    int capacity;
    int scheduler;
} t_heap;