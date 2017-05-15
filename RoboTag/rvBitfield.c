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

    $Id: rvBitfield.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include "rvBitfield.h"

// Implements a bitfield object where the individual bits can
// be manipulated using SetBit() and GetBit() functions.  Some
// functions accept an optional mapping array can be used to
// implement a logical bit order that differs from the underlying
// physical bit order.

// Normal Mapping
rvUint8 bitfield_mapping_normal[] =
{
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

rvBitfield* rvBitfield_New(rvUint8 bitCount)
// Allocate a new bit field object of the specified bit count.
{
    rvUint8 i;
    rvUint8 byteCount;
    rvBitfield* self = NULL;

    // Bit count must be a multiple of bits in a byte.
    if ((bitCount == 0) || ((bitCount % 8) != 0)) return NULL;

    // Determine the byte count.
    byteCount = bitCount / 8;

    // Allocate a new rvBitfield object;
    self = (rvBitfield *) malloc(sizeof(rvBitfield) + byteCount);

    // Did we allocate the object.
    if (self != NULL)
    {
        // Save the byte count.
        self->byteCount = byteCount;

        // Initialize each byte.
        for (i = 0; i < byteCount; ++i) self->bytes[i] = 0;
    }

    return self;
}


void rvBitfield_Free(rvBitfield *self)
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the rvBitfield object.
        free(self);
    }
}


void rvBitfield_SetBit(rvBitfield *self, rvUint8 index, rvUint8 value)
// Set the specified bit in the rvBitfield.
{
    rvUint8 bitPos = index % 8;
    rvUint8 bytePos = index / 8;

    // Set the indicated bit in the bytes.
    self->bytes[bytePos] = (self->bytes[bytePos] & ~(0x01 << bitPos)) | ((value & 0x01) << bitPos);
}


rvUint8 rvBitfield_GetBit(rvBitfield *self, rvUint8 index)
// Get the specified bit in the rvBitfield.
{
    rvUint8 bitPos = index % 8;
    rvUint8 bytePos = index / 8;

    return (self->bytes[bytePos] >> bitPos) & 0x01;
}


rvUint8 rvBitfield_GetBitCount(rvBitfield *self)
// Get the number of bits in the rvBitfield.
{
    return self->byteCount * 8;
}


void rvBitfield_SetBits(rvBitfield *self, rvUint8 count, rvUint8 *values, rvUint8 *mapping)
// Set the rvBitfield values from an array of bits.  The mapping array is used
// to map bits to specific index locations.
{
    int i;

    // Test use of the default mapping.
    if (mapping == NULL) mapping = bitfield_mapping_normal;

    // Sanity check the count value.
    if (count > (self->byteCount << 3)) return;

    // Zero all the bits.
    for (i = 0; i < self->byteCount; ++i) self->bytes[0] = 0;

    // Loop over each bit value.
    for (i = 0; i < count; ++i)
    {
        // Set the bit value using the mapping array to determine the index.
        rvBitfield_SetBit(self, mapping[i], values[i]);
    }
}


void rvBitfield_GetBits(rvBitfield *self, rvUint8 count, rvUint8 *values, rvUint8 *mapping)
// Get the rvBitfield values into an array of bits.  The mapping array is used
// to map bits to specific index locations.
{
    int i;

    // Test use of the default mapping.
    if (mapping == NULL) mapping = bitfield_mapping_normal;

    // Sanity check the count value.
    if (count > (self->byteCount << 3)) return;

    // Loop over each bit value.
    for (i = 0; i < count; ++i)
    {
        // Set the bit value using the mapping array to determine the index.
        values[i] = rvBitfield_GetBit(self, mapping[i]);
    }
}


void rvBitfield_SetByte(rvBitfield *self, rvUint8 index, rvUint8 value)
// Set the specified byte in the rvBitfield.
{
    // Make sure the position isn't too large.
    if (index >= self->byteCount) return;

    // Set the byte value at the indicated position.
    self->bytes[index] = value;
}


rvUint8 rvBitfield_GetByte(rvBitfield *self, rvUint8 index)
// Get the specified byte in the rvBitfield.
{
    // Make sure the position isn't too large.
    if (index >= self->byteCount) return 0;

    // Get the byte value at the indicated position.
    return self->bytes[index];
}


rvUint8 rvBitfield_GetByteCount(rvBitfield *self)
// Get the number of bytes in the rvBitfield.
{
    return self->byteCount;
}


void rvBitfield_SetBytes(rvBitfield *self, rvUint8 count, rvUint8 *bytes)
// Set the rvBitfield values from an array of bytes.
{
    int i;

    // Sanity check the count value.
    if (count > self->byteCount) return;

    // Zero all the bits.
    for (i = 0; i < self->byteCount; ++i) self->bytes[0] = 0;

    // Loop over each byte value.
    for (i = 0; i < count; ++i)
    {
        // Set the byte.
        rvBitfield_SetByte(self, i, bytes[i]);
    }
}


void rvBitfield_GetBytes(rvBitfield *self, rvUint8 count, rvUint8 *bytes)
// Get the rvBitfield values as an array of bytes.
{
    int i;

    // Sanity check the count value.
    if (count > self->byteCount) return;

    // Loop over each byte value.
    for (i = 0; i < count; ++i)
    {
        // Get the byte.
        bytes[i] = rvBitfield_GetByte(self, i);
    }
}


