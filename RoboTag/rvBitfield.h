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

    $Id: rvBitfield.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_BITFIELD_INCLUDED_
#define _RV_BITFIELD_INCLUDED_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Bitfield types.
typedef struct _rvBitfield rvBitfield;

// Bitfield structures.
struct _rvBitfield
{
    rvUint8 byteCount;
    rvUint8 bytes[1];
};

// Bitfield methods.
rvBitfield* rvBitfield_New(rvUint8 bitCount);
void rvBitfield_Free(rvBitfield *self);
void rvBitfield_SetBit(rvBitfield *self, rvUint8 pos, rvUint8 value);
rvUint8 rvBitfield_GetBit(rvBitfield *self, rvUint8 pos);
rvUint8 rvBitfield_GetBitCount(rvBitfield *self);
void rvBitfield_SetBits(rvBitfield *self, rvUint8 count, rvUint8 *values, rvUint8 *mapping);
void rvBitfield_GetBits(rvBitfield *self, rvUint8 count, rvUint8 *values, rvUint8 *mapping);
void rvBitfield_SetByte(rvBitfield *self, rvUint8 pos, rvUint8 value);
rvUint8 rvBitfield_GetByte(rvBitfield *self, rvUint8 pos);
rvUint8 rvBitfield_GetByteCount(rvBitfield *self);
void rvBitfield_SetBytes(rvBitfield *self, rvUint8 count, rvUint8 *bytes);
void rvBitfield_GetBytes(rvBitfield *self, rvUint8 count, rvUint8 *bytes);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_BITFIELD_INCLUDED_

