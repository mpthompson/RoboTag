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

    $Id: rvGrid.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_GRID_INCLUDED_
#define _RV_GRID_INCLUDED_

#include "rvTypes.h"
#include "rvTag.h"
#include "cv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RVGRID_MAX_NAV_TAGS         128
#define RVGRID_MAX_OBJ_TAGS         16
#define RVGRID_MAX_CHAR_TAGS        32
#define RVGRID_MAX_CALIBRATE_TAGS   (32 * 256)
#define RVGRID_MAX_CALIBRATE_IMAGES (256)

enum
{
    RVGRID_DISPLAY_COLOR = 0,
    RVGRID_DISPLAY_GRAY,
    RVGRID_DISPLAY_EDGE,
    RVGRID_DISPLAY_COUNT
};

enum
{
    RVGRID_EDGE_ADAPTIVE = 0,
    RVGRID_EDGE_CANNY,
    RVGRID_EDGE_SUZAN,
    RVGRID_EDGE_COUNT
};


enum
{
    RVGRID_ADAPTIVE_METHOD_MEAN = 0,
    RVGRID_ADAPTIVE_METHOD_GAUSSIAN,
    RVGRID_ADAPTIVE_METHOD_COUNT
};

// Grid types.
typedef struct _rvGrid rvGrid;
typedef struct _rvGridNavTag rvGridNavTag;
typedef struct _rvGridObjTag rvGridObjTag;
typedef struct _rvGridCharTag rvGridCharTag;

// Grid navigation tag structure.
struct _rvGridNavTag
{
    rvUint16 id;
    CvPoint2D32f corners[4];
};

// Grid object tag structure.
struct _rvGridObjTag
{
    rvUint16 id;
    CvPoint2D32f corners[4];
    double rotationData[3];
    double translationData[3];
    double positionData[16];
    CvMat rotationVector;
    CvMat translationVector;
    CvMat positionMatrix;
};

// Grid character tag structure.
struct _rvGridCharTag
{
    rvUint16 id;
    CvPoint2D32f corners[4];
};

// Grid structures.
struct _rvGrid
{
    bool results;

    CvMat *cameraMatrix;
    CvMat *distortionCoeffs;
    CvMat *rotationVector;
    CvMat *translationVector;
    CvMat *cameraPositionMatrix;

    CvSize imageSize;
    IplImage *grayImage;
    IplImage *edgeImage;
    CvMemStorage *memStorage;

    rvTag *tag;

    CvFont idFont;
    CvFont charFont;

    // Navigation tags.
    rvUint16 navTagCount;
    rvGridNavTag navTags[RVGRID_MAX_NAV_TAGS];

    // Object tags.
    rvUint16 objTagCount;
    rvGridObjTag objTags[RVGRID_MAX_OBJ_TAGS];

    // Character tags.
    rvUint16 charTagCount;
    rvGridCharTag charTags[RVGRID_MAX_CHAR_TAGS];

    // Calibration data.
    rvUint16 calibrateTagCount;
    rvUint16 calibrateImageCount;
    rvUint16 calibrateTags[RVGRID_MAX_CALIBRATE_TAGS];
    rvUint16 calibrateImages[RVGRID_MAX_CALIBRATE_IMAGES];
    CvPoint2D32f calibrateCorners[RVGRID_MAX_CALIBRATE_TAGS * 4];

    // Properties.
    int display;                // Processing display type.
    int edgeMethod;             // Edge detection method.
    int gaussianBlur;           // Gausian blur applied against gray image.
    int adaptiveMethod;         // Adaptive method -- mean or guassian.
    int adaptiveBlockSize;      // Adaptive block size.
    int adaptiveSubtraction;    // Adaptive subtraction.
    int edgeDilation;           // Edge dilation.

    // Flags to control drawing of tag properties.
    bool drawRawContours;
    bool drawPolygonContours;
    bool drawQuadContours;
    bool drawTagCorners;
    bool drawTagReferences;
    bool drawTagSamples;
    bool drawTagIdentifiers;
    bool drawCameraPosition;
    bool drawTagReprojection;
    bool drawObjectReprojection;
    bool drawCharacters;
};

// Grid methods.
rvGrid *rvGrid_New(CvSize imageSize, int origin);
void rvGrid_Free(rvGrid *self);

// Intrinsics methods.
bool rvGrid_ResetIntrinsics(rvGrid *self);
bool rvGrid_LoadIntrinsics(rvGrid *self, const char *filepath);
bool rvGrid_SaveIntrinsics(rvGrid *self, const char *filepath);

// Property getters.
int rvGrid_GetDisplay(rvGrid *self);
int rvGrid_GetEdgeMethod(rvGrid *self);
int rvGrid_GetEdgeDilation(rvGrid *self);
int rvGrid_GetGaussianBlur(rvGrid *self);
int rvGrid_GetAdaptiveMethod(rvGrid *self);
int rvGrid_GetAdaptiveBlockSize(rvGrid *self);
int rvGrid_GetAdaptiveSubtraction(rvGrid *self);

// Draw property getters.
bool rvGrid_GetDrawRawContours(rvGrid *self);
bool rvGrid_GetDrawPolygonContours(rvGrid *self);
bool rvGrid_GetDrawQuadContours(rvGrid *self);
bool rvGrid_GetDrawTagCorners(rvGrid *self);
bool rvGrid_GetDrawTagReferences(rvGrid *self);
bool rvGrid_GetDrawTagSamples(rvGrid *self);
bool rvGrid_GetDrawTagIdentifiers(rvGrid *self);
bool rvGrid_GetDrawCameraPosition(rvGrid *self);
bool rvGrid_GetDrawTagReprojection(rvGrid *self);
bool rvGrid_GetDrawObjectReprojection(rvGrid *self);
bool rvGrid_GetDrawCharacters(rvGrid *self);

// Property setters.
void rvGrid_SetDisplay(rvGrid *self, int display);
void rvGrid_SetEdgeMethod(rvGrid *self, int edgeMethod);
void rvGrid_SetEdgeDilation(rvGrid *self, int edgeDilation);
void rvGrid_SetGaussianBlur(rvGrid *self, int gaussianBlur);
void rvGrid_SetAdaptiveMethod(rvGrid *self, int adaptiveMethod);
void rvGrid_SetAdaptiveBlockSize(rvGrid *self, int adaptiveMethod);
void rvGrid_SetAdaptiveSubtraction(rvGrid *self, int adaptiveMethod);

// Draw property setters.
void rvGrid_SetDrawRawContours(rvGrid *self, bool value);
void rvGrid_SetDrawPolygonContours(rvGrid *self, bool value);
void rvGrid_SetDrawQuadContours(rvGrid *self, bool value);
void rvGrid_SetDrawTagCorners(rvGrid *self, bool value);
void rvGrid_SetDrawTagReferences(rvGrid *self, bool value);
void rvGrid_SetDrawTagSamples(rvGrid *self, bool value);
void rvGrid_SetDrawTagIdentifiers(rvGrid *self, bool value);
void rvGrid_SetDrawCameraPosition(rvGrid *self, bool value);
void rvGrid_SetDrawTagReprojection(rvGrid *self, bool value);
void rvGrid_SetDrawObjectReprojection(rvGrid *self, bool value);
void rvGrid_SetDrawCharacters(rvGrid *self, bool value);

bool rvGrid_SetCameraMatrix(rvGrid *self, CvMat *cameraMatrix);
bool rvGrid_SetDistortionCoeffs(rvGrid *self, CvMat *distortionCoeffs);

bool rvGrid_GetCameraMatrix(rvGrid *self, CvMat **cameraMatrix);
bool rvGrid_GetDistortionCoeffs(rvGrid *self, CvMat **distortionCoeffs);
bool rvGrid_GetRotationVector(rvGrid *self, CvMat **rotationVector);
bool rvGrid_GetTranslationVector(rvGrid *self, CvMat **translationVector);
bool rvGrid_GetCameraPositionMatrix(rvGrid *self, CvMat **inverseExtrinsicMatrix);

bool rvGrid_CameraPosition(rvGrid *self);
bool rvGrid_ObjectPositions(rvGrid *self);

// Processing method.
bool rvGrid_ProcessImage(rvGrid *self, IplImage *image);

// Calibration methods.
bool rvGrid_CalibrateAdd(rvGrid *self);
bool rvGrid_CalibrateReset(rvGrid *self);
int rvGrid_GetCalibrateTagCount(rvGrid *self);
int rvGrid_GetCalibrateImageCount(rvGrid *self);
bool rvGrid_Calibrate(rvGrid *self);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_GRID_INCLUDED_
