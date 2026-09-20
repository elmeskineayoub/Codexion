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

void	order_dongles(t_coder *coder, int *a, int *b)
{
	*a = coder->left_dongle;
	*b = coder->right_dongle;
	if (*a > *b)
	{
		*a = coder->right_dongle;
		*b = coder->left_dongle;
	}
}

int	can_take_both(t_coder *coder, t_sim *sim)
{
	t_request	f;
	long		now;

	now = now_ms();
	if (!is_free(&sim->dongles[coder->left_dongle], now))
		return (0);
	if (!is_free(&sim->dongles[coder->right_dongle], now))
		return (0);
	if (heap_peek(sim->dongles[coder->left_dongle].waiters, &f) != 0)
		return (0);
	if (f.coder_id != coder->id)
		return (0);
	if (heap_peek(sim->dongles[coder->right_dongle].waiters, &f) != 0)
		return (0);
	if (f.coder_id != coder->id)
		return (0);
	return (1);
}

void	take_both(t_coder *coder, t_sim *sim)
{
	t_request	f;

	heap_pop(sim->dongles[coder->left_dongle].waiters, &f);
	heap_pop(sim->dongles[coder->right_dongle].waiters, &f);
	sim->dongles[coder->left_dongle].taken = 1;
	sim->dongles[coder->right_dongle].taken = 1;
	log_state(sim, coder->id + 1, "has taken a dongle");
	log_state(sim, coder->id + 1, "has taken a dongle");
}

int	request_dongles(t_coder *coder, t_sim *sim)
{
	t_request	req;
	int			a;
	int			b;

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
		unlock_both(sim, a, b);
		usleep(200);
		lock_both(sim, a, b);
	}
	unlock_both(sim, a, b);
	return (1);
}

void	release_dongles(t_coder *coder, t_sim *sim)
{
	int		a;
	int		b;
	long	now;

	order_dongles(coder, &a, &b);
	lock_both(sim, a, b);
	now = now_ms();
	sim->dongles[a].taken = 0;
	sim->dongles[a].available_at = now + sim->dongle_cooldown;
	sim->dongles[b].taken = 0;
	sim->dongles[b].available_at = now + sim->dongle_cooldown;
	unlock_both(sim, a, b);
}
