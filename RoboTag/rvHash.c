/*
    String-to-pointer hash functions. This was based on code
    from perl5 by Larry Wall and heavily modified.

    Copyright (c) 1991-1997, Larry Wall
    You may distribute under the terms of either the GNU General Public
    License or the Artistic License, as specified in the README file.

    $Id: rvHash.c 28 2010-03-09 23:49:39Z mike $
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rvMemBlock.h"
#include "rvHash.h"

// Percent at which the table is expanded. Don't make greater than 99.
#define RVHASH_FILLPCT 60

// Hash table object.
struct _rvHash
{
    rvMemBlock              *allocator;
    struct _rvHashEntry     **entries;
    int                     tableMax;
    int                     tableFill;
    bool                    caseSensitive;
    rvHash_DeleteFunction   deleteFunction;
    // Extra bytes go here.
};

// Hash table entry.
struct _rvHashEntry
{
    struct _rvHashEntry     *next;
    char                    *key;
    rvUint32                hash;
    void                    *value;
};

// Hash table iterator struct.
struct _rvHashIterator
{
    rvHash                  *parent;
    int                     index;
    rvHashEntry             *entry;
};

static rvUint32 rvHash_Hash(rvHash *self, const char *key)
// Calculate the hash value. Currently uses Knuth's function.
// If this proves inadequate, see http://burtleburtle.net/bob/hash/index.html
// (also in Dr. Dobbs Sep 97) for more hashing options.
{
    const rvUint8 *s = (const rvUint8 *) key;
    rvUint32 hash = 0;

    if (self->caseSensitive)
    {
        while (*s) hash = ((hash<<5) ^ (hash>>27)) ^ *s++;
    }
    else
    {
        while (*s) hash = ((hash<<5) ^ (hash>>27)) ^ tolower(*s++);
    }
    return hash;
}


static void rvHash_Expand(rvHash *self)
// Expand the table to accomodate more entries.
{
    int i;
    int oldsize = self->tableMax + 1;
    int newsize = oldsize * 2;
    rvHashEntry **a;
    rvHashEntry **b;
    rvHashEntry *entry;
    rvHashEntry **oentry;

    // Expand our hash table.
    a = (rvHashEntry**) realloc(self->entries, newsize * sizeof(rvHashEntry*));

    // Zero out the new portion of the structure.
    memset(&a[oldsize], 0, oldsize * sizeof(rvHashEntry*));

    // Update the table information.
    self->tableMax = --newsize;
    self->entries = a;

    // Loop thru the old entries in the first half of the table.
    for (i = 0; i < oldsize; i++, a++)
    {
        // Non-existent?
        if (!*a) continue;

        b = a+oldsize;
        for (oentry = a, entry = *a; entry; entry = *oentry)
        {
            // Does this old entry map to a new location?
            if ((entry->hash & newsize) != i)
            {
                // Remove this entry from the chain.
                *oentry = entry->next;

                // Insert it into the table at new location (which is
                // always at currentloc + oldsize because we doubled the
                // size of the table.
                entry->next = *b;
                if (!*b) self->tableFill++;
                *b = entry;
                continue;
            }
            else
            {
                oentry = &entry->next;
            }
        }

        // Everything moved?
        if (!*a) self->tableFill--;
    }
}


rvHash *rvHash_New(bool caseSensitive, int grow, rvHash_DeleteFunction deleteFunction, int extra)
// Allocate and initialize a new hash table object.
//
// caseSensitive        True for case-sensitive compares, false for case-insensitive.
// grow                 A new memory allocation is done for each "grow" entries that
//                      are added to the table. Pass 0 for default (8).
// deleteFunction       Callback func that frees a value. May be null.
// extra                Extra bytes to allocate with hash object.
{
    rvHash *self;
    rvMemBlock *allocator;

    // Sanity check the grow value.
    if (grow <= 0) grow = 8;

    // Our object will live within the extra memory block allocator space.
    allocator = (rvMemBlock *) rvMemBlock_New(grow, sizeof(rvHashEntry), (int) sizeof(rvHash) + extra);

    // Get our reserved memory from the memory block allocator.
    self = (rvHash*) rvMemBlock_Extra(allocator);

    // Initialize this object.
    memset(self, 0, sizeof(rvHash));
    self->tableFill = 0;
    self->tableMax = 7;
    self->allocator = allocator;
    self->caseSensitive = caseSensitive;
    self->deleteFunction = deleteFunction;

    // Initialize the first entry.
    self->entries = (rvHashEntry**) malloc(8 * sizeof(rvHashEntry*));
    memset(self->entries, 0, 8 * sizeof(rvHashEntry*));

    return self;
}


void rvHash_Free(rvHash *self)
// Free this object and all associated storage.
{
    int i;
    rvHashEntry *entry;

    // Call delete func on all entries & free the keys.
    for (i = 0; i <= self->tableMax; i++)
    {
        for (entry = self->entries[i]; entry; entry = entry->next)
        {
            // Call the delete function?
            if (self->deleteFunction) (*self->deleteFunction)(entry->value);

            // Free the key.
            free(entry->key);
        }
    }

    // Free the table.
    free(self->entries);

    // Free self & all entries.
    rvMemBlock_Free(self->allocator);
}


void *rvHash_Extra(rvHash *self)
// Return pointer to extra bytes allocated by rvHash_New.
{
    return self + 1;
}


void *rvHash_Get(rvHash *self, const char *key)
// Fetch the value associated with this key or NULL if no such entry.
{
    rvUint32 hash;
    rvHashEntry *entry;

    // Calculate the hash value of this key.
    hash = rvHash_Hash(self, key);

    // Find matching entry in the table.
    for (entry = self->entries[hash & self->tableMax]; entry; entry = entry->next)
    {
        // Quick test based on hash.
        if (entry->hash != hash)
            continue;

        // Is this it?
        if (self->caseSensitive)
        {
            if (strcmp(entry->key, key) != 0) continue;
        }
        else
        {
            if (_stricmp(entry->key, key) != 0) continue;
        }

        // Found it.
        return entry->value;
    }

    return NULL;
}


bool rvHash_Set(rvHash *self, const char *key, void *value)
// Store this key/value in the hash table. Replaces the existing
// key if it's already present. Returns true if existing key
// was present, false if not.
{
    int i;
    rvUint32 hash;
    rvHashEntry *entry;
    rvHashEntry **oentry;

    // Calculate the hash value of this key.
    hash = rvHash_Hash(self, key);

    // Look for an existing entry.
    oentry = &(self->entries[hash & self->tableMax]);
    entry = *oentry;
    i = 1;
    for (; entry; i=0, entry = entry->next)
    {
        // String pointers can't be equal.
        if (entry->hash != hash) continue;

        // Is this it?
        if (self->caseSensitive)
        {
            if (strcmp(entry->key, key) != 0) continue;
        }
        else
        {
            if (_stricmp(entry->key, key) != 0) continue;
        }

        // Call delete function if provided.
        if (self->deleteFunction) (*self->deleteFunction)(entry->value);

        // Set the new value.
        entry->value = value;

        return true;
    }

    // Allocate the entry.
    entry = (rvHashEntry*) rvMemBlock_AllocBlockNoClear(self->allocator);

    // Allocate and copy the key into a buffer.
    entry->key = (char *) malloc(strlen(key) + 1);
    strcpy(entry->key, key);

    // Fill in the rest of the hash information.
    entry->value = value;
    entry->hash = hash;
    entry->next = *oentry;
    *oentry = entry;

    // Initial entry?
    if (i)
    {
        self->tableFill++;
        if ((self->tableFill * 100 / (self->tableMax + 1)) > RVHASH_FILLPCT) rvHash_Expand(self);
    }

    return false;
}


bool rvHash_Delete(rvHash *self, const char *key)
// Delete an entry. Returns true if the entry was deleted and
// false if it wasn't in the table.
{
    int i;
    rvUint32 hash;
    rvHashEntry *entry;
    rvHashEntry **oentry;

    // Calculate the hash value of this key.
    hash = rvHash_Hash(self, key);

    // Look for an existing entry.
    oentry = &(self->entries[hash & self->tableMax]);
    entry = *oentry;
    i = 1;
    for (; entry; i = 0, oentry = &entry->next, entry = entry->next)
    {
        // Strings pointers can't be equal.
        if (entry->hash != hash) continue;

        // Is this it?
        if (self->caseSensitive)
        {
            if (strcmp(entry->key, key) != 0) continue;
        }
        else
        {
            if (_stricmp(entry->key, key) != 0) continue;
        }

        // Call delete function if provided.
        if (self->deleteFunction) (*self->deleteFunction)(entry->value);

        // Free the key.
        free(entry->key);

        *oentry = entry->next;
        rvMemBlock_FreeBlock(self->allocator, entry);

        if (i) self->tableFill--;

        return true;
    }

    return false;
}


const char *rvHashIterator_First(rvHashIterator *iter, rvHash *parent)
// Initializes this iterator for this hash table and returns
// the first key or NULL if the table is empty.
{
    iter->parent = parent;
    iter->index = -1;
    iter->entry = NULL;

    // Return first key.
    return rvHashIterator_Next(iter);
}


const char *rvHashIterator_Next(rvHashIterator *iter)
// Return the next key in the table or NULL if done.
// Do not delete the entry being enumerated. If entries
// are added during iteration some entries may be skipped
// or enumerated twice.
{
    rvHashEntry *entry;

    // Point to the first entry.
    entry = iter->entry;

    // Loop over each entry.
    do
    {
        // Advance to next entry in chain.
        if (entry) entry = entry->next;

        // Advance to next entry in table.
        if (!entry)
        {
            iter->index++;
            if (iter->index > iter->parent->tableMax)
            {
                break;
            }
            entry = iter->parent->entries[iter->index];
        }
    } while (!entry);

    iter->entry = entry;

    // Return NULL if we reached the last entry.
    return (entry ? entry->key : NULL);
}


void *rvHashIterator_Value(rvHashIterator *iter)
// Returns the value associated with the current entry.
{
    return (iter->entry ? iter->entry->value : NULL);
}


#ifdef DEBUG
void
rvHash_Show(rvHash *self)
{
    int i;
    int n;
    int total = 0;
    int longest = 0;
    int shortest = 9999;

    printf("Hash table @%08x:\n", self);
    printf("  Size: %5d Entries: %4d (%2d%%)\n", self->tableMax+1, self->tableFill, self->tableFill * 100 / (self->tableMax + 1));

    for (i = 0; i <= self->tableMax; ++i)
    {
        rvHashEntry *entry;

        for (entry = self->entries[i], n = 0; entry; entry = entry->next) n++;

        if (n > 0)
        {
            if (longest < n) longest = n;
            if (shortest > n) shortest = n;
            total += n;
            printf("  [%4d] %d entries\n", i, n);
        }
    }

    printf("  Longest chain: %d Shortest: %d Average: %f\n", longest, shortest, (float) total / self->tableFill);

}
#endif
