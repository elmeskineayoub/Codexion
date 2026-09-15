/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:03:02 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 16:30:39 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_sim(t_sim *sim)
{
	sim->stop = 0;
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (1);
	sim->start_time = now_ms();
	if (init_dongles(sim) != 0)
		return (1);
	if (init_coders(sim) != 0)
		return (1);
	sim->coder_threads = malloc(sim->num_coders * sizeof(pthread_t));
	if (!sim->coder_threads)
		return (1);
	return (0);
}

int	init_dongles(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->num_coders;
	sim->dongles = malloc(n * sizeof(t_dongle));
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < n)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].taken = 0;
		sim->dongles[i].available_at = sim->start_time;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		sim->dongles[i].waiters = heap_create(n, sim->scheduler);
		if (!sim->dongles[i].waiters)
			return (1);
		i++;
	}
	return (0);
}

int	init_coders(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->num_coders;
	sim->coders = malloc(n * sizeof(t_coder));
	if (!sim->coders)
		return (1);
	i = 0;
	while (i < n)
	{
		sim->coders[i].id = i;
		sim->coders[i].left_dongle = i;
		sim->coders[i].right_dongle = ((i + 1) % n);
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].sim = sim;
		pthread_mutex_init(&sim->coders[i].state_mutex, NULL);
		i++;
	}
	return (0);
}

int	join_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_join(sim->coder_threads[i], NULL);
		i++;
	}
	pthread_join(sim->monitor_thread, NULL);
	return (0);
}

void	cleanup_all(t_sim *sim)
{
	int	i;

	if (sim->dongles)
	{
		i = 0;
		while (i < sim->num_coders)
		{
			heap_destroy(sim->dongles[i].waiters);
			pthread_cond_destroy(&sim->dongles[i].cond);
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			i++;
		}
		free(sim->dongles);
	}
	if (sim->coders)
	{
		i = 0;
		while (i < sim->num_coders)
		{
			pthread_mutex_destroy(&sim->coders[i].state_mutex);
			i++;
		}
		free(sim->coders);
	}
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->log_mutex);
}
