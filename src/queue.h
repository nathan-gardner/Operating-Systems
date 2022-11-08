/**
 * @file queue.h
 * @author Nathan Gardner
 * @brief
 * @version 0.2.2
 * @date 2022-10-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

/**
 * @brief Process Control Block Type
 *
 */
typedef struct block {
  // all of the data inside the process control block
  uint32_t esp;
  int pid;
  struct block *next;
} pcb_t;

/**
 * @brief Process Control Block LinkedList type with pointer to front and rear
 * of list
 *
 */
typedef struct queue {
  pcb_t *front, *rear;
} pcbq_t;

/**
 * @brief Create a Queue object
 *
 * @return pcbq_t*
 */
pcbq_t *createQueue();
/**
 * @brief Create a new node for the Process Control Block
 *
 * @param esp stack pointer
 * @param pid process id
 * @return pcb_t*
 */
pcb_t *newPCB(uint32_t esp, int pid);
/**
 * @brief Insert node into LinkedList at the rear
 *
 * @param q pointer to the queue
 * @param pcb node that needs to be enqueued into the LinkedList
 */
void enqueue(pcbq_t *q, pcb_t *pcb);
/**
 * @brief Pop node off the front of the LinkedList and return node
 *
 * @param q pointer to the queue
 * @return pcb_t*
 */
pcb_t *dequeue(pcbq_t *q);

#endif