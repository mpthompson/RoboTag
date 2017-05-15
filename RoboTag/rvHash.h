/*
    Hash table object that manages a table of pointers indexed on unique
    string based keys.  This was based on code from perl5 by Larry Wall
    and heavily modified.

    Copyright (c) 1991-1997, Larry Wall
    You may distribute under the terms of either the GNU General Public
    License or the Artistic License, as specified in the README file.

    $Id: rvHash.h 28 2010-03-09 23:49:39Z mike $
 */

#ifndef _RV_HASH_H_
#define _RV_HASH_H_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Hash table types.
typedef struct _rvHash rvHash;
typedef struct _rvHashEntry rvHashEntry;
typedef struct _rvHashIterator rvHashIterator;

// Callback type to delete hashed values.
typedef void (*rvHash_DeleteFunction)(void *value);

// Hash table methods.
extern rvHash *rvHash_New(bool caseSensitive, int grow, rvHash_DeleteFunction deleteFunction, int extra);
extern void rvHash_Free(rvHash *self);
extern void *rvHash_Extra(rvHash *self);
extern void* rvHash_Get(rvHash *self, const char *key);
extern bool rvHash_Set(rvHash *self, const char *key, void *value);
extern bool rvHash_Delete(rvHash *self, const char *key);

// Hash table iteration.
extern const char *rvHashIterator_First(rvHashIterator *iter, rvHash *tb);
extern const char *rvHashIterator_Next(rvHashIterator *iter);
extern void *rvHashIterator_Value(rvHashIterator *iter);

// Utility and debug functions.
#ifdef DEBUG
extern void rvHash_Show(rvHash *self);
#endif

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_HASH_H_
