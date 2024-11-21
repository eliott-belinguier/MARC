//
// Created by flasque on 19/10/2024.
//

#ifndef UNTITLED1_QUEUE_H
#define UNTITLED1_QUEUE_H
#include "loc.h"
/**
 * @brief Structure for the queue of integers
 */
typedef struct s_queue
{
    position_s *values;
    int size;
    int last;
    int first;
} t_queue;

/**
 * @brief Function to create a queue
 * @param size : the size of the queue
 * @return the queue
 */
t_queue createQueue(int);

/**
 * @brief Function to enqueue a value in the queue
 * @param p_queue : pointer to the queue
 * @param value : the position to enqueue
 * @return none
 */
void enqueue(t_queue *,position_s);

/**
 * @brief Function to dequeue a value from the queue
 * @param p_queue : pointer to the queue
 * @return the value dequeued
 */
position_s dequeue(t_queue *);

#endif //UNTITLED1_QUEUE_H
