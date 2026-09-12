/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 23:26:42 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/12 23:49:01 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_error(char *msg)
{
	write(2, msg, ft_strlen(msg));
}

int	get_scheduler(char *str)
{
	if (ft_strcmp(str, "fifo") == 0)
		return (FIFO);
	if (ft_strcmp(str, "edf") == 0)
		return (EDF);
	return (-1);
}

int	parse_numeric_args(char **argv, int *values)
{
	int	i;
	int	verified;

	i = 1;
	while (i <= 7)
	{
		if (!is_number(argv[i]))
		{
			print_error("[ERROR] invalid numeric argument!\n");
			exit(1);
		}
		values[i - 1] = ft_atoi(argv[i]);
		if (i - 1 == 0)
			verified = (values[i - 1] > 0);
		else
			verified = (values[i - 1] >= 0);
		if (verified)
		{
			print_error("[ERROR] invalid argument value!\n");
			exit(1);
		}
		i++;
	}
	return (0);
}

void	fill_sim(t_sim *sim, int *values, int sched)
{
	sim->num_coders = values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->compiles_required = values[5];
	sim->dongle_cooldown = values[6];
	sim->scheduler = sched;
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	int	values[7];
	int	sched;

	if (argc != 9)
	{
		print_error("[ERROR] wrong number of arguments!\n");
		exit(1);
	}
	parse_numeric_args(argv, values);
	sched = get_scheduler(argv[8]);
	if (sched == -1)
	{
		print_error("[ERROR] scheduler must be fifo or edf!\n");
		exit(1);
	}
	fill_sim(sim, values, sched);
	return (0);
}
