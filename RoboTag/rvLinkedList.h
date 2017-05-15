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

    $Id: rvLinkedList.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_LINKEDLIST_H_
#define _RV_LINKEDLIST_H_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _rvLinkedList rvLinkedList;

// Linked list object utility method types.
typedef void (*rvLinkedList_ObjectFreeFunc)(const void*);
typedef int (*rvLinkedList_ObjectCompareFunc)(const void*, const void*, void* param);

// Linked list methods.
rvLinkedList* rvLinkedList_New(int objectCount, int objectSize, int extra);
void rvLinkedList_Free(rvLinkedList* list, rvLinkedList_ObjectFreeFunc objfree);
void rvLinkedList_Clear(rvLinkedList* self, rvLinkedList_ObjectFreeFunc objfree);
void* rvLinkedList_Extra(rvLinkedList* self);
void* rvLinkedList_Append(rvLinkedList* list, const void* object);
void* rvLinkedList_Insert(rvLinkedList* list, const void* object);
void* rvLinkedList_InsertBefore(rvLinkedList* self, const void* newObject, const void* atObject);
void* rvLinkedList_InsertSorted(rvLinkedList* self, const void* object, rvLinkedList_ObjectCompareFunc objcompare, void* param);
void* rvLinkedList_Next(rvLinkedList* list, const void* object);
void* rvLinkedList_Prev(rvLinkedList* list, const void* object);
void rvLinkedList_Delete(rvLinkedList* self, const void* object, rvLinkedList_ObjectFreeFunc objfree);
void rvLinkedList_Sort(rvLinkedList* self, rvLinkedList_ObjectCompareFunc objcompare, void* param);
int rvLinkedList_GetCount(rvLinkedList* self);
int rvLinkedList_GetObjectSize(rvLinkedList* self);

// In-line helper methods.
static __inline void* rvLinkedList_First(rvLinkedList* self)
{
    // Get the first object.
    return rvLinkedList_Next(self, NULL);
}

static __inline void* rvLinkedList_Last(rvLinkedList* self)
{
    // Get the last object.
    return rvLinkedList_Prev(self, NULL);
}

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_LINKEDLIST_H_
