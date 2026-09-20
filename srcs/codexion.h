/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 23:28:50 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 16:36:12 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}					t_scheduler;

typedef struct s_request
{
	int				coder_id;
	long			deadline;
	long			arrival;
}					t_request;

typedef struct s_heap
{
	t_request		*data;
	int				size;
	int				capacity;
	int				scheduler;
}					t_heap;

typedef struct s_dongle
{
	int				id;
	int				taken;
	long			available_at;
	pthread_mutex_t	mutex;
	t_heap			*waiters;
}					t_dongle;

typedef struct s_coder
{
	int				id;
	int				left_dongle;
	int				right_dongle;
	int				compile_count;
	long			last_compile_start;
	pthread_mutex_t	state_mutex;
	struct s_sim	*sim;
}					t_coder;

typedef struct s_sim
{
	int				num_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				compiles_required;
	long			dongle_cooldown;
	int				scheduler;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_t		*coder_threads;
	pthread_t		monitor_thread;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	log_mutex;
	long			start_time;
}					t_sim;

/* parse_utils.c */
int					is_digit(char c);
int					ft_strlen(char *str);
int					ft_strcmp(char *s1, char *s2);
int					ft_atoi(char *str);
int					is_number(char *str);

/* parse.c */
void				print_error(char *msg);
int					get_scheduler(char *str);
int					parse_numeric_args(char **argv, int *values);
void				fill_sim(t_sim *sim, int *values, int sched);
int					parse_args(int argc, char **argv, t_sim *sim);

/* init.c */
int					init_sim(t_sim *sim);
int					init_dongles(t_sim *sim);
int					init_coders(t_sim *sim);
int					join_all(t_sim *sim);
void				cleanup_dongles(t_sim *sim);
void				cleanup_all(t_sim *sim);

/* utils.c */
long				now_ms(void);
void				precise_sleep(t_sim *sim, long ms);
void				log_state(t_sim *sim, int coder_id, char *state);
int					sim_stopped(t_sim *sim);

/* heap.c */
int					heap_push(t_heap *heap, t_request req);
void				sift_up(t_heap *heap, int i);
void				sift_down(t_heap *heap, int i);
int					heap_pop(t_heap *heap, t_request *out);
int					heap_peek(t_heap *heap, t_request *out);

/* heap_utils.c */
void				swap(t_request *a, t_request *b);
int					cmp_request(t_request *a, t_request *b, t_scheduler sched);
t_heap				*heap_create(int capacity, int scheduler);
void				heap_destroy(t_heap *heap);

/* dongle.c */
void				order_dongles(t_coder *coder, int *a, int *b);
int					can_take_both(t_coder *coder, t_sim *sim);
void				take_both(t_coder *coder, t_sim *sim);
int					request_dongles(t_coder *coder, t_sim *sim);
void				release_dongles(t_coder *coder, t_sim *sim);

/* dongle_utils.c */
t_request			build_request(t_coder *coder, t_sim *sim, long arrival);
void				lock_both(t_sim *sim, int a, int b);
void				unlock_both(t_sim *sim, int a, int b);
int					is_free(t_dongle *d, long now);
int					wait_single_dongle(t_sim *sim);

/* coder.c */
void				do_compile(t_coder *coder, t_sim *sim);
void				do_phase(t_coder *coder, t_sim *sim, long dur, char *state);
void				record_compile(t_coder *coder, long start);
void				*coder_routine(void *arg);

/* monitor.c */
void				*monitor_routine(void *arg);
int					check_burnout(t_sim *sim);
int					check_all_done(t_sim *sim);
void				stop_sim(t_sim *sim);
int					launch_threads(t_sim *sim);

#endif