#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>

void *kmalloc(uint32_t size);
void kfree(void *mem);
void buddy_init();
void run_test();
void heap_walk();

#endif
