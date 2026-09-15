/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 07:41:12 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 15:15:59 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	do_compile(t_coder *coder, t_sim *sim)
{
	long	start;

	if (request_dongles(coder, sim) != 0)
		return ;
	log_state(sim, coder->id + 1, "is compiling");
	start = now_ms();
	precise_sleep(sim, sim->time_to_compile);
	release_dongles(coder, sim);
	record_compile(coder, start);
}

void	do_phase(t_coder *coder, t_sim *sim, long duration, char *state)
{
	log_state(sim, coder->id + 1, state);
	precise_sleep(sim, duration);
}

void	record_compile(t_coder *coder, long start)
{
	pthread_mutex_lock(&coder->state_mutex);
	coder->compile_count++;
	coder->last_compile_start = start;
	pthread_mutex_unlock(&coder->state_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!sim_stopped(sim))
	{
		do_compile(coder, sim);
		if (sim_stopped(sim))
			break ;
		do_phase(coder, sim, sim->time_to_debug, "is debugging");
		if (sim_stopped(sim))
			break ;
		do_phase(coder, sim, sim->time_to_refactor, "is refactoring");
	}
	return (NULL);
}
