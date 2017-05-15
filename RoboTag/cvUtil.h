/*
    Additional OpenCV related utility functions.

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

    $Id: cvUtil.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _CVUTIL_INCLUDED_
#define _CVUTIL_INCLUDED_

#include "cv.h"
#include "cvSusan.h"

void cvNormalizeCorners(CvPoint2D32f corners[4]);
void cvVectorsToMatrix(CvMat *rotationVector, CvMat *translationVector, CvMat *matrix);
void cvMatrixToVectors(CvMat *matrix, CvMat *rotationVector, CvMat *translationVector);
void cvMatToAngles(CvMat *matrix, double* angleX, double* angleY, double* angleZ);
void cvMatToPositions(CvMat *matrix, double* positionX, double* positionY, double* positionZ);
void cvDrawArrow(CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness CV_DEFAULT(1), int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
void cvDrawCross(IplImage* img, CvPoint point, CvScalar color);
void cvDrawCrosses(IplImage* img, CvPoint2D32f *points, int count, CvScalar color);
void cvDrawLines(IplImage* img, CvPoint2D32f* points, int count, CvScalar color, int thickness CV_DEFAULT(1), int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
void cvDrawCorners(CvArr* img, CvPoint2D32f corners[4], CvScalar colorFirst, CvScalar colorRest, int thickness CV_DEFAULT(1), int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
void cvDrawContourArrows(CvArr* img, CvSeq* contours, CvScalar colorFirst, CvScalar colorRest, int thickness CV_DEFAULT(1), int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
void cvDrawPositionInfo(IplImage* image, CvMat *positionMatrix, int eulerAngles);

#endif // _CVUTIL_INCLUDED_

