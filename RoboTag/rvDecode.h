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

    $Id: rvDecode.h 30 2010-03-11 17:57:43Z mike $
*/

#ifndef _RV_DECODE_INCLUDED_
#define _RV_DECODE_INCLUDED_

#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "rvFec.h"
#include "rvBitfield.h"

// Global mappings.
extern rvUint8 rvDecodeMappingNorth[];
extern rvUint8 rvDecodeMappingEast[];
extern rvUint8 rvDecodeMappingSouth[];
extern rvUint8 rvDecodeMappingWest[];

// Decode enums.
enum
{
    rvDecodeUnknown = -1,
    rvDecodeNorth = 0,
    rvDecodeSouth = 1,
    rvDecodeEast = 2,
    rvDecodeWest = 3
};

// Decode types.
typedef struct _rvDecode rvDecode;

// Decode structures.
struct _rvDecode
{
    bool result;
    rvUint8 bitcount;
    rvInt16 gridDir;
    rvUint16 gridId;
    rvFec* gridFec;
    rvBitfield* gridBits;
};

// Decode methods.
rvDecode* rvDecode_New(rvUint8 bitcount);
void rvDecode_Free(rvDecode *self);
bool rvDecode_SetBits(rvDecode *self, rvUint8 *values, rvUint8 count);
bool rvDecode_GetId(rvDecode *self, rvUint16 *id);
bool rvDecode_GetDirection(rvDecode *self, rvInt16 *direction);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_DECODE_INCLUDED_

