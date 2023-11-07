// SPDX-License-Identifier: BSD-3-Clause

#include <internal/mm/mem_list.h>
#include <internal/types.h>
#include <internal/essentials.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

void *malloc(size_t size)
{
	void *plen;
    plen = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	mem_list_add(plen, size);
	return plen;
}

void *calloc(size_t nmemb, size_t size)
{
	void *p;
	p = malloc(size * nmemb);
	p = memset(p, 0, size * nmemb);
	return p;
}

void free(void *ptr)
{
	struct mem_list *here = mem_list_find(ptr);
	munmap(here, sizeof(here));
	mem_list_del(here);
}

void *realloc(void *ptr, size_t size)
{
	struct mem_list *new = mem_list_find(ptr);
	mem_list_del(ptr);
	void *new_ptr = malloc(size);
	memcpy(new_ptr, new, sizeof(new));

	return new_ptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	struct mem_list *new = mem_list_find(ptr);
	mem_list_del(ptr);
	void *new_ptr = malloc(size * nmemb);
	memcpy(new_ptr, new, sizeof(new));
	return new_ptr;
}
