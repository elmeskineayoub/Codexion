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

int	is_available(t_dongle *dongle)
{
	return (dongle->taken == 0 && now_ms() >= dongle->available_at);
}

int	acquire_one(t_dongle *dongle, t_coder *coder, t_sim *sim)
{
	t_request	req;
	t_request	front;

	pthread_mutex_lock(&dongle->mutex);
	req = build_request(coder, sim);
	heap_push(dongle->waiters, req);
	while (!sim_stopped(sim))
	{
		heap_peek(dongle->waiters, &front);
		if (is_available(dongle) && front.coder_id == coder->id)
		{
			heap_pop(dongle->waiters, &front);
			dongle->taken = 1;
			log_state(sim, coder->id, "has taken a dongle");
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		wait_for_dongle(dongle, req.deadline);
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

int	request_dongles(t_coder *coder, t_sim *sim)
{
	int	first;
	int	second;

	if (coder->left_dongle == coder->right_dongle)
		return (wait_single_dongle(sim));
	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first > second)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (acquire_one(&sim->dongles[first], coder, sim) != 0)
		return (1);
	if (acquire_one(&sim->dongles[second], coder, sim) != 0)
	{
		release_one(&sim->dongles[first], sim);
		return (1);
	}
	return (0);
}

void	release_one(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->taken = 0;
	dongle->available_at = now_ms() + sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_coder *coder, t_sim *sim)
{
	release_one(&sim->dongles[coder->left_dongle], sim);
	release_one(&sim->dongles[coder->right_dongle], sim);
}
