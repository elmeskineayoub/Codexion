/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 22:02:40 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/14 22:13:36 by aelmeski         ###   ########.fr       */
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

void	wait_for_dongle(t_dongle *dongle, long deadline)
{
	long			wait_ms;
	struct timespec	ts;

	if (dongle->taken == 0)
		wait_ms = dongle->available_at - now_ms();
	else
		wait_ms = deadline - now_ms();
	if (wait_ms < 0)
		wait_ms = 0;
	ms_to_abstime(&ts, wait_ms);
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
}

int	wait_single_dongle(t_sim *sim)
{
	while (!sim_stopped(sim))
		usleep(300);
	return (1);
}
