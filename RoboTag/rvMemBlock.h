/*
    Fast memory block allocator object.

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

    $Id: rvMemBlock.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_MEMBLOCK_H_
#define _RV_MEMBLOCK_H_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _rvMemBlock rvMemBlock;

// Block memory allocator methods.
rvMemBlock* rvMemBlock_New(int blockCount, int blockSize, int extra);
void rvMemBlock_Free(rvMemBlock* self);
int rvMemBlock_Memory(rvMemBlock* self);
void* rvMemBlock_Extra(rvMemBlock* self);
void* rvMemBlock_AllocBlock(rvMemBlock* self);
void* rvMemBlock_AllocBlockNoClear(rvMemBlock* self);
bool rvMemBlock_FreeBlock(rvMemBlock* self, void *block);
void rvMemBlock_ReclaimMemory(rvMemBlock* self, bool reclaim);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_MEMBLOCK_H_
