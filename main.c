/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:13:11 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/15 16:34:13 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// launch_threads(sim) -> int:
//     i = 0
//     while i < sim->num_coders:
//         if pthread_create(&sim->coder_threads[i], NULL,
//                           coder_routine, &sim->coders[i]) != 0:
//             return 1
//         i++
//     if pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0:
//         return 1
//     return 0


int launch_threads(t_sim *sim)
{
	int i;
	i = 0;
	while (i < sim->num_coders)
	{
		if (pthread_create(&sim->coder_threads[i], NULL, coder_routine, &sim->coders[i]) != 0)
			return 1;
		i++;
	}
	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
		return 1;
	return 0;
}
int	main(int argc, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(t_sim));
	parse_args(argc, argv, &sim);
	if (init_sim(&sim) != 0)
	{
		print_error("[ERROR] initialization failed!\n");
		cleanup_all(&sim);
		return (1);
	}
	if (launch_threads(&sim) != 0)
	{
		print_error("[ERROR] thread creation failed!\n");
		cleanup_all(&sim);
		return (1);
	}
	join_all(&sim);
	cleanup_all(&sim);
	return (0);
}