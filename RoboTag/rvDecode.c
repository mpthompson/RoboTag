/*
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

    $Id: rvDecode.c 30 2010-03-11 17:57:43Z mike $
*/

#include <stdlib.h>
#include "rvDecode.h"
#include "rvCrc16.h"

// Used to convert from  integer to direction.
static rvInt16 directions[4] =
{
    rvDecodeNorth,
    rvDecodeWest,
    rvDecodeSouth,
    rvDecodeEast
};

// North Mapping
rvUint8 rvDecodeMappingNorth[] =
{
     0,  1,  2,  3, 16, 17, 18, 19,
     4,  5,  6,  7, 20, 21, 22, 23,
    32, 33, 34, 35, 48, 49, 50, 51,
    36, 37, 38, 39, 52, 53, 54, 55,
    40, 41, 42, 43, 56, 57, 58, 59,
    44, 45, 46, 47, 60, 61, 62, 63,
    24, 25, 26, 27,  8,  9, 10, 11,
    28, 29, 30, 31, 12, 13, 14, 15
};

// East Mapping
rvUint8 rvDecodeMappingEast[] =
{
    28, 24, 44, 40, 36, 32,  4,  0,
    29, 25, 45, 41, 37, 33,  5,  1,
    30, 26, 46, 42, 38, 34,  6,  2,
    31, 27, 47, 43, 39, 35,  7,  3,
    12,  8, 60, 56, 52, 48, 20, 16,
    13,  9, 61, 57, 53, 49, 21, 17,
    14, 10, 62, 58, 54, 50, 22, 18,
    15, 11, 63, 59, 55, 51, 23, 19
};

// South Mapping
rvUint8 rvDecodeMappingSouth[] =
{
    15, 14, 13, 12, 31, 30, 29, 28,
    11, 10,  9,  8, 27, 26, 25, 24,
    63, 62, 61, 60, 47, 46, 45, 44,
    59, 58, 57, 56, 43, 42, 41, 40,
    55, 54, 53, 52, 39, 38, 37, 36,
    51, 50, 49, 48, 35, 34, 33, 32,
    23, 22, 21, 20,  7,  6,  5,  4,
    19, 18, 17, 16,  3,  2,  1,  0
};

// West Mapping
rvUint8 rvDecodeMappingWest[] =
{
    19, 23, 51, 55, 59, 63, 11, 15,
    18, 22, 50, 54, 58, 62, 10, 14,
    17, 21, 49, 53, 57, 61,  9, 13,
    16, 20, 48, 52, 56, 60,  8, 12,
     3,  7, 35, 39, 43, 47, 27, 31,
     2,  6, 34, 38, 42, 46, 26, 30,
     1,  5, 33, 37, 41, 45, 25, 29,
     0,  4, 32, 36, 40, 44, 24, 28
};

// Used to convert from  integer to mapping.
static rvUint8 *mappings[4] =
{
    rvDecodeMappingNorth,
    rvDecodeMappingWest,
    rvDecodeMappingSouth,
    rvDecodeMappingEast
};

rvDecode *rvDecode_New(rvUint8 bitcount)
// Allocate a new bit rvDecode object with the specified bit count.
{
    rvDecode* self = NULL;
    rvBitfield* gridBits = NULL;
    rvFec* gridFec = NULL;

    // Bit count must be a multiple of bits in a byte.
    if ((bitcount == 0) || ((bitcount % 8) != 0)) return NULL;

    // Allocate a new rvDecode object and other objects.
    self = (rvDecode *) malloc(sizeof(rvDecode));
    gridFec = rvFec_New(8, 4, 4);
    gridBits = rvBitfield_New(bitcount);

    // Did we allocate the object.
    if ((self != NULL) && (gridFec != NULL) && (gridBits != NULL))
    {
        // Set the object variables.
        self->result = false;
        self->bitcount = bitcount;
        self->gridFec = gridFec;
        self->gridBits = gridBits;
        self->gridId = 0;
        self->gridDir = rvDecodeUnknown;
    }
    else
    {
        // Clean up.
        if (self) free(self);
        if (gridFec) rvFec_Free(gridFec);
        if (gridBits) rvBitfield_Free(gridBits);

        return NULL;
    }

    return self;
}


void rvDecode_Free(rvDecode *self)
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the rvDecode object.
        rvFec_Free(self->gridFec);
        rvBitfield_Free(self->gridBits);
        free(self);
    }
}


bool rvDecode_SetBits(rvDecode *self, rvUint8 *values, rvUint8 count)
{
    rvUint16 i;
    rvUint16 dirCount;
    rvUint8 gridCrc[2];
    rvUint8 gridBytes[8];

    // Assume decode fails.
    self->result = false;

    // Reset the grid id and grid direction.
    self->gridId = 0;
    self->gridDir = rvDecodeUnknown;

    // Reset the direction count.
    dirCount = 0;

    // Loop over each direction.
    for (i = 0; i < 4; ++i)
    {
        // Set the bits in the indexed direction.
        rvBitfield_SetBits(self->gridBits, count, values, mappings[i]);

        // Get the bytes from the rvBitfield.
        rvBitfield_GetBytes(self->gridBits, sizeof(gridBytes), gridBytes);

        // Can we error correct the grid bytes?
        if (rvFec_Correct(self->gridFec, gridBytes))
        {
            // Yes.  Obtain the crc bytes for validation.
            rvCrc16_CCITT(gridBytes, 2, gridCrc);

            // Validate the CRC bytes.
            if ((gridBytes[2] == gridCrc[0]) && (gridBytes[3] == gridCrc[1]))
            {
                // Success. Save the grid id and grid direction.
                self->gridId = (((rvUint16) gridBytes[0]) << 8) | gridBytes[1];
                self->gridDir = directions[i];

                // XOR the grid id.
                self->gridId ^= 0xa5a5;

                // Save the count of valid directions.
                ++dirCount;
            }
        }
    }

    // We should never find more than one direction.
    if (dirCount == 1)
    {
        // Success.  Set the return value.
        self->result = true;
    }
    else
    {
        // Reset the grid id and grid direction.
        self->gridId = 0;
        self->gridDir = rvDecodeUnknown;
    }

    return self->result;
}


bool rvDecode_GetId(rvDecode *self, rvUint16 *id)
{
    // Did last rvDecode work?
    if (self->result)
    {
        // Return the id.
        *id = self->gridId;
    }

    return self->result;
}


bool rvDecode_GetDirection(rvDecode *self, rvInt16 *direction)
{
    // Did last rvDecode work?
    if (self->result)
    {
        // Return the direction.
        *direction = self->gridDir;
    }

    return self->result;
}
