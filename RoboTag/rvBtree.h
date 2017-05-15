/*
    Balanced Binary Tree Object

    Binary tree creation, traversal and freeing utilizing a user-supplied
    comparison routine.

    Code copied from:
    "Fundamentals of Data Structures"
    Ellis Horowitz and Sartaj Sahni
    Computer Science Press, Inc. 1976
    ISBN 0-914894-20'X
    pp. 454-455

    $Id: rvBtree.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_BTREE_H_
#define _RV_BTREE_H_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _rvBtree rvBtree;

// Binary tree object utility methods.
typedef int (*rvBtree_Compare)(const void *, const void *);
typedef void (*rvBtree_DeleteFunction)(const void *);

// Binary tree methods.
extern rvBtree *rvBtree_New(rvBtree_Compare compFunc, int objectCount, int objectSize, int extra);
extern void rvBtree_Free(rvBtree *self, rvBtree_DeleteFunction objfree);
extern void *rvBtree_Extra(rvBtree *self);
extern bool rvBtree_IsEmpty(rvBtree *self);
extern int rvBtree_Used(rvBtree *self);
extern size_t rvBtree_Memory(rvBtree *self);
extern void *rvBtree_Add(rvBtree *self, const void *object);
extern void *rvBtree_Find(rvBtree *self, const void *object);
extern void *rvBtree_Next(rvBtree *self, const void *object);
extern void *rvBtree_Prev(rvBtree *self, const void *object);
extern void *rvBtree_ElementUpdate(rvBtree *self, const void *object, rvBtree_DeleteFunction objfree);

#ifdef DEBUG
extern bool rvBtree_Validate(rvBtree *self, bool print_error);
#endif

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_BTREE_H_
