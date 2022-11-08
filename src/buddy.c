//#include <stdio.h>
#include "buddy.h"
#include "convert.h"
#include "io.h"
#include <stdint.h>

#define HEAP_BASE (1024 * 1024) /* 1k * 1024 = 1 Mb */
#define HEAP_SIZE (1024 * 1024) /* allocatoin starts at 1Mb */

#define LEFT(idx) (2 * idx + 1)
#define RIGHT(idx) (2 * idx + 2)
#define SIBLING(idx) (idx % 2 ? idx - 1 : idx + 1)
#define PARENT(idx) ((idx - 1) / 2)
#define ROOT 0
#define IS_LEAF(t, idx) (!(t[LEFT(idx)].exists || t[RIGHT(idx)].exists))
#define MIN_NODE_SIZE 64 /* don't allow less that 64 bytes to be allocated */
/* If min_node_size is 64, then the max number of nodes in the tree is 32767.
   Why?  Because a tree of height 15 has 16384 leaves and 64*16384 is 1Mb.
   Futhermore, a tree of height 15 has (2^15)-1 MAX_NODES, which is 32767. */
#define MAX_NODES 32767

struct mem_node {
  uint32_t offset;
  uint32_t size;
  int allocated;
  int exists;
};
typedef struct mem_node mem_node_t;

// mem_node_t tree[MAX_NODES];
mem_node_t *tree = (mem_node_t *)0x200000; // point to memory at 2Mb, after heap

void run_test() {

  buddy_init();
  int row = 1;
  int col = 1;
  void *alloc_list[15];
  char buffer[20];
  uint32_t total;
  k_printstr("Testing heap memory allocator...", row, col);
  row++;
  for (int i = 0; i < 15; i++) {
    alloc_list[i] = kmalloc(i * 64);
    if (alloc_list[i] == 0) {
      k_printstr("kmalloc returned a null pointer for allocation ", row, col);
      int len1 = convert_num(i, buffer);
      k_printstr(buffer, row, col + 47);
      k_printstr(" of ", row, col + 47 + len1);
      int len2 = convert_num((uint32_t)alloc_list[i], buffer);
      k_printstr(buffer, row, col + 47 + len1 + 4);
      k_printstr(" bytes", row, col + 47 + len1 + 4 + len2);
    } else {
      k_printstr("kmalloc returned ", row, col);
      convert_num((uint32_t)alloc_list[i], buffer);
      k_printstr(buffer, row, col + 17);
      total += i * 64;
    }
    row++;
  }

  // printf("Total alloation is: %f Mb\n", total/1024.0/1024);

  // heap_walk();

  k_printstr("Freeing...", row, 1);

  // do a bad free
  kfree((void *)234);

  for (int i = 74; i >= 0; i--) {
    kfree(alloc_list[i]);
  }

  for (int i = 150; i >= 75; i--) {
    kfree(alloc_list[i]);
  }

  // heap_walk();
}

void heap_walk_helper(mem_node_t *t, int idx);
int get_malloc_idx(mem_node_t *t, int request);
int get_free_idx_find_exact(mem_node_t *t, int start_idx, int offset);
int get_malloc_idx_find(mem_node_t *t, int start_idx, int request);
int get_malloc_idx_cut(mem_node_t *t, int start_idx, int request);
void buddy_init_i(mem_node_t *t, int max);

void buddy_init() { buddy_init_i(tree, MAX_NODES); }

void heap_walk() { heap_walk_helper(tree, 0); }

void heap_walk_helper(mem_node_t *t, int idx) {
  // printf("(n:%d,o:%x,s:%u,a?:%d)", idx, (uint32_t)t[idx].offset, t[idx].size,
  // t[idx].allocated); fflush(stdout);
  int left = LEFT(idx);
  int right = RIGHT(idx);
  if (t[left].exists)
    heap_walk_helper(t, left);
  if (t[right].exists)
    heap_walk_helper(t, right);
}

void buddy_init_i(mem_node_t *t, int max) {
  for (int i = 0; i < max; i++) {
    char buffer[20];
    t[i].size = 0;
    t[i].offset = 0;
    t[i].allocated = 0;
    t[i].exists = 0;
    convert_num(i, buffer);
    k_printstr(buffer, 2, 1);
  }
  t[0].size = HEAP_SIZE;
  t[0].offset = 0;
  t[0].allocated = 0;
  t[0].exists = 1;
}

void *kmalloc(uint32_t size) {
  if (size == 0)
    return 0;
  int idx = get_malloc_idx(tree, size);
  if (idx == -1)
    return 0;
  tree[idx].allocated = 1;
  return (void *)((uint32_t)HEAP_BASE + tree[idx].offset);
}

int get_malloc_idx(mem_node_t *t, int request) {
  int start_idx = get_malloc_idx_find(t, 0, request);
  if (start_idx == -1)
    return -1;
  return get_malloc_idx_cut(t, start_idx, request);
}

int get_malloc_idx_find(mem_node_t *t, int start_idx, int request) {
  /* go to unallocated leaf with size >= request
     return -1 if no such leaf is found */

  /* stop the recursion if we get to a node that is too small */
  if (request > t[start_idx].size)
    return -1;
  /* is it a leaf ? */
  // if (t[LEFT(start_idx)].exists == 0 && t[RIGHT(start_idx)].exists == 0) {
  if (IS_LEAF(t, start_idx)) {
    if (t[start_idx].allocated) /* already allocated ? */
      return -1;
    /* is it big enought ? */
    if (t[start_idx].size >= request)
      return start_idx;
    else
      return -1;
  } else { /* not a leaf */
    int idx = get_malloc_idx_find(t, LEFT(start_idx), request);
    if (idx == -1)
      idx = get_malloc_idx_find(t, RIGHT(start_idx), request);
    return idx;
  }
}

int get_malloc_idx_cut(mem_node_t *t, int start_idx, int request) {

  /* Prerequisite: start_idx should point to unallocated leaf at least
     request size. */

  int leaf_size = t[start_idx].size;
  /* if we cannot cut the node... */
  if ((leaf_size == MIN_NODE_SIZE && request < MIN_NODE_SIZE) ||
      request > leaf_size / 2)
    return start_idx;
  else {
    // assert(leaf_size > MIN_NODE_SIZE); /* can be cut */

    /* cut leaf in half by making two new children */
    int left = LEFT(start_idx);
    int right = RIGHT(start_idx);
    int cut_size = leaf_size / 2;
    t[left].size = cut_size;
    ;
    t[left].offset = t[start_idx].offset;
    t[left].allocated = 0;
    t[left].exists = 1;
    t[right].size = cut_size;
    t[right].offset = t[start_idx].offset + cut_size;
    t[right].allocated = 0;
    t[right].exists = 1;

    /* now try to allocate from the new left child */
    return get_malloc_idx_cut(t, left, request);
  }
}

void kfree(void *mem) {
  uint32_t offset = (uint32_t)(mem - HEAP_BASE);

  int idx = get_free_idx_find_exact(tree, 0, offset);

  if (idx == -1) {
    char buffer[20];
    k_printstr("Invalid free of ", 24, 0);
    convert_num((uint32_t)mem, buffer);
    k_printstr(buffer, 24, 16);
  }
}

int get_free_idx_find_exact(mem_node_t *t, int start_idx, int offset) {
  if (t[start_idx].offset == offset && IS_LEAF(t, start_idx)) {
    t[start_idx].allocated = 0; // free it
    return start_idx;
  }
  /* if node has no children, then return -1 (and it is not the right address)*/
  if (IS_LEAF(t, start_idx))
    return -1;
  int idx = -1;
  if (offset >= (t[start_idx].offset + t[start_idx].size / 2)) { /* go right */
    idx = get_free_idx_find_exact(t, RIGHT(start_idx), offset);
  } else {
    idx = get_free_idx_find_exact(t, LEFT(start_idx), offset);
  }
  // if idx != -1, then check the children.  If all are unused (allocated == 0),
  // then set them to non-existant (exists = 0), So this node becomes a leaf.
  // So, as we unwind the recursion, we coalescenodes.
  if (idx != -1) {
    if (IS_LEAF(t, LEFT(start_idx)) && IS_LEAF(t, RIGHT(start_idx))) {
      if (t[LEFT(start_idx)].allocated == 0 &&
          t[RIGHT(start_idx)].allocated == 0) {
        t[LEFT(start_idx)].exists = 0;
        t[RIGHT(start_idx)].exists = 0;
      }
    }
  }
  return idx;
}
