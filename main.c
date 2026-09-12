/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:13:11 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/13 00:13:23 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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