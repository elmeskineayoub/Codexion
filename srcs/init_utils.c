/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:03:02 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 16:30:39 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_dongles(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->num_coders)
	{
		heap_destroy(sim->dongles[i].waiters);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	free(sim->dongles);
}

void	cleanup_all(t_sim *sim)
{
	int	i;

	cleanup_dongles(sim);
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
	free(sim->coder_threads);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->log_mutex);
}
