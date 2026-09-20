/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 22:09:41 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/14 22:14:15 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "codexion.h"

t_request	build_request(t_coder *coder, t_sim *sim, long arrival)
{
	t_request	req;

	req.coder_id = coder->id;
	req.arrival = arrival;
	req.deadline = coder->last_compile_start + sim->time_to_burnout;
	return (req);
}

void	lock_both(t_sim *sim, int a, int b)
{
	pthread_mutex_lock(&sim->dongles[a].mutex);
	if (b != a)
		pthread_mutex_lock(&sim->dongles[b].mutex);
}

void	unlock_both(t_sim *sim, int a, int b)
{
	if (b != a)
		pthread_mutex_unlock(&sim->dongles[b].mutex);
	pthread_mutex_unlock(&sim->dongles[a].mutex);
}

int	is_free(t_dongle *d, long now)
{
	return (d->taken == 0 && now >= d->available_at);
}

int	wait_single_dongle(t_sim *sim)
{
	while (!sim_stopped(sim))
		usleep(300);
	return (1);
}
