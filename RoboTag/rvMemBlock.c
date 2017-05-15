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

    $Id: rvMemBlock.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include <string.h>
#include "rvMemBlock.h"

// #define DEBUG_MEMBLOCK

typedef struct _rvMemBlockItem rvMemBlockItem;
typedef struct _rvMemBlockGroup rvMemBlockGroup;

// This is a dynamic length structure that contains a single
// object allocated within a memblock group.  In addition to
// the object data it contains a pointer that serves to link
// free items together or points to the parent group when
// the item is allocated to an object.
struct _rvMemBlockItem
{
    union
    {
        rvMemBlockItem* next;
        rvMemBlockGroup* parent;
    } info;
    int data[1];
};

// This is a dynamic length structure that contains one or more
// memblock items.  It contains information to indicate the number
// and location of allocated and free items within the group.
// Other information is used to keep track of elements within
// a link list managed by the memblock allocator.
struct _rvMemBlockGroup
{
    int used;
    int count;
    rvMemBlock* parent;
    rvMemBlockItem* free;
    rvMemBlockGroup* prev;
    rvMemBlockGroup* next;
    rvMemBlockItem items[1];
};

// This structure represents a single memblock allocator.
struct _rvMemBlock
{
    int used;               // Master count of allocated items.
    int count;              // Master count of available items.
    int blockSize;          // The size of a block.
    int blockCount;         // The count of blocks to allocate from system heap.
    bool reclaim;           // TRUE if memory should be reclaimed after rvMemBlock_FreeBlock.
    rvMemBlockGroup* head;  // Head of the group list.
    rvMemBlockGroup* tail;  // Tail of the group list.
};


#ifdef DEBUG_MEMBLOCKS

static void
rvMemBlock_Check(rvMemBlock* self)
// Validate the memblock structure.
{
    int used;
    int element_used;
    rvMemBlockGroup* group;

    // Reset the used count.
    used = 0;

    // Start at the head of the group list.
    group = self->head;

    // Loop over each group.
    while (group)
    {
        rvMemBlockItem* item;

        // Reset the group used count.
        element_used = self->blockCount;

        // Point to the unused item.
        item = group->free;

        // Loop over each unused item.
        while (item != NULL)
        {
            // Decrement the group used count.
            --element_used;

            // Get the next unused item.
            item = item->info.next;
        }

        // Validate the item count.
        if (element_used != group->used)
        {
            // Cause a segment violation.
            *((char*) 0) = 0;
        }

        // Add to the group used count.
        used += element_used;

        // Set the next group.
        group = group->next;
    }

    // Validate the item count.
    if (used != self->used)
    {
        // Cause a segment violation.
        *((char*) 0) = 0;
    }
}

#endif

rvMemBlock* rvMemBlock_New(int blockCount, int blockSize, int extra)
// Creates a new memblock object.  Blocks of blockSize bytes are
// allocated from the system heap in blockCount increments.  An
// arbitrary number of blocks can be allocated.
{
    rvMemBlock* self;

    // Sanity check the arguments.
    if (blockSize < 4) blockSize = sizeof(int);
    if (blockCount < 4) blockCount = 4;

    // Allocate the object from standard memory.
    self = (rvMemBlock*) malloc(sizeof(rvMemBlock) + extra);

    // Initialize the memblock object.
    self->head = NULL;
    self->tail = NULL;
    self->used = 0;
    self->count = 0;
    self->reclaim = true;
    self->blockSize = blockSize;
    self->blockCount = blockCount;

    // Return the memblock object.
    return self;
}


void rvMemBlock_Free(rvMemBlock* self)
// This will free the entire memblock object.
{
    rvMemBlockGroup* group = self->head;

    // Loop over all memblock elements.
    while (group)
    {
        // Save the next group.
        rvMemBlockGroup* next = group->next;

        // Free the memblock group.
        free(group);

        // Set the next group.
        group = next;
    }

    // Free the memblock object.
    free(self);
}


void* rvMemBlock_Extra(rvMemBlock* self)
// Return a pointer to the extra information in the memblock object.
// The extra information can be used to store additional information
// associated with the data stored in the memblock allocator items.
{
    // Return a pointer to the extra information.
    return self + 1;
}


int rvMemBlock_Memory(rvMemBlock* self)
// Returns the total memory allocated in memblocks.  This does not
// include memory in unallocated memblock blocks in elements.
{
    // Return the used count multiplied by the object size.
    return self->used*  self->blockSize;
}


void* rvMemBlock_AllocBlockNoClear(rvMemBlock* self)
// Allocate a new object, but do not change or
// clear or otherwise touch the allocated memory.
{
    void* object = NULL;
    rvMemBlockGroup* group = NULL;

    // Determine if there are available items.
    if (self->used < self->count)
    {
        // Start at the head of the group list.
        group = self->head;

        // Normally, looping to find a memblock group with a free item would
        // be inefficient.  However, whenever an item is freed its group
        // is moved to the head of the list.  This insures that elements with
        // free items are searched first.
        while (group)
        {
            // Stop if this group has free items.
            if (group->used < group->count) break;

            // Set the next group.
            group = group->next;
        }
    }

    // Did we find a free item?
    if (!group)
    {
        int i;
        int item_size;
        int element_size;
        rvMemBlockItem* item;

        // No, so create a group of blocks.

        // Determine the size of a memblock item.
        item_size = sizeof(rvMemBlockItem) - sizeof(item->data) + self->blockSize;

        // Determine the size of an memblock group.
        element_size = sizeof(rvMemBlockGroup) - sizeof(group->items) + (item_size * self->blockCount);

        // Allocate and initialize a new memblock elements.
        group = (rvMemBlockGroup*) malloc(element_size);

        // Initialize the group.
        group->used = 0;
        group->count = self->blockCount;
        group->parent = self;

        // Update the count of items in the entire memblock allocator.
        self->count += self->blockCount;

        // The first free item is the first item on the list.
        group->free = &group->items[0];

        // Point to the first item.
        item = group->free;

        // Initialize each item.
        for (i = 0; i < (group->count - 1); ++i)
        {
            // Fill in the item information.
            item->info.next = (rvMemBlockItem*) (((char*) item) + item_size);

            // Move to the next item;
            item = item->info.next;
        }

        // Initialize the last item.
        item->info.next = NULL;

        // Is there a head object?
        if (!self->head)
        {
            // Place as the only object in the list.
            group->prev = NULL;
            group->next = NULL;

            // Link into the list.
            self->head = group;
            self->tail = group;
        }
        else
        {
            // Place as the first group in the list.
            group->prev = NULL;
            group->next = self->head;

            // Link into the list.
            self->head->prev = group;
            self->head = group;
        }
    }

    // We should have an item by now.
    if (group)
    {
        // Point the first free item.
        rvMemBlockItem* item = group->free;

        // Update the parent group.
        group->used += 1;
        group->free = item->info.next;

        // Update the parent memblock allocator.
        self->used += 1;

        // Remember the parent.
        item->info.parent = group;

        // Set the object data pointer.
        object = &item->data[0];

        // Are all elements in this item allocated?
        if (group->used == group->count)
        {
            // Move the memblock group to the end of the list to optimize
            // the search when allocating a new object.  This is a low
            // overhead operation that should speed things up.
            if (self->tail != group)
            {
                // Unlink the group from the free list.
                if (group->prev) group->prev->next = group->next;
                if (group->next) group->next->prev = group->prev;

                // Update the memblock object.
                if (self->head == group) self->head = group->next;
                if (self->tail == group) self->tail = group->prev;

                // Is there a head object?
                if (!self->tail)
                {
                    // Place as the only object in the list.
                    group->prev = NULL;
                    group->next = NULL;

                    // Link into the list.
                    self->head = group;
                    self->tail = group;
                }
                else
                {
                    // Place as the last group in the list.
                    group->prev = self->tail;
                    group->next = NULL;

                    // Link into the list.
                    self->tail->next = group;
                    self->tail = group;
                }
            }
        }
    }

#ifdef DEBUG_MEMBLOCKS
    rvMemBlock_Check(self);
#endif

    // Return the allocated object.
    return object;
}


void* rvMemBlock_AllocBlock(rvMemBlock* self)
// Allocate an object and set the allocated memory to 0.
{
    void* object;

    // Allocate an object.
    object = rvMemBlock_AllocBlockNoClear(self);

    // If an object was allocated, clear its memory.
    if (object) memset(object, 0, self->blockSize);

    // Return the allocated object.
    return object;
}


bool rvMemBlock_FreeBlock(rvMemBlock* self, void* block)
{
    // Determine the item from the object pointer.
    rvMemBlockItem* item = (rvMemBlockItem*) (((char*) block) - sizeof(rvMemBlockItem) + sizeof(item->data));

    // Determine the group from the item.
    rvMemBlockGroup* group = item->info.parent;

    // Validate that this is the parent of the object passed in.
    if (self != group->parent) return false;

    // The item is now added to the free list for the group.
    item->info.next = group->free;
    group->free = item;

    // One list item is used in the group.
    group->used -= 1;

    // Update the parent memblock allocator.
    self->used -= 1;

    // Should the group be removed from the memblock?
    if ((group->used == 0) && (self->reclaim))
    {
        // Unlink the group from the free list.
        if (group->prev) group->prev->next = group->next;
        if (group->next) group->next->prev = group->prev;

        // Update the memblock object.
        if (self->head == group) self->head = group->next;
        if (self->tail == group) self->tail = group->prev;

        // Update the count of items in the entire memblock allocator.
        self->count -= self->blockCount;

        // Free the group.
        free(group);
    }
    else
    {
        // Move the memblock to the head of the list to optimize
        // the search when allocating a new object.  This is a low
        // overhead operation that should speed things up.
        if (self->head != group)
        {
            // Unlink the group from the free list.
            if (group->prev) group->prev->next = group->next;
            if (group->next) group->next->prev = group->prev;

            // Update the memblock object.
            if (self->head == group) self->head = group->next;
            if (self->tail == group) self->tail = group->prev;

            // Is there a head object?
            if (!self->head)
            {
                // Place as the only object in the list.
                group->prev = NULL;
                group->next = NULL;

                // Link into the list.
                self->head = group;
                self->tail = group;
            }
            else
            {
                // Place as the first group in the list.
                group->prev = NULL;
                group->next = self->head;

                // Link into the list.
                self->head->prev = group;
                self->head = group;
            }
        }
    }

#ifdef DEBUG_MEMBLOCKS
    rvMemBlock_Check(self);
#endif

    return true;
}


void rvMemBlock_ReclaimMemory(rvMemBlock* self, bool reclaim)
// Set the reclaim flag in this object.  This flag controls whether
// memblock elements are returned to system memory once all items within
// the group have been freed.
{
    // Set the memory reclaim flag.
    self->reclaim = reclaim;
}


