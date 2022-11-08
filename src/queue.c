/**
 * @file queue.c
 * @author Nathan Gardner
 * @brief
 * @version 0.2.2
 * @date 2022-10-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "buddy.h"
#include "queue.h"

/**
 * @brief Create a Queue object
 *
 * @return pcbq_t*
 */
pcbq_t *createQueue() {
  pcbq_t *q = (pcbq_t *)kmalloc(sizeof(pcbq_t));
  q->rear = NULL;
  q->front = NULL;
  return q;
}

/**
 * @brief Create a new node for the Process Control Block
 *
 * @param esp stack pointer
 * @param pid process id
 * @return pcb_t*
 */
pcb_t *newPCB(uint32_t esp, int pid) {
  pcb_t *temp = (pcb_t *)kmalloc(sizeof(pcb_t));
  temp->esp = esp;
  temp->pid = pid;
  temp->next = NULL;
  return temp;
}

/**
 * @brief Insert node into LinkedList at the rear
 *
 * @param q pointer to the queue
 * @param pcb node that needs to be enqueued into the LinkedList
 */
void enqueue(pcbq_t *q, pcb_t *pcb) {
  // Create a new linkedlist node
  if (q->rear == NULL) {
    q->front = q->rear = pcb;
    return;
  }

  // Add the node to the end of the queue by setting next to new node and
  // changing rear to new node
  q->rear->next = pcb;
  q->rear = pcb;
}

/**
 * @brief Pop node off the front of the LinkedList and return node
 *
 * @param q pointer to the queue
 * @return pcb_t*
 */
pcb_t *dequeue(pcbq_t *q) {
  // If the queue is empty, return NULL
  if (q->front == NULL) {
    return NULL;
  }
  pcb_t *temp = q->front;
  // Set new front for bookkeeping
  q->front = q->front->next;
  // If front becomes NULL, rear also becomes NULL
  if (q->front == NULL) {
    q->rear = NULL;
  }
  return temp;
}