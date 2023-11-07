// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "helpers.h"
#include <unistd.h>

typedef struct block_meta *block_ptr;
block_ptr head;

#define META_SIZE sizeof(struct block_meta)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define MMAP_THRESHOLD (128 * 1024)
#define CALLOC_MMAP_THRESHOLD 4096

void *os_malloc(size_t size)
{
	block_ptr new, curr, poss, ceva, ultim;
	// Edge cases
	if (size == 0)
		return NULL;
	// Check if mmap is possible
	if (ALIGN(size) >= MMAP_THRESHOLD) {
		new = (block_ptr) mmap(NULL, ALIGN(size) + ALIGN(META_SIZE),
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		new->size = ALIGN(size);
		new->status = STATUS_MAPPED;
		return (void *)new + ALIGN(META_SIZE);
	}
	// Check if preallocation is needed
	if (head == NULL) {
		head = (block_ptr) sbrk(MMAP_THRESHOLD);
		head->size = MMAP_THRESHOLD - ALIGN(META_SIZE);
		head->next = NULL;
		head->status = STATUS_FREE;
	}
	// If mmap is not possible, try sbrk
	curr = head;
	// Merge free adiacent blocks
	while (curr->next != NULL) {
		if (curr->status == STATUS_FREE && curr->next->status == STATUS_FREE) {
			curr->size = curr->size + curr->next->size + ALIGN(META_SIZE);
			curr->next = curr->next->next;
		} else {
			curr = curr->next;
		}
	}
	// Find best fit for the needed size
	curr = head;
	poss = NULL;
	while (curr != NULL) {
		if (curr->status == STATUS_FREE) {
			if (curr->size >= ALIGN(size)) {
				if (poss != NULL) {
					if (curr->size < poss->size)
						poss = curr;
				} else {
					poss = curr;
				}
			}
		}
		curr = curr->next;
	}
	// If there is no best fit, check if last block is free
	if (poss == NULL) {
		curr = head;
		while (curr->next != NULL)
			curr = curr->next;
		// If it is free, expand
		if (curr->status == STATUS_FREE) {
			sbrk(ALIGN(size) - curr->size);
			curr->status = STATUS_ALLOC;
			curr->size = ALIGN(size);
			return (void *) curr + ALIGN(META_SIZE);
		}
		// Otherwise, allocate a new block and add it to the end of queue
		ultim = (block_ptr) sbrk(ALIGN(size) + ALIGN(META_SIZE));
		ultim->status = STATUS_ALLOC;
		ultim->size = ALIGN(size);
		ultim->next = NULL;
		curr->next = ultim;
		return (void *) ultim + ALIGN(META_SIZE);
	}
	// If best fit is found, check if splitting is possible
	if (poss->size - ALIGN(size) >= ALIGN(META_SIZE) + 1) {
		ceva = poss->next;
		size_t que = poss->size;

		poss->next = (void *) poss + ALIGN(META_SIZE) + ALIGN(size);
		poss->status = STATUS_ALLOC;
		poss->size = ALIGN(size);
		poss->next->status = STATUS_FREE;
		poss->next->size = que - ALIGN(size) - ALIGN(META_SIZE);
		poss->next->next = ceva;
	}
	// Otherwise, return best fit
	poss->status = STATUS_ALLOC;
	return (void *) poss + ALIGN(META_SIZE);
}

void os_free(void *ptr)
{
	// Edge cases
	if (ptr == NULL)
		return;
	block_ptr curr = (block_ptr) ptr - 1;
	// If the block is mapped
	if (curr->status == STATUS_MAPPED) {
		munmap(curr, curr->size + META_SIZE);
		return;
	}
	// If it is not mapped, check if it can be merged with an adiacent one
	curr->status = STATUS_FREE;
	curr = head;
	while (curr->next != NULL) {
		if (curr->status == STATUS_FREE && curr->next->size == STATUS_FREE) {
			curr->size = curr->size + curr->next->size + ALIGN(META_SIZE);
			curr->next = curr->next->next;
		} else {
			curr = curr->next;
		}
	}
}

void *os_calloc(size_t nmemb, size_t size)
{
	block_ptr new, curr, poss, ceva, ultim;
	// Edge cases
	if (size == 0 || nmemb == 0)
		return NULL;
	// Check if mmap is possible
	if (ALIGN(size*nmemb) + ALIGN(META_SIZE) >= CALLOC_MMAP_THRESHOLD) {
		new = (block_ptr) mmap(NULL, ALIGN(nmemb * size) + ALIGN(META_SIZE),
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		new->size = ALIGN(nmemb * size);
		new->status = STATUS_MAPPED;
		memset((void *) new + ALIGN(META_SIZE), 0, ALIGN(nmemb * size));
		return (void *) new + ALIGN(META_SIZE);
	}
	// Check if preallocation is needed
	if (head == NULL) {
		head = (block_ptr) sbrk(MMAP_THRESHOLD);
		head->size = MMAP_THRESHOLD - ALIGN(META_SIZE);
		head->next = NULL;
		head->status = STATUS_FREE;
	}
	// If mmap is not possible, try sbrk
	curr = head;
	// Merge free adiacent blocks
	while (curr->next != NULL) {
		if (curr->status == STATUS_FREE && curr->next->status == STATUS_FREE) {
			curr->size = curr->size + curr->next->size + ALIGN(META_SIZE);
			curr->next = curr->next->next;
		} else {
			curr = curr->next;
		}
	}
	// Find best fit for the needed size
	curr = head;
	poss = NULL;
	while (curr != NULL) {
		if (curr->status == STATUS_FREE) {
			if (curr->size >= ALIGN(size * nmemb)) {
				if (poss != NULL) {
					if (curr->size < poss->size)
						poss = curr;
				} else {
					poss = curr;
				}
			}
		}
		curr = curr->next;
	}
	// If there is no best fit, check if last block is free
	if (poss == NULL) {
		curr = head;
		while (curr->next != NULL)
			curr = curr->next;
		// If it is free, expand
		if (curr->status == STATUS_FREE) {
			sbrk(ALIGN(nmemb * size) - curr->size);
			curr->status = STATUS_ALLOC;
			curr->size = ALIGN(nmemb * size);
			memset((void *) curr + ALIGN(META_SIZE), 0, ALIGN(nmemb * size));
			return (void *) curr + ALIGN(META_SIZE);
		}
		// Otherwise, allocate a new block and add it to the end of queue
		ultim = (block_ptr) sbrk(ALIGN(nmemb * size) + ALIGN(META_SIZE));
		ultim->status = STATUS_ALLOC;
		ultim->size = ALIGN(nmemb * size);
		ultim->next = NULL;
		curr->next = ultim;
		memset((void *) ultim + ALIGN(META_SIZE), 0, ALIGN(nmemb * size));
		return (void *) ultim + ALIGN(META_SIZE);
	}
	// If best fit is found, check if splitting is possible
	if (poss->size - ALIGN(size * nmemb) >= ALIGN(META_SIZE) + 1) {
		ceva = poss->next;
		size_t que = poss->size;

		poss->next = (void *) poss + ALIGN(META_SIZE) + ALIGN(size * nmemb);
		poss->status = STATUS_ALLOC;
		poss->size = ALIGN(size * nmemb);
		poss->next->status = STATUS_FREE;
		poss->next->size = que - ALIGN(size * nmemb) - ALIGN(META_SIZE);
		poss->next->next = ceva;
	}
	// Otherwise, return best fit
	poss->status = STATUS_ALLOC;
	memset((void *) poss + ALIGN(META_SIZE), 0, ALIGN(nmemb * size));
	return (void *) poss + ALIGN(META_SIZE);
}

void *os_realloc(void *ptr, size_t size)
{
	// Edge cases
	if (ptr == NULL)
		return os_malloc(size);
	if (size == 0) {
		os_free(ptr);
		return NULL;
	}
	block_ptr blk = (block_ptr) ptr;
	block_ptr curr = head;

	// If the block is mapped, call malloc
	if (blk->status == STATUS_MAPPED) {
		void *new_ptr = os_malloc(size);

		memcpy(new_ptr, ptr, blk->size);
		os_free(ptr);
		return new_ptr;
	}

	// If it is not mapped
	else {
		while (curr != NULL) {
			// Check if the block is in the queue
			if (curr == blk) {
				size_t remaining_size = curr->size - ALIGN(size);

				// If it is in the queue, check if it can be merged with it neighbour
				if (curr->next != NULL && curr->next->status == STATUS_FREE) {
					curr->size += curr->next->size + ALIGN(META_SIZE);
					remaining_size = curr->size - ALIGN(size);
					curr->next = curr->next->next;
				}

				// Check if it can be split
				if (remaining_size >= ALIGN(META_SIZE) + 1) {
					block_ptr next_block = curr->next;

					curr->next = (void *) curr + ALIGN(META_SIZE) + ALIGN(size);
					curr->next->status = STATUS_FREE;
					curr->next->size = remaining_size - ALIGN(META_SIZE);
					curr->next->next = next_block;

					curr->status = STATUS_ALLOC;
					curr->size = ALIGN(size);

					return (void *) curr + ALIGN(META_SIZE);
				}
				// If the size needed is lower than that of the block, change the size of the block
				else if (curr->size >= ALIGN(size)) {
					curr->status = STATUS_ALLOC;
					curr->size = ALIGN(size);

					return (void *) curr + ALIGN(META_SIZE);
				}
				// Otherwise allocate a new block
				else {
					void *new_ptr = os_malloc(size);

					memcpy(new_ptr, ptr, size);
					os_free(ptr);
					return new_ptr;
				}
			}
			curr = curr->next;
		}

		// If it is not in the queue, call malloc
		void *new_ptr = os_malloc(size);

		memcpy(new_ptr, ptr, size);
		os_free(ptr);
		return new_ptr;
	}
}
