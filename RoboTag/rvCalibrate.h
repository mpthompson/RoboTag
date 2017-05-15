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

    $Id: rvCalibrate.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_CALIBRATE_INCLUDED_
#define _RV_CALIBRATE_INCLUDED_

#include "rvTypes.h"
#include "rvTag.h"
#include "cv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RVCALIBRATE_MAX_TAGS        (16 * 1024)
#define RVCALIBRATE_MAX_VIEWS       (1024)

// Calibrate types.
typedef struct _rvCalibrate rvCalibrate;

// Calibrate structures.
struct _rvCalibrate
{
    bool results;

    CvMat *cameraMatrix;
    CvMat *distortionCoeffs;

    IplImage *grayImage;
    IplImage *edgeImage;
    CvMemStorage *memStorage;

    rvTag* tag;

#ifdef _DEBUG
    CvFont font;
#endif

    CvSize imageSize;

    rvUint16 tagCount;
    rvUint16 viewCount;
    rvUint16 tags[RVCALIBRATE_MAX_TAGS];
    rvUint16 views[RVCALIBRATE_MAX_VIEWS];
    CvPoint2D32f corners[RVCALIBRATE_MAX_TAGS * 4];
};

// Calibrate methods.
rvCalibrate *rvCalibrate_New(CvSize imageSize, int origin);
void rvCalibrate_Free(rvCalibrate *self);

bool rvCalibrate_SaveIntrinsics(rvCalibrate *self, const char *filepath);

bool rvCalibrate_GetCameraMatrix(rvCalibrate *self, CvMat **cameraMatrix);
bool rvCalibrate_GetDistortionCoeffs(rvCalibrate *self, CvMat **distortionCoeffs);

bool rvCalibrate_Reset(rvCalibrate *self);
bool rvCalibrate_GetTagCount(rvCalibrate *self, rvUint16* count);
bool rvCalibrate_GetViewCount(rvCalibrate *self, rvUint16* count);

bool rvCalibrate_AddView(rvCalibrate *self, IplImage *image, bool add);
bool rvCalibrate_ProcessViews(rvCalibrate *self);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_CALIBRATE_INCLUDED_
