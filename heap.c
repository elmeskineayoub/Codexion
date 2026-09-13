/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 11:40:22 by aelmeski          #+#    #+#             */
/*   Updated: 2026/09/13 11:40:58 by aelmeski         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// heap_create(capacity, scheduler):
//     heap = malloc(t_heap)
//     heap->data = malloc(capacity * sizeof(t_request))
//     heap->size = 0
//     heap->capacity = capacity
//     heap->scheduler = scheduler
//     return heap





// cmp_request(a, b, scheduler):
//     # returns true if 'a' should come BEFORE 'b' (a has higher priority)
//     if scheduler == FIFO:
//         return a->arrival < b->arrival
//     else:  # EDF
//         if a->deadline != b->deadline:
//             return a->deadline < b->deadline
//         return a->arrival < b->arrival   # tie-break

// heap_push(heap, req):
//     if heap->size == heap->capacity:
//         return error   # shouldn't happen, capacity = num_coders
//     heap->data[heap->size] = req
//     i = heap->size
//     heap->size++
//     sift_up(heap, i)

// sift_up(heap, i):
//     while i > 0:
//         parent = (i - 1) / 2
//         if cmp_request(&heap->data[i], &heap->data[parent], heap->scheduler):
//             swap(heap->data[i], heap->data[parent])
//             i = parent
//         else:
//             break

// heap_pop(heap, out):
//     if heap->size == 0:
//         return error
//     *out = heap->data[0]              # root = highest priority
//     heap->size--
//     heap->data[0] = heap->data[heap->size]   # move last element to root
//     sift_down(heap, 0)
//     return success

// sift_down(heap, i):
//     while true:
//         left = 2*i + 1
//         right = 2*i + 2
//         best = i
//         if left < heap->size and cmp_request(&heap->data[left], &heap->data[best], heap->scheduler):
//             best = left
//         if right < heap->size and cmp_request(&heap->data[right], &heap->data[best], heap->scheduler):
//             best = right
//         if best == i:
//             break
//         swap(heap->data[i], heap->data[best])
//         i = best

// heap_peek(heap, out):
//     if heap->size == 0:
//         return error
//     *out = heap->data[0]
//     return success