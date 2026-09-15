/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 09:44:58 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 07:35:23 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	precise_sleep(t_sim *sim, int ms)
{
	int	target;

	target = now_ms() + ms;
	while (now_ms() < target)
	{
		if (sim_stopped(sim))
			return ;
		usleep(300);
	}
}

void	ms_to_abstime(struct timespec *ts, int ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += (long)(ms % 1000)*1000000L;
	if (ts->tv_nsec >= 1000000000L)
	{
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000L;
	}
}

void	log_state(t_sim *sim, int coder_id, char *state)
{
	if (sim_stopped(sim))
		return ;
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", now_ms() - sim->start_time, coder_id, state);
	pthread_mutex_unlock(&sim->log_mutex);
}

int	sim_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_mutex);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (val);
}
