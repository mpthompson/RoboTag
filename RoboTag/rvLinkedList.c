/*
    Linked list creation, traversal and freeing methods.

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

    $Id: rvLinkedList.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include <string.h>
#include "rvMemBlock.h"
#include "rvLinkedList.h"

typedef struct _rvLinkedListElement rvLinkedListElement;

struct _rvLinkedListElement
{
    rvLinkedListElement* next;
    rvLinkedListElement* prev;
    char data[1];
};

struct _rvLinkedList
{
    int objectSize;
    int objectCount;
    int dataOffset;
    rvLinkedListElement* head;
    rvLinkedListElement* tail;
    rvMemBlock* blocks;
    int extra[1];
};


rvLinkedList* rvLinkedList_New(int objectCount, int objectSize, int extra)
// This function returns a link list object.
{
    rvLinkedList* self;
    rvMemBlock* blocks;
    int elementSize;
    int dataOffset;

    // Sanity check the arguments.
    if (objectSize < sizeof(int)) objectSize = sizeof(int);
    if (objectCount < 4) objectCount = 4;

    // Determine the data offset within the element.
    dataOffset = (int) (sizeof(rvLinkedListElement*) + sizeof(rvLinkedListElement*));

    // Determine the size of each element including the extra space.
    elementSize = dataOffset + objectSize;

    // Allocate a memory block allocator object.
    blocks = rvMemBlock_New(objectCount, elementSize, sizeof(rvLinkedList) + extra);

    // The extra information in the memblocks contains the linked list object.
    self = (rvLinkedList*) rvMemBlock_Extra(blocks);

    // Initialize the linked list object.
    self->head = NULL;
    self->tail = NULL;
    self->blocks = blocks;
    self->dataOffset = dataOffset;
    self->objectSize = objectSize;
    self->objectCount = objectCount;

    // Return the linked list object.
    return self;
}


static void rvLinkedList_RemoveAll(rvLinkedList* self, rvLinkedList_ObjectFreeFunc objfree, bool freeChunks)
// Empty out this list, and free memory for each item, but don't free the
// actual memory block allocator (so it can be reused).
{
    rvLinkedListElement* element = self->head;

    // Loop over all list elements.
    while (element)
    {
        // Save the next element.
        rvLinkedListElement* next = element->next;

        // Free the object.
        if (objfree) objfree(element->data);

        // Free the element itself.
        if (freeChunks) rvMemBlock_FreeBlock(self->blocks, element);

        // Set the next element.
        element = next;
    }

    self->head = NULL;
    self->tail = NULL;
}


void rvLinkedList_Clear(rvLinkedList* self, rvLinkedList_ObjectFreeFunc objfree)
// Empty out this list, and free memory for each item, but don't free the
// actual memory block allocator (so it can be reused).
{
    // Remove each element and free its memory in the memory block allocator.
    rvLinkedList_RemoveAll(self, objfree, true);
}


void rvLinkedList_Free(rvLinkedList* self, rvLinkedList_ObjectFreeFunc objfree)
// This will free the memory allocated for the list.
{
    // Empty out the list, but don't worry about freeing memory block allocator.
    rvLinkedList_RemoveAll(self, objfree, false);

    // Free the memory block allocator.  This also frees this object.
    rvMemBlock_Free(self->blocks);
}


void* rvLinkedList_Extra(rvLinkedList* self)
// Return a pointer to the extra information in the linked list object.
// The extra information can be used to store additional information
// associated with the data stored in the linked list objects.
{
    // Return a pointer to the extra information.
    return (void*) self->extra;
}


void* rvLinkedList_Append (rvLinkedList* self, const void* object)
// Append a new object to the list.
{
    rvLinkedListElement* element = NULL;

    // Allocate a list element.
    element = (rvLinkedListElement*) rvMemBlock_AllocBlockNoClear(self->blocks);

    // Is there a head object?
    if (!self->head)
    {
        // Place as the only object in the list.
        element->prev = 0;
        element->next = 0;

        // Copy the object data into the element.
        memcpy(element->data, object, self->objectSize);

        // Link into the list.
        self->head = element;
        self->tail = element;
    }
    else
    {
        // Place as the last object in the list.
        element->prev = self->tail;
        element->next = 0;

        // Copy the object data into the element.
        memcpy(element->data, object, self->objectSize);

        // Link into the list.
        self->tail->next = element;
        self->tail = element;
    }

    // Return a pointer to the allocated data.
    return element ? element->data : NULL;
}


void* rvLinkedList_Insert(rvLinkedList* self, const void* object)
// Insert a new object to the front of the list.
{
    rvLinkedListElement* element = NULL;

    // Allocate a list element.
    element = (rvLinkedListElement*) rvMemBlock_AllocBlockNoClear(self->blocks);

    // Is there a head object?
    if (!self->head)
    {
        // Place as the only object in the list.
        element->prev = 0;
        element->next = 0;

        // Copy the object data into the element.
        memcpy(element->data, object, self->objectSize);

        // Link into the list.
        self->head = element;
        self->tail = element;
    }
    else
    {
        // Place as the first object in the list.
        element->prev = 0;
        element->next = self->head;

        // Copy the object data into the element.
        memcpy(element->data, object, self->objectSize);

        // Link into the list.
        self->head->prev = element;
        self->head = element;
    }

    // Return a pointer to the allocated data.
    return element ? element->data : NULL;
}


void* rvLinkedList_InsertBefore(rvLinkedList* self, const void* newobject, const void* atobject)
// Insert a new object in front of the specified object. If newobject is NULL
// inserts at the head of the list.
{
    void* result = NULL;

    // Initialize the marker element pointer.
    rvLinkedListElement* atelement = atobject ? (rvLinkedListElement*) (((char*) atobject) - self->dataOffset) : NULL;

    // Insert at head of list if marker is null, if list is empty,
    // or if marker is the head.
    if (atobject == NULL || self->head == NULL || atelement->prev == NULL)
    {
        result = rvLinkedList_Insert(self, newobject);
    }
    else
    {
        // Allocate a list element.
        rvLinkedListElement* newelement = (rvLinkedListElement*) rvMemBlock_AllocBlockNoClear(self->blocks);

        // Link into list in front of the marker.
        newelement->next = atelement;
        newelement->prev = atelement->prev;
        atelement->prev->next = newelement;
        atelement->prev = newelement;

        // Copy the object data into the element.
        memcpy(newelement->data, newobject, self->objectSize);

        // Return a pointer to the allocated data.
        result = newelement->data;
    }

    return result;
}


void* rvLinkedList_InsertSorted(rvLinkedList* self, const void* object, rvLinkedList_ObjectCompareFunc objcompare, void* param)
// Insert a new object into the list at the correct sort position based on
// objcompare. The list must already be in sort order for this to work properly.
// param is passed to the compare function.
{
    void* result = NULL;

    // Find insert position.
    rvLinkedListElement* at = self->head;
    while (at && objcompare(object, at->data, param) > 0)
        at = at->next;

    // Insert before insertion point or at end of list.
    if (at)
        result = rvLinkedList_InsertBefore(self, object, at->data);
    else
        result = rvLinkedList_Append(self, object);

    return result;
}


void* rvLinkedList_Next(rvLinkedList* self, const void* object)
// Returns the next element in the list or the first element
// if the element passed in is null.
{
    // Initialize the element from the object information.
    rvLinkedListElement* element = object ? (rvLinkedListElement*) (((char*) object) - self->dataOffset) : NULL;

    // Was an element passed in?
    if (!element)
    {
        // Return the head of the list.
        element = self->head;
    }
    else
    {
        // Return the next element.
        element = element->next;
    }

    // Return a pointer to the element data.
    return element ? element->data : NULL;
}


void* rvLinkedList_Prev(rvLinkedList* self, const void* object)
// Returns the previous element in the list or the last element
// if the element passed in is null.
{
    // Initialize the element from the object information.
    rvLinkedListElement* element = object ? (rvLinkedListElement*) (((char*) object) - self->dataOffset) : NULL;

    // Was an element passed in?
    if (!element)
    {
        // Return the tail of the list.
        element = self->tail;
    }
    else
    {
        // Return the previous element.
        element = element->prev;
    }

    // Return a pointer to the element data.
    return element ? element->data : NULL;
}


void rvLinkedList_Delete(rvLinkedList* self, const void* object, rvLinkedList_ObjectFreeFunc objfree)
// Remove the indicated element from the list.
{
    // Initialize the element from the object information.
    rvLinkedListElement* element = object ? (rvLinkedListElement*) (((char*) object) - self->dataOffset) : NULL;

    // Make sure we have an element.
    if (element)
    {
        // Unlink the element from the chain of linked elements.
        if (element->prev) element->prev->next = element->next;
        if (element->next) element->next->prev = element->prev;
        if (self->head == element) self->head = element->next;
        if (self->tail == element) self->tail = element->prev;

        // Free the object.
        if (objfree) objfree(element->data);

        // Free the element itself.
        rvMemBlock_FreeBlock(self->blocks, element);
    }
}


void rvLinkedList_Sort(rvLinkedList* self, rvLinkedList_ObjectCompareFunc objcompare, void* param)
// Perform a quick and dirty sort, in place, on the linked list.
// param is passed through to the compare function.
{
    rvLinkedListElement* head = self->head;         // Head of the list to sort.

    // head gets moved along as the head of the list remaining to be sorted.
    while (head != NULL)
    {
        rvLinkedListElement* next = head->next;     // Next element.
        rvLinkedListElement* smallest = head;           // Assume.

        // Search for the smallest element remaining in the list.
        while (next != NULL)
        {
            if (objcompare(next->data, smallest->data, param) < 0)
            {
                // A new potential smallest item.
                smallest = next;
            }

            // Get the next item in the remaining list.
            next = next->next;
        }

        // Smallest points to the smallest item in the remaining list.
        // We want smallest to be moved from its present position to the
        // head of the list we're currently sorting.

        // Is smallest already at the head?
        if (smallest != head)
        {
            // Unlink smallest from its current position.

            // If smallest->next is NULL, it was the tail.
            if (smallest->next == NULL)
            {
                 // Set the new tail.
                self->tail = smallest->prev;

                // Make sure the tail's next is nowhere.
                self->tail->next = NULL;
            }
            else
            {
                // Set the previous' next item to our next item.
                smallest->prev->next = smallest->next;

                // Set the next item's previous to our previous.
                smallest->next->prev = smallest->prev;
            }

            // Make smallest the head of the sublist.

            // Is the head of the list we're sorting the entire list's head?
            if (head == self->head)
            {
                // Set smallest as the head of the entire list.
                self->head = smallest;
            }
            else
            {
                // Make sure someone's next pointer is pointing at us.
                head->prev->next = smallest;
            }

            // Set our next to the previous head.
            smallest->next = head;

            // Set our previous.
            smallest->prev = head->prev;

            // Make sure someone's previous pointer is pointing at us.
            head->prev = smallest;
        }
        // The head of the list we now have to sort is next element.
        head = smallest->next;
    }
}


int rvLinkedList_GetCount(rvLinkedList* self)
// Return the number of objects in the list.
{
    int i;
    rvLinkedListElement* next;

    // Get the first element.
    next = (rvLinkedListElement*) rvLinkedList_Next(self, NULL);

    // Slow and dirty.
    for (i = 0; next != NULL; i++, next = (rvLinkedListElement*) rvLinkedList_Next(self, next));

    // i holds the number of elements in the list.
    return i;
}


int rvLinkedList_GetObjectSize(rvLinkedList* self)
// Return the object size of the elements of the list.
{
    return self->objectSize;
}
