//
// Created by flasque on 19/10/2024.
//

#include <assert.h>
#include <stdlib.h>
#include "queue.h"

/**
 * @brief Function to create a queue
 * @param size : the size of the queue
 * @return the queue
 */
t_queue createQueue(int size)
{
    // the size of the queue must be positive
    assert(size > 0);
    t_queue queue;
    queue.size = size;
    queue.first = 0;
    queue.last = 0;
    queue.values = (position_s *)malloc(size * sizeof(position_s));
    return queue;
}

void enqueue(t_queue *p_queue, position_s pos)
{
    // the queue must not be full
    assert((p_queue->last - p_queue->first) < p_queue->size);
    p_queue->values[(p_queue->last) % p_queue->size] = pos;
    p_queue->last++;
    return;
}

position_s dequeue(t_queue *p_queue)
{
    // the queue must not be empty
    assert(p_queue->last != p_queue->first);
    p_queue->first++;
    return p_queue->values[(p_queue->first - 1) % p_queue->size];
}