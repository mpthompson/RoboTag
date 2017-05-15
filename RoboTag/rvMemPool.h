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

    $Id: rvMemPool.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_MEMPOOL_H_
#define _RV_MEMPOOL_H_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// MemPool types.
typedef struct _rvMemPool rvMemPool;

// MemPool table methods.
extern rvMemPool *rvMemPool_New(int allocSize);
extern void rvMemPool_Free(rvMemPool *self);
extern void rvMemPool_Reset(rvMemPool *self);
extern void *rvMemPool_Alloc(rvMemPool *self, int size);
extern char *rvMemPool_AllocStr(rvMemPool *self, const char *str);

// Utility and debug functions.
#ifdef DEBUG
extern void rvMemPool_Dump(rvMemPool *self);
#endif

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_MEMPOOL_H_
