/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 11:40:22 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/14 21:42:03 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (cmp_request(&heap->data[i], &heap->data[parent], heap->scheduler))
		{
			swap(&heap->data[i], &heap->data[parent]);
			i = parent;
		}
		else
			break ;
	}
}

int	heap_push(t_heap *heap, t_request req)
{
	int	i;

	if (heap->size == heap->capacity)
		return (1);
	heap->data[heap->size] = req;
	i = heap->size;
	heap->size++;
	sift_up(heap, i);
	return (0);
}

void	sift_down(t_heap *heap, int i)
{
	int	left;
	int	best;
	int	right;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < heap->size && cmp_request(&heap->data[left],
				&heap->data[best], heap->scheduler))
			best = left;
		if (right < heap->size && cmp_request(&heap->data[right],
				&heap->data[best], heap->scheduler))
			best = right;
		if (best == i)
			break ;
		swap(&heap->data[i], &heap->data[best]);
		i = best;
	}
}

int	heap_pop(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (1);
	*out = heap->data[0];
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	sift_down(heap, 0);
	return (0);
}

int	heap_peek(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (1);
	*out = heap->data[0];
	return (0);
}
