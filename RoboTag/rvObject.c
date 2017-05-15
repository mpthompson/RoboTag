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

    $Id: rvObject.c 28 2010-03-09 23:49:39Z mike $
*/

#include "rvObject.h"

// Functions for getting information related to known objects.

static double small_block_points[72] =
{
    19.0, 19.0, 19.0,           -19.0, 19.0, 19.0,
    -19.0, 19.0, 19.0,          -19.0, -19.0, 19.0,
    -19.0, -19.0, 19.0,         19.0, -19.0, 19.0,
    19.0, -19.0, 19.0,          19.0, 19.0, 19.0,

    19.0, 19.0, -19.0,          -19.0, 19.0, -19.0,
    -19.0, 19.0, -19.0,         -19.0, -19.0, -19.0,
    -19.0, -19.0, -19.0,        19.0, -19.0, -19.0,
    19.0, -19.0, -19.0,         19.0, 19.0, -19.0,

    19.0, 19.0, 19.0,           19.0, 19.0, -19.0,
    -19.0, 19.0, 19.0,          -19.0, 19.0, -19.0,
    -19.0, -19.0, 19.0,         -19.0, -19.0, -19.0,
    19.0, -19.0, 19.0,          19.0, -19.0, -19.0
};

static double large_block_points[72] =
{
    25.0, 25.0, 25.0,           -25.0, 25.0, 25.0,
    -25.0, 25.0, 25.0,          -25.0, -25.0, 25.0,
    -25.0, -25.0, 25.0,         25.0, -25.0, 25.0,
    25.0, -25.0, 25.0,          25.0, 25.0, 25.0,

    25.0, 25.0, -25.0,          -25.0, 25.0, -25.0,
    -25.0, 25.0, -25.0,         -25.0, -25.0, -25.0,
    -25.0, -25.0, -25.0,        25.0, -25.0, -25.0,
    25.0, -25.0, -25.0,         25.0, 25.0, -25.0,

    25.0, 25.0, 25.0,           25.0, 25.0, -25.0,
    -25.0, 25.0, 25.0,          -25.0, 25.0, -25.0,
    -25.0, -25.0, 25.0,         -25.0, -25.0, -25.0,
    25.0, -25.0, 25.0,          25.0, -25.0, -25.0
};

static double star_block_points[180] =
{
    0.0, 51.7, 30.0,             -16.8, 23.3, 30.0,
    -16.8, 23.3, 30.0,           -49.2, 15.9, 30.0,
    -49.2, 15.9, 30.0,           -27.4, -9.3, 30.0,
    -27.4, -9.3, 30.0,           -30.5, -42.5, 30.0,
    -30.5, -42.5, 30.0,          0.0, -29.3, 30.0,
    0.0, -29.3, 30.0,            30.5, -42.5, 30.0,
    30.5, -42.5, 30.0,           27.4, -9.3, 30.0,
    27.4, -9.3, 30.0,            49.2, 15.9, 30.0,
    49.2, 15.9, 30.0,            16.8, 23.3, 30.0,
    16.8, 23.3, 30.0,            0.0, 51.7, 30.0,

    0.0, 51.7, -30.0,            -16.8, 23.3, -30.0,
    -16.8, 23.3, -30.0,          -49.2, 15.9, -30.0,
    -49.2, 15.9, -30.0,          -27.4, -9.3, -30.0,
    -27.4, -9.3, -30.0,          -30.5, -42.5, -30.0,
    -30.5, -42.5, -30.0,         0.0, -29.3, -30.0,
    0.0, -29.3, -30.0,           30.5, -42.5, -30.0,
    30.5, -42.5, -30.0,          27.4, -9.3, -30.0,
    27.4, -9.3, -30.0,           49.2, 15.9, -30.0,
    49.2, 15.9, -30.0,           16.8, 23.3, -30.0,
    16.8, 23.3, -30.0,           0.0, 51.7, -30.0,

    0.0, 51.7, 30.0,             0.0, 51.7, -30.0,
    -16.8, 23.3, 30.0,           -16.8, 23.3, -30.0,
    -49.2, 15.9, 30.0,           -49.2, 15.9, -30.0,
    -27.4, -9.3, 30.0,           -27.4, -9.3, -30.0,
    -30.5, -42.5, 30.0,          -30.5, -42.5, -30.0,
    0.0, -29.3, 30.0,            0.0, -29.3, -30.0,
    30.5, -42.5, 30.0,           30.5, -42.5, -30.0,
    27.4, -9.3, 30.0,            27.4, -9.3, -30.0,
    49.2, 15.9, 30.0,            49.2, 15.9, -30.0,
    16.8, 23.3, 30.0,            16.8, 23.3, -30.0
};


static double hex_block_points[108] =
{
    -25.5, 44.2, 24.0,          -51.0, 0.0, 24.0,
    -51.0, 0.0, 24.0,           -25.5, -44.2, 24.0,
    -25.5, -44.2, 24.0,         25.5, -44.2, 24.0,
    25.5, -44.2, 24.0,          51.0, 0.0, 24.0,
    51.0, 0.0, 24.0,            25.5, 44.2, 24.0,
    25.5, 44.2, 24.0,           -25.5, 44.2, 24.0,

    -25.5, 44.2, -24.0,         -51.0, 0.0, -24.0,
    -51.0, 0.0, -24.0,          -25.5, -44.2, -24.0,
    -25.5, -44.2, -24.0,        25.5, -44.2, -24.0,
    25.5, -44.2, -24.0,         51.0, 0.0, -24.0,
    51.0, 0.0, -24.0,           25.5, 44.2, -24.0,
    25.5, 44.2, -24.0,          -25.5, 44.2, -24.0,

    -25.5, 44.2, 24.0,          -25.5, 44.2, -24.0,
    -51.0, 0.0, 24.0,           -51.0, 0.0, -24.0,
    -25.5, -44.2, 24.0,         -25.5, -44.2, -24.0,
    25.5, -44.2, 24.0,          25.5, -44.2, -24.0,
    51.0, 0.0, 24.0,            51.0, 0.0, -24.0,
    25.5, 44.2, 24.0,           25.5, 44.2, -24.0
};


static double plug_points[120] =
{
    32.0, 54.0, 0.0,            -32.0, 54.0, 0.0,
    -32.0, 54.0, 0.0,           -32.0, -54.0, 0.0,
    -32.0, -54.0, 0.0,          32.0, -54.0, 0.0,
    32.0, -54.0, 0.0,           32.0, 54.0, 0.0,

    7.0, 26.0, 0.0,            5.0, 26.0, 0.0,
    5.0, 26.0, 0.0,            5.0, 19.0, 0.0,
    5.0, 19.0, 0.0,            7.0, 19.0, 0.0,
    7.0, 19.0, 0.0,            7.0, 26.0, 0.0,

    -7.0, 26.0, 0.0,           -5.0, 26.0, 0.0,
    -5.0, 26.0, 0.0,           -5.0, 20.0, 0.0,
    -5.0, 20.0, 0.0,           -7.0, 20.0, 0.0,
    -7.0, 20.0, 0.0,           -7.0, 26.0, 0.0,

    7.0, -13.0, 0.0,            5.0, -13.0, 0.0,
    5.0, -13.0, 0.0,            5.0, -20.0, 0.0,
    5.0, -20.0, 0.0,            7.0, -20.0, 0.0,
    7.0, -20.0, 0.0,            7.0, -13.0, 0.0,

    -7.0, -13.0, 0.0,           -5.0, -13.0, 0.0,
    -5.0, -13.0, 0.0,           -5.0, -19.0, 0.0,
    -5.0, -19.0, 0.0,           -7.0, -19.0, 0.0,
    -7.0, -19.0, 0.0,           -7.0, -13.0, 0.0
};


bool rvObject_GetObjectVectors(rvUint16 id, CvMat *positionMatrix, CvMat *objectVectors)
// Gets the shape as a matrix of 4xN vectors with float values.
{
    bool rv;
    CvMat tempMatrix;
    CvMat offsetMatrix;
    double tempData[16];
    double offsetData[16];

    // Assume we fail.
    rv = false;

    // Do we know the object.
    if (id == 2048)
    {
        // Initialize the offset matrix.
        cvInitMatHeader(&tempMatrix, 4, 4, CV_64FC1, &tempData, CV_AUTOSTEP);
        cvInitMatHeader(&offsetMatrix, 4, 4, CV_64FC1, &offsetData, CV_AUTOSTEP);

        // Copy the position matrix to the temporary matrix.
        cvCopy(positionMatrix, &tempMatrix, NULL);

        // Set the offset matrix to identity.
        cvSetIdentity(&offsetMatrix, cvRealScalar(1));

        // Set the offset matrix to translate from the origin of the object.
        cvSetReal2D(&offsetMatrix, 2, 3, cvGetReal2D(&offsetMatrix, 2, 3) - 19.0);

        // Multiply the positon (temp) matrix by the offset matrix to compensate
        // for the location of the tag relative to the origin of the object.
        cvMatMul(&tempMatrix, &offsetMatrix, positionMatrix);

        // Initialize the object vectors matrix.
        cvInitMatHeader(objectVectors, 24, 3, CV_64FC1, small_block_points, CV_AUTOSTEP);

        // We succeeded.
        rv = true;
    }
    else if (id == 2049)
    {
        // Initialize the offset matrix.
        cvInitMatHeader(&tempMatrix, 4, 4, CV_64FC1, &tempData, CV_AUTOSTEP);
        cvInitMatHeader(&offsetMatrix, 4, 4, CV_64FC1, &offsetData, CV_AUTOSTEP);

        // Copy the position matrix to the temporary matrix.
        cvCopy(positionMatrix, &tempMatrix, NULL);

        // Set the offset matrix to identity.
        cvSetIdentity(&offsetMatrix, cvRealScalar(1));

        // Set the offset matrix to translate from the origin of the object.
        cvSetReal2D(&offsetMatrix, 2, 3, cvGetReal2D(&offsetMatrix, 2, 3) - 25.0);

        // Multiply the positon (temp) matrix by the offset matrix to compensate
        // for the location of the tag relative to the origin of the object.
        cvMatMul(&tempMatrix, &offsetMatrix, positionMatrix);

        // Initialize the object vectors matrix.
        cvInitMatHeader(objectVectors, 24, 3, CV_64FC1, large_block_points, CV_AUTOSTEP);

        // We succeeded.
        rv = true;
    }
    else if (id == 2050)
    {
        // Initialize the offset matrix.
        cvInitMatHeader(&tempMatrix, 4, 4, CV_64FC1, &tempData, CV_AUTOSTEP);
        cvInitMatHeader(&offsetMatrix, 4, 4, CV_64FC1, &offsetData, CV_AUTOSTEP);

        // Copy the position matrix to the temporary matrix.
        cvCopy(positionMatrix, &tempMatrix, NULL);

        // Set the offset matrix to identity.
        cvSetIdentity(&offsetMatrix, cvRealScalar(1));

        // Set the offset matrix to translate from the origin of the object.
        cvSetReal2D(&offsetMatrix, 1, 3, cvGetReal2D(&offsetMatrix, 1, 3) + 3.0);
        cvSetReal2D(&offsetMatrix, 2, 3, cvGetReal2D(&offsetMatrix, 2, 3) - 30.0);

        // Multiply the positon (temp) matrix by the offset matrix to compensate
        // for the location of the tag relative to the origin of the object.
        cvMatMul(&tempMatrix, &offsetMatrix, positionMatrix);

        // Initialize the object vectors matrix.
        cvInitMatHeader(objectVectors, 60, 3, CV_64FC1, star_block_points, CV_AUTOSTEP);

        // We succeeded.
        rv = true;
    }
    else if (id == 2051)
    {
        // Initialize the offset matrix.
        cvInitMatHeader(&tempMatrix, 4, 4, CV_64FC1, &tempData, CV_AUTOSTEP);
        cvInitMatHeader(&offsetMatrix, 4, 4, CV_64FC1, &offsetData, CV_AUTOSTEP);

        // Copy the position matrix to the temporary matrix.
        cvCopy(positionMatrix, &tempMatrix, NULL);

        // Set the offset matrix to identity.
        cvSetIdentity(&offsetMatrix, cvRealScalar(1));

        // Set the offset matrix to translate from the origin of the object.
        cvSetReal2D(&offsetMatrix, 2, 3, cvGetReal2D(&offsetMatrix, 2, 3) - 24.0);

        // Multiply the positon (temp) matrix by the offset matrix to compensate
        // for the location of the tag relative to the origin of the object.
        cvMatMul(&tempMatrix, &offsetMatrix, positionMatrix);

        // Initialize the object vectors matrix.
        cvInitMatHeader(objectVectors, 36, 3, CV_64FC1, hex_block_points, CV_AUTOSTEP);

        // We succeeded.
        rv = true;
    }
    else if (id == 2052)
    {
        // Initialize the offset matrix.
        cvInitMatHeader(&tempMatrix, 4, 4, CV_64FC1, &tempData, CV_AUTOSTEP);
        cvInitMatHeader(&offsetMatrix, 4, 4, CV_64FC1, &offsetData, CV_AUTOSTEP);

        // Copy the position matrix to the temporary matrix.
        cvCopy(positionMatrix, &tempMatrix, NULL);

        // Set the offset matrix to identity.
        cvSetIdentity(&offsetMatrix, cvRealScalar(1));

        // Set the offset matrix to translate from the origin of the object.
        cvSetReal2D(&offsetMatrix, 1, 3, cvGetReal2D(&offsetMatrix, 1, 3) - 88.0);
        cvSetReal2D(&offsetMatrix, 2, 3, cvGetReal2D(&offsetMatrix, 2, 3) + 5.0);

        // Multiply the positon (temp) matrix by the offset matrix to compensate
        // for the location of the tag relative to the origin of the object.
        cvMatMul(&tempMatrix, &offsetMatrix, positionMatrix);

        // Initialize the object vectors matrix.
        cvInitMatHeader(objectVectors, 40, 3, CV_64FC1, plug_points, CV_AUTOSTEP);

        // We succeeded.
        rv = true;
    }

    return rv;
}

