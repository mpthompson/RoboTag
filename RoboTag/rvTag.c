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

    $Id: rvTag.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include "rvTag.h"

rvTag* rvTag_New(void)
// Allocate a new bit tag object.
{
    rvTag *self = NULL;
    rvDecode *decoder = NULL;

    // Allocate a new tag object and other objects.
    self = (rvTag*) malloc(sizeof(rvTag));
    decoder = rvDecode_New(RVTAG_SAMPLE_COUNT);

    // Did we allocate the object.
    if ((self != NULL) && (decoder != NULL))
    {
        // Set the object variables.
        self->result = false;
        self->decoder = decoder;
        self->id = 0;
        self->direction = rvDecodeUnknown;
        memset(self->corners, 0, sizeof(rvUint8) * RVTAG_CORNER_COUNT);
    }
    else
    {
        // Clean up.
        if (self) free(self);
        if (decoder) rvDecode_Free(decoder);

        return NULL;
    }

    return self;
}


void rvTag_Free(rvTag* self)
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the tag object.
        rvDecode_Free(self->decoder);
        free(self);
    }
}


bool rvTag_DecodeSamples(rvTag* self, CvPoint2D32f corners[RVTAG_CORNER_COUNT], rvUint8 samples[RVTAG_SAMPLE_COUNT])
{
    // Assume we fail.
    self->result = false;

    // Decode the bits and see if we found a valid pattern.
    self->result = rvDecode_SetBits(self->decoder, samples, RVTAG_SAMPLE_COUNT);

    // Did the decoding succeed?
    if (self->result)
    {
        // Get the id.
        rvDecode_GetId(self->decoder, &self->id);

        // Get the direction.
        rvDecode_GetDirection(self->decoder, &self->direction);

        // Orient the corners.
        switch (self->direction)
        {
        default:
            case rvDecodeNorth:

                // Orient the corners.
                self->corners[0] = corners[0];
                self->corners[1] = corners[1];
                self->corners[2] = corners[2];
                self->corners[3] = corners[3];
                break;

            case rvDecodeSouth:

                // Orient the corners.
                self->corners[0] = corners[2];
                self->corners[1] = corners[3];
                self->corners[2] = corners[0];
                self->corners[3] = corners[1];
                break;

            case rvDecodeEast:

                // Orient the corners.
                self->corners[0] = corners[1];
                self->corners[1] = corners[2];
                self->corners[2] = corners[3];
                self->corners[3] = corners[0];
                break;

            case rvDecodeWest:

                // Orient the corners.
                self->corners[0] = corners[3];
                self->corners[1] = corners[0];
                self->corners[2] = corners[1];
                self->corners[3] = corners[2];
                break;
        }
    }

    // Return the result.
    return self->result;
}


bool rvTag_GetDecodedResult(rvTag* self)
{
    // Return the result of previous decoding.
    return self->result;
}


bool rvTag_GetDecodedId(rvTag* self, rvUint16 *id)
{
    // Make sure we have a successful result.
    if (self->result)
    {
        // Return the id.
        *id = self->id;
    }

    // Return the result of previous decoding.
    return self->result;
}


bool rvTag_GetDecodedDirection(rvTag* self, rvInt16 *direction)
{
    // Make sure we have a successful result.
    if (self->result)
    {
        // Return the direction.
        *direction = self->direction;
    }

    // Return the result of previous decoding.
    return self->result;
}


bool rvTag_GetDecodedCorners(rvTag* self, CvPoint2D32f corners[RVTAG_CORNER_COUNT])
{
    // Make sure we have a successful result.
    if (self->result)
    {
        // Copy the corners.
        memcpy(corners, self->corners, sizeof(self->corners));
    }

    // Return the result of previous decoding.
    return self->result;
}

