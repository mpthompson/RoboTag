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

    $Id: rvTag.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_TAG_INCLUDED_
#define _RV_TAG_INCLUDED_

#include "rvTypes.h"
#include "rvDecode.h"
#include "cv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RVTAG_CORNER_COUNT  4
#define RVTAG_SAMPLE_COUNT  64

// Tag types.
typedef struct _rvTag rvTag;

// Tag structures.
struct _rvTag
{
    bool result;
    rvDecode *decoder;
    rvUint16 id;
    rvInt16 direction;
    rvUint8 values[RVTAG_SAMPLE_COUNT];
    CvPoint2D32f corners[RVTAG_CORNER_COUNT];
};

// Tag methods.
rvTag* rvTag_New(void);
void rvTag_Free(rvTag* self);
bool rvTag_DecodeSamples(rvTag* self, CvPoint2D32f corners[RVTAG_CORNER_COUNT], rvUint8 samples[RVTAG_SAMPLE_COUNT]);
bool rvTag_GetDecodedResult(rvTag* self);
bool rvTag_GetDecodedId(rvTag* self, rvUint16 *id);
bool rvTag_GetDecodedDirection(rvTag* self, rvInt16 *direction);
bool rvTag_GetDecodedCorners(rvTag* self, CvPoint2D32f corners[RVTAG_CORNER_COUNT]);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_TAG_INCLUDED_
