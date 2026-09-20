/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 07:41:53 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 16:19:41 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	stop_sim(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
}

int	check_burnout(t_sim *sim)
{
	int		i;
	long	elapsed;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_mutex);
		elapsed = now_ms() - sim->coders[i].last_compile_start;
		pthread_mutex_unlock(&sim->coders[i].state_mutex);
		if (elapsed > sim->time_to_burnout)
		{
			log_state(sim, i + 1, "burned out");
			stop_sim(sim);
			return (1);
		}
		i++;
	}
	return (0);
}

int	check_all_done(t_sim *sim)
{
	int	i;
	int	count;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_mutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].state_mutex);
		if (count < sim->compiles_required)
			return (0);
		i++;
	}
	stop_sim(sim);
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		if (check_burnout(sim))
			break ;
		if (check_all_done(sim))
			break ;
		usleep(300);
	}
	return (NULL);
}
