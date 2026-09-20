/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 18:28:52 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/20 14:17:13 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	cmp_request(t_request *a, t_request *b, t_scheduler sched)
{
	if (sched == FIFO)
	{
		if (a->arrival != b->arrival)
			return (a->arrival < b->arrival);
	}
	if (sched == EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
		if (a->arrival != b->arrival)
			return (a->arrival < b->arrival);
	}
	return (a->coder_id < b->coder_id);
}

t_heap	*heap_create(int capacity, int scheduler)
{
	t_heap	*heap;

	heap = malloc(sizeof(t_heap));
	if (!heap)
		return (NULL);
	heap->data = malloc(capacity * sizeof(t_request));
	if (!heap->data)
	{
		free(heap);
		return (NULL);
	}
	heap->size = 0;
	heap->capacity = capacity;
	heap->scheduler = scheduler;
	return (heap);
}

void	heap_destroy(t_heap *heap)
{
	if (!heap)
		return ;
	free(heap->data);
	free(heap);
}
