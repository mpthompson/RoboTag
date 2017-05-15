/*
    This code is intended to implement a very fast, low-overhead sub-allocator to
    prevent fragmentation of memory.  A memory pools is meant to be used making
    many small memory allocations that are all freed at once -- meaning you can't
    independently free a memory block from the sub-allocator.  The only way to free
    memory is to reset or free the entire memory pool.

    This allocator is much faster than malloc when dealing with many small
    allocations ( < 256 bytes). Its performance approaches malloc as the request
    size increases.

    Copyright (C) 2010, Michael P. Thompson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 2 as 
    specified in the README.txt file or as published by the Free Software 
    Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    $Id: rvMemPool.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rvMemPool.h"

// Mempool object and block header.
struct _rvMemPool
{
    int size;
    int used;
    rvMemPool *next;
    // Alloc space follows here.
};

static __inline void *rvMemPool_AllocBlock(rvMemPool *self, size_t nbytes)
{
    // Allocate the memory block.
    return (rvMemPool *) malloc(nbytes);
}

static __inline void
rvMemPool_FreeBlock(rvMemPool *self, void *block)
{
    // Free the memory block.
    free(block);
}


rvMemPool *
rvMemPool_New(int allocSize)
// Allocate a new rvMemPool of allocSize bytes. Additional
// memory is allocated in allocSize chunks, as needed.
{
    rvMemPool *self;

    // Use default allocation size?
    if (allocSize == 0) allocSize = 8192;

    // Tiny allocation sizes are pointless. We need at least sizeof(rvMemPool).
    if (allocSize < 512) allocSize = 512;

    // Add the memory management overhead to the size of the memory allocated.
    allocSize += sizeof(rvMemPool);

    // Allocate the object.
    self = (rvMemPool *) malloc(allocSize);

    // Initialize this block.
    self->size = allocSize;
    self->used = sizeof(rvMemPool);

    // This is a circular list. self->next is always the most recently added block.
    self->next = self;

    return self;
}


void
rvMemPool_Free(rvMemPool *self)
// Free the rvMemPool and all its memory.
{
    // Reset frees any extra blocks.
    rvMemPool_Reset(self);

    // Free this object.
    free(self);
}


void
rvMemPool_Reset(rvMemPool *self)
// Release all the space allocated from the rvMemPool but don't
// free the rvMemPool itself.
{
    // Free any linked blocks.
    rvMemPool *pool = self->next;
    while (pool != self)
    {
        rvMemPool *next = pool->next;
        rvMemPool_FreeBlock(self, pool);
        pool = next;
    }

    // Back to a circle of one.
    self->next = self;

    // Release all the space in the first block.
    self->used = sizeof(rvMemPool);
}


void *
rvMemPool_Alloc(rvMemPool *self, int size)
// Allocate size bytes from this pool. Expands the pool as needed to
// satisfy the request. Allocations may be larger than the initial
// allocSize passed to rvMemPool_New.
{
    int allocSize;
    rvMemPool *pool;

    // See if there's space in the current block.
    pool = self->next;
    if (pool->size - pool->used >= size)
    {
        pool->used += size;
        return (char *)pool + pool->used - size;
    }

    // Couldn't get space in the current block, so add a new one.
    allocSize = self->size;
    if (size + (int) sizeof(rvMemPool) > self->size)
        allocSize = size + sizeof(rvMemPool);

    pool = (rvMemPool*) rvMemPool_AllocBlock(self, allocSize);
    pool->size = allocSize;
    pool->used = sizeof(rvMemPool) + size;
    pool->next = self->next;
    self->next = pool;

    return (char *) pool + sizeof(rvMemPool);
}


char *
rvMemPool_AllocStr(rvMemPool *self, const char *str)
// Duplicate this string in the rvMemPool, returns a pointer to the string.
{
    return strcpy((char *) rvMemPool_Alloc(self, (int) strlen(str) + 1), str);
}

#ifdef DEBUG

void
rvMemPool_Dump(rvMemPool *self)
// Debug routine to dump a rvMemPool.
{
    int i = 1;
    int total = 0;
    int used = 0;
    rvMemPool *pool = self;
    do
    {
        printf("rvMemPool[%d], size %d, used %d, remaining %d\n", i, pool->size, pool->used, (pool->size - pool->used));
        total += pool->size;
        used += pool->used;
        pool = pool->next;
        ++i;
    } while (pool != self);

    printf("Total %d, used %d, waste %d (%d%%)\n", total, used, (total-used), (total-used) * 100 / total);
}

#endif