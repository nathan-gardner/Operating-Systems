/**
 * @file kernel.c
 * @author Nathan Gardner
 * @brief
 * @version 0.2.2
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kernel.h" // print_border and k_clearscr header
#include "buddy.h"  // memory allocation header
#include "convert.h"
#include "io.h"    // k_printstr function header
#include "queue.h" // queue library
#include <stddef.h>
#include <stdint.h>

#define WAITFOREVER while (1)
#define BLANK_LINE                                                             \
  "                                                                          " \
  "      "
#define CODE_SEGMENT_REG 16

// ready queue
pcbq_t *ready_q;
// next process id
uint32_t next_pid = 0;
// currently running process
pcb_t *running;
// interrupt descriptor table
idt_entry_t idt[256];

/**
 * @brief Main entry point for operating system
 *
 * @return int
 */
int main() {
  static int p1_status, p2_status; // status indicator for process allocation
  buddy_init();
  k_clearscr();               // call clear screen function (written by me in C)
  print_border(0, 0, 24, 79); // call print border function (written by me in C)
  k_printstr("Running processes", 1, 1);
  ready_q = createQueue();
  init_idt();
  p1_status = create_process((uint32_t)(&p1)); // Create Process 1
  if (p1_status < 0) {
    k_printstr("Error allocating Process 1\0", 22, 1);
  }
  p2_status = create_process((uint32_t)(&p2)); // Create Process 2
  if (p2_status < 0) {
    k_printstr("Error allocating Process 2\0", 23, 1);
  }
  go();
  WAITFOREVER; // wait here forever
  return 0;
}

/**
 * @brief Print a rectagular boarder given the folloing parameters
 *
 * @param start_row start row for print
 * @param start_col start column for print
 * @param end_row end row for print
 * @param end_col end column for print
 */
void print_border(int start_row, int start_col, int end_row, int end_col) {
  // print border code
  for (int i = start_col + 1; i < end_col; i++) {
    k_printstr("-\0", start_row, i);
    k_printstr("-\0", end_row, i);
  }
  for (int i = start_row + 1; i < end_row; i++) {
    k_printstr("|\0", i, start_col);
    k_printstr("|\0", i, end_col);
  }
  k_printstr("+\0", start_row, start_col);
  k_printstr("+\0", start_row, end_col);
  k_printstr("+\0", end_row, start_col);
  k_printstr("+\0", end_row, end_col);
}

/**
 * @brief Print a general error to the screen
 */
void print_general_error() {
  k_clearscr();
  k_printstr("ERROR\0", 0, 0);
  WAITFOREVER;
}

/**
 * @brief Clear screen by printing 25 rows of of 80 blank characters
 *
 */
void k_clearscr() {
  // code to clear the screen
  for (int row = 0; row < 25; row++) {
    k_printstr(BLANK_LINE, row, 0);
  }
}

/**
 * @brief Allocate the stack for a process in memory. This stack will maintain
 * the process state between process switches. Also pushes PCB struct onto ready
 * queue to signify that the process is ready to be used.
 *
 * @param code_address The address of the first instruction of the process code
 * in memory
 * @return int zero:success, non-zero:failure
 */
int create_process(uint32_t code_address) {
  uint32_t *stackptr = (uint32_t *)kmalloc(1024 * sizeof(int));
  uint32_t *st = stackptr + 1024;
  // eflags
  *(--st) = 0x0;
  // cs
  *(--st) = CODE_SEGMENT_REG;
  // code address
  *(--st) = code_address;
  // dispatch_leave
  *(--st) = (uint32_t)dispatch_leave;
  // allocate registers for process
  // ebp
  *(--st) = 0;
  // esp
  *(--st) = 0;
  // edi
  *(--st) = 0;
  // esi
  *(--st) = 0;
  // edx
  *(--st) = 0;
  // ecx
  *(--st) = 0;
  // ebx
  *(--st) = 0;
  // eax
  *(--st) = 0;

  pcb_t *pcb = newPCB((uint32_t)st, next_pid);
  next_pid++;

  enqueue(ready_q, pcb);

  return 0;
}

/**
 * @brief Code for Process 1
 *
 */
void p1() {
  static int num = 0;
  char buf[4];
  print_border(10, 10, 13, 35);
  k_printstr("Process 1 running...\0", 11, 11);
  k_printstr("value: \0", 12, 11);
  while (1) {
    convert_num(num, buf);
    k_printstr(buf, 12, 18);
    num++;
    if (num >= 1000) {
      num = 0;
    }
    asm("int $32"); // Call dispatcher
  }
}

/**
 * @brief Code for Process 2
 *
 */
void p2() {
  static int num = 0;
  char buf[4];
  print_border(15, 10, 18, 35);
  k_printstr("Process 2 running...\0", 16, 11);
  k_printstr("value: \0", 17, 11);
  while (1) {
    convert_num(num, buf);
    k_printstr(buf, 17, 18);
    num++;
    if (num >= 1000) {
      num = 0;
    }
    asm("int $32"); // Call dispatcher
  }
}

void init_idt_entry(idt_entry_t *entry, uint32_t addr_of_handler,
                    uint16_t code_selector, uint8_t access) {
  entry->base_low16 = (uint16_t)(addr_of_handler & 0x0000ffff);
  entry->selector = code_selector;
  entry->always0 = 0;
  entry->access = access;
  entry->base_hi16 = (uint16_t)((addr_of_handler >> 16) & 0x0000ffff);
}

void init_idt() {
  for (int i = 0; i < 32; i++) {
    init_idt_entry(&idt[i], (uint32_t)print_general_error, 16, 0x8e);
  };
  init_idt_entry(&idt[32], (uint32_t)dispatch, 16, 0x8e);
  for (int i = 33; i < 256; i++) {
    init_idt_entry(&idt[i], 0, 0, 0);
  };
  idtr_t idtr;
  idtr.base = (uint32_t)(&idt);
  idtr.limit = sizeof(idt) - 1;
  lidtr(&idtr);
}
