/**
 * @file kernel.h
 * @author Nathan Gardner
 * @brief
 * @version 0.2.2
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

struct idt_entry {
  uint16_t base_low16;
  uint16_t selector; // always 16
  uint8_t always0;
  uint8_t access; // always 0x8e
  uint16_t base_hi16;
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
typedef struct idtr idtr_t;

/**
 * @brief Print a rectagular boarder given the folloing parameters
 *
 * @param start_row start row for print
 * @param start_col start column for print
 * @param end_row end row for print
 * @param end_col end column for print
 */

/**
 * @brief Print a general error to the screen
 */
void print_general_error();

void print_border(int start_row, int start_col, int end_row, int end_col);
/**
 * @brief Clear screen by printing 25 rows of of 80 blank characters
 *
 */
void k_clearscr();
/**
 * @brief Jump to begin running processes
 *
 */
void go(); // written in assembly
/**
 * @brief Executes "iret" x86 assembly instruction
 *
 */
void dispatch_leave();
/**
 * @brief Call to dispatcher to yield
 *
 */
void dispatch();
/**
 * @brief Allocate the stack for a process in memory. This stack will maintain
 * the process state between process switches. Also pushes PCB struct onto ready
 * queue to signify that the process is ready to be used.
 *
 * @param code_address The address of the first instruction of the process code
 * in memory
 * @return int zero:success, non-zero:failure
 */
int create_process(uint32_t code_address);
/**
 * @brief Code for Process 1
 *
 */
void p1();
/**
 * @brief Code for Process 2
 *
 */
void p2();
/**
 * @brief Initializes the values in the interrupt descriptor table
 *
 * @param entry
 * @param addr_of_handler
 * @param code_selector
 * @param access
 */
void init_idt_entry(idt_entry_t *entry, uint32_t addr_of_handler,
                    uint16_t code_selector, uint8_t access);

/**
 * @brief Init the interrupt descriptor table
 *
 */
void init_idt();

/**
 * @brief Call to execute lidt function in assembly
 *
 * @param idtr
 */
void lidtr(idtr_t *idtr);

#endif
