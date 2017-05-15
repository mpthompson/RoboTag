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

    $Id: rvTags384.c 28 2010-03-09 23:49:39Z mike $
*/

#include "rvTags384.h"

// Functions for getting information related to a 16 x 24 grid of tags.

bool rvTags384_IsNavTag(rvUint16 id)
// Returns true if the tag is a valid grid tag.
{
    // Return true if the id is valid.
    if (id < 400) return true;
    if ((id > 1023) && (id < 1064)) return true;

    // Not a valid navigation id.
    return false;
}

bool rvTags384_IsObjectTag(rvUint16 id)
// Returns true if the tag is a valid object tag.
{
    // Return true if the id is valid.
    if ((id >= 2048) && (id < 4096)) return true;

    // Not a valid id.
    return false;
}

bool rvTags384_IsCharTag(rvUint16 id)
// Returns true if the tag is a valid character tag.
{
    // Return true if the id is valid.
    if ((id >= 4096) && (id < 4325)) return true;

    // Not a valid id.
    return false;
}

bool rvTags384_GetCorners(rvUint16 id, CvPoint3D32f corners[RVTAG_CORNER_COUNT])
// Converts tag id to the XY position of each corner of the tag.
{
    int rv = false;
    CvPoint2D32f pt;

    // Is this a small target ID?
    if (id < 400)
    {
        // Convert the ID to a position.
        pt.x = (float) (id % 16);
        pt.y = (float) (id / 16);

        // Convert position to millimeters.
        pt.x *= -30.0f;
        pt.y *= -30.0f;

        // Shift position to the center.
        pt.x += 235.0f;
        pt.y += 380.0f;

        // Set the corner positions for the tag.
        corners[0].x = pt.x;
        corners[0].y = pt.y;
        corners[0].z = 0.0;
        corners[1].x = pt.x - 20.0f;
        corners[1].y = pt.y;
        corners[1].z = 0.0;
        corners[2].x = pt.x - 20.0f;
        corners[2].y = pt.y - 20.0f;
        corners[2].z = 0.0;
        corners[3].x = pt.x;
        corners[3].y = pt.y - 20.0f;
        corners[3].z = 0.0;

        // Success.
        rv = true;
    }
    // Is this a big target ID?
    else if ((id > 1023) && (id < 1064))
    {
        // Convert the ID to a position.
        pt.x = (float) ((id - 1024) % 5);
        pt.y = (float) ((id - 1024) / 5);

        // Convert position to millimeters.
        pt.x *= -90.0f;
        pt.y *= -90.0f;

        // Adjust the origin.
        pt.x -= 30.0f;
        pt.y -= 30.0f;

        // Shift position to the center.
        pt.x += 235.0f;
        pt.y += 380.0f;

        // Set the corner positions for the tag.
        corners[0].x = pt.x;
        corners[0].y = pt.y;
        corners[0].z = 0.0;
        corners[1].x = pt.x - 50.0f;
        corners[1].y = pt.y;
        corners[1].z = 0.0;
        corners[2].x = pt.x - 50.0f;
        corners[2].y = pt.y - 50.0f;
        corners[2].z = 0.0;
        corners[3].x = pt.x;
        corners[3].y = pt.y - 50.0f;
        corners[3].z = 0.0;

        // Success.
        rv = true;
    }
    else if (id == 2048)
    {
        corners[0].x = 15.0f;
        corners[0].y = 15.0f;
        corners[0].z = 0.0f;
        corners[1].x = -15.0f;
        corners[1].y = 15.0f;
        corners[1].z = 0.0f;
        corners[2].x = -15.0f;
        corners[2].y = -15.0f;
        corners[2].z = 0.0f;
        corners[3].x = 15.0f;
        corners[3].y = -15.0f;
        corners[3].z = 0.0f;
    }
    else if ((id >= 2049) && (id <= 2052))
    {
        corners[0].x = 20.0f;
        corners[0].y = 20.0f;
        corners[0].z = 0.0f;
        corners[1].x = -20.0f;
        corners[1].y = 20.0f;
        corners[1].z = 0.0f;
        corners[2].x = -20.0f;
        corners[2].y = -20.0f;
        corners[2].z = 0.0f;
        corners[3].x = 20.0f;
        corners[3].y = -20.0f;
        corners[3].z = 0.0f;
    }
    else
    {
        corners[0].x = 10.0f;
        corners[0].y = 10.0f;
        corners[0].z = 0.0f;
        corners[1].x = -10.0f;
        corners[1].y = 10.0f;
        corners[1].z = 0.0f;
        corners[2].x = -10.0f;
        corners[2].y = -10.0f;
        corners[2].z = 0.0f;
        corners[3].x = 10.0f;
        corners[3].y = -10.0f;
        corners[3].z = 0.0f;
    }

    return rv;
}

