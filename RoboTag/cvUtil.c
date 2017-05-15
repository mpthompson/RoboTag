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

    $Id: cvUtil.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include <stdio.h>
#include "cvUtil.h"

#define RADIANS 57.2957795

void cvNormalizeCorners(CvPoint2D32f corners[4])
{
    CvPoint3D32f v1;
    CvPoint3D32f v2;
    float z;

    // Create two 3D vectors from the first two lines of the polygon.
    v1.x = corners[1].x - corners[0].x;
    v1.y = corners[1].y - corners[0].y;
    v1.z = 0.0f;
    v2.x = corners[1].x - corners[2].x;
    v2.y = corners[1].y - corners[2].y;
    v2.z = 0.0f;

    // Determine the sign of the Z component of the cross product.
    z = v1.x * v2.y - v2.x - v1.y;

    // If the Z component is negative we need to reverse the sequence of the corners.
    if (z < 0.0)
    {
        CvPoint2D32f tmp = corners[1];
        corners[1] = corners[3];
        corners[3] = tmp;
    }
}


void cvVectorsToMatrix(CvMat *rotationVector, CvMat *translationVector, CvMat *matrix)
// This function converts the compact translation and rotation vectors
// as output from cvFindExtrinsicCameraParams2 into 4x4 matrix.  The
// The translation and rotation vectors must be (3x1 or 1x3).  The
// matrix to be filled in must be (4x4).
{
    double rotationData[9];
    CvMat rotationMatrix;

    // Zero the matrix.
    cvSetZero(matrix);

    // Set the translation values in the matrix.
    cvSetReal2D(matrix, 0, 3, cvGetReal1D(translationVector, 0));
    cvSetReal2D(matrix, 1, 3, cvGetReal1D(translationVector, 1));
    cvSetReal2D(matrix, 2, 3, cvGetReal1D(translationVector, 2));

    // Initialize the rotation matrix.
    cvInitMatHeader(&rotationMatrix, 3, 3, CV_64FC1, rotationData, CV_AUTOSTEP);

    // Convert the rotation vector to a rotation matrix.  The rotation
    // vector is a compact representation of rotation matrix.
    cvRodrigues2(rotationVector, &rotationMatrix, NULL);

    // Move the rotation values into the matrix.
    cvSetReal2D(matrix, 0, 0, cvGetReal2D(&rotationMatrix, 0, 0));
    cvSetReal2D(matrix, 0, 1, cvGetReal2D(&rotationMatrix, 0, 1));
    cvSetReal2D(matrix, 0, 2, cvGetReal2D(&rotationMatrix, 0, 2));
    cvSetReal2D(matrix, 1, 0, cvGetReal2D(&rotationMatrix, 1, 0));
    cvSetReal2D(matrix, 1, 1, cvGetReal2D(&rotationMatrix, 1, 1));
    cvSetReal2D(matrix, 1, 2, cvGetReal2D(&rotationMatrix, 1, 2));
    cvSetReal2D(matrix, 2, 0, cvGetReal2D(&rotationMatrix, 2, 0));
    cvSetReal2D(matrix, 2, 1, cvGetReal2D(&rotationMatrix, 2, 1));
    cvSetReal2D(matrix, 2, 2, cvGetReal2D(&rotationMatrix, 2, 2));

    // Set the last element of the matrix.
    cvSetReal2D(matrix, 3, 3, 1.0);
}


void cvMatrixToVectors(CvMat *matrix, CvMat *rotationVector, CvMat *translationVector)
// This function converts the matrix to the compact translation and
// rotation vectors. The matrix must be (4x4) and the translation and
// rotation vectors must be (3x1 or 1x3).
{
    double rotationData[9];
    CvMat rotationMatrix;

    // Set the translation values from the matrix.
    cvSetReal1D(translationVector, 0, cvGetReal2D(matrix, 0, 3));
    cvSetReal1D(translationVector, 1, cvGetReal2D(matrix, 1, 3));
    cvSetReal1D(translationVector, 2, cvGetReal2D(matrix, 2, 3));

    // Initialize the rotation matrix.
    cvInitMatHeader(&rotationMatrix, 3, 3, CV_64FC1, rotationData, CV_AUTOSTEP);

    // Move the rotation values into the matrix.
    cvSetReal2D(&rotationMatrix, 0, 0, cvGetReal2D(matrix, 0, 0));
    cvSetReal2D(&rotationMatrix, 0, 1, cvGetReal2D(matrix, 0, 1));
    cvSetReal2D(&rotationMatrix, 0, 2, cvGetReal2D(matrix, 0, 2));
    cvSetReal2D(&rotationMatrix, 1, 0, cvGetReal2D(matrix, 1, 0));
    cvSetReal2D(&rotationMatrix, 1, 1, cvGetReal2D(matrix, 1, 1));
    cvSetReal2D(&rotationMatrix, 1, 2, cvGetReal2D(matrix, 1, 2));
    cvSetReal2D(&rotationMatrix, 2, 0, cvGetReal2D(matrix, 2, 0));
    cvSetReal2D(&rotationMatrix, 2, 1, cvGetReal2D(matrix, 2, 1));
    cvSetReal2D(&rotationMatrix, 2, 2, cvGetReal2D(matrix, 2, 2));

    // Convert the rotation matrix to a rotation vector.  The rotation
    // vector is a compact representation of rotation matrix.
    cvRodrigues2(&rotationMatrix, rotationVector, NULL);
}


void cvMatToAngles(CvMat *matrix, double* angleX, double* angleY, double* angleZ)
// Utility function to convert rotation portion of matrix to 3D Euler angles.
{
    double cosY;
    double trX;
    double trY;

    // Calculate Y-axis angle.
    *angleY = asin(cvGetReal2D(matrix, 0, 2));

    cosY = cos(*angleY);

    // Gimball lock?
    if (fabs(cosY) > 0.005)
    {
        // No, so get X-axis angle.
        trX = cvGetReal2D(matrix, 2, 2) / cosY;
        trY = -cvGetReal2D(matrix, 1, 2) / cosY;
        *angleX = atan2(trY, trX);
        trX = cvGetReal2D(matrix, 0, 0) / cosY;
        trY = -cvGetReal2D(matrix, 0, 1) / cosY;
        *angleZ = atan2(trY, trX);
    }
    else
    {
        *angleX = 0.0;
        trX = cvGetReal2D(matrix, 1, 1);
        trY = cvGetReal2D(matrix, 1, 0);
        *angleZ = atan2(trY, trX);
    }

    // Convert to degrees.
    *angleY *= RADIANS;
    *angleX *= RADIANS;
    *angleZ *= RADIANS;

    // Return only positive angles.
    if (*angleX < 0.0) *angleX += 360;
    if (*angleY < 0.0) *angleY += 360;
    if (*angleZ < 0.0) *angleZ += 360;
}


void cvMatToPositions(CvMat *matrix, double* positionX, double* positionY, double* positionZ)
// Utility function to convert rotation portion of matrix to 3D positions.
{
    *positionX = cvGetReal2D(matrix, 0, 3);
    *positionY = cvGetReal2D(matrix, 1, 3);
    *positionZ = cvGetReal2D(matrix, 2, 3);
}


void cvDrawArrow(CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness, int line_type, int shift)
// Draw an arrow connecting two points with the second point being the head of the arrow.
{
    double angle = atan2((double) pt1.y - pt2.y, (double) pt1.x - pt2.x);
    cvLine(img, pt1, pt2, color, thickness, line_type, shift);
    pt1.x = (int) (pt2.x + 9 * cos(angle + (CV_PI / 8)));
    pt1.y = (int) (pt2.y + 9 * sin(angle + (CV_PI / 8)));
    cvLine(img, pt1, pt2, color, thickness, line_type, shift);
    pt1.x = (int) (pt2.x + 9 * cos(angle - (CV_PI / 8)));
    pt1.y = (int) (pt2.y + 9 * sin(angle - (CV_PI / 8)));
    cvLine(img, pt1, pt2, color, thickness, line_type, shift);
}


void cvDrawCross(IplImage* img, CvPoint point, CvScalar color)
// Draw a small cross at the indicated point.
{
    uchar *pixel;
    uchar *pixel_lt;
    uchar *pixel_rt;
    uchar *pixel_up;
    uchar *pixel_dn;
    uchar red = cvRound(color.val[0]);
    uchar green = cvRound(color.val[1]);
    uchar blue = cvRound(color.val[2]);

    // Sanity check the values.
    if (point.x < 1 || point.y < 1 || point.x >= (img->width - 1) || point.y >= (img->height - 1)) return;

    // Get the pixels to be colored.
    pixel = &((uchar*)(img->imageData + img->widthStep * point.y))[point.x * 3];
    pixel_lt = &((uchar*)(img->imageData + img->widthStep * point.y))[(point.x - 1) * 3];
    pixel_rt = &((uchar*)(img->imageData + img->widthStep * point.y))[(point.x + 1) * 3];
    pixel_up = &((uchar*)(img->imageData + img->widthStep * (point.y - 1)))[point.x * 3];
    pixel_dn = &((uchar*)(img->imageData + img->widthStep * (point.y + 1)))[point.x * 3];

    // Draw the cross.
    pixel[0] = red; pixel[1] = green; pixel[2] = blue;
    pixel_lt[0] = red; pixel_lt[1] = green; pixel_lt[2] = blue;
    pixel_rt[0] = red; pixel_rt[1] = green; pixel_rt[2] = blue;
    pixel_up[0] = red; pixel_up[1] = green; pixel_up[2] = blue;
    pixel_dn[0] = red; pixel_dn[1] = green; pixel_dn[2] = blue;
}


void cvDrawCrosses(IplImage* img, CvPoint2D32f *points, int count, CvScalar color)
// Draw multiple crosses.
{
    int i;

    // Loop over each point.
    for (i = 0; i < count; ++i)
    {
        CvPoint pt;
        pt.x = cvRound(points[i].x);
        pt.y = cvRound(points[i].y);
        cvDrawCross(img, pt, color);
    }
}

void cvDrawLines(IplImage* img, CvPoint2D32f* points, int count, CvScalar color, int thickness, int line_type, int shift)
// Draw lines between the points.
{
    int i;
    CvPoint pt1;
    CvPoint pt2;

    if (count < 1) return;

    // Get the first point.
    pt1.x = cvRound(points[0].x);
    pt1.y = cvRound(points[0].y);

    // Loop over each remaining point.
    for (i = 1; i < count; ++i)
    {
        // Get the next point.
        pt2.x = cvRound(points[i].x);
        pt2.y = cvRound(points[i].y);

        // Draw a line between the points.
        cvLine(img, pt1, pt2, color, thickness, line_type, shift);

        // Move to the next point.
        pt1 = pt2;
    }
}

void cvDrawCorners(CvArr* img, CvPoint2D32f corners[4], CvScalar colorFirst, CvScalar colorRest, int thickness, int line_type, int shift)
// Draw arrows around corners.
{
    // Get each grid point.
    CvPoint p1 = cvPointFrom32f(corners[0]);
    CvPoint p2 = cvPointFrom32f(corners[1]);
    CvPoint p3 = cvPointFrom32f(corners[2]);
    CvPoint p4 = cvPointFrom32f(corners[3]);

    // Draw the line segments as arrows.
    cvDrawArrow(img, p1, p2, colorFirst, thickness, line_type, shift);
    cvDrawArrow(img, p2, p3, colorRest, thickness, line_type, shift);
    cvDrawArrow(img, p3, p4, colorRest, thickness, line_type, shift);
    cvDrawArrow(img, p4, p1, colorRest, thickness, line_type, shift);
}


void cvDrawContourArrows(CvArr* img, CvSeq* contours, CvScalar colorFirst, CvScalar colorRest, int thickness, int line_type, int shift)
// Draw arrows across the countours.
{
    int i;
    CvPoint p1;
    CvPoint p2;

    // Draw each contour as arrows.
    while (contours)
    {
        // Get the first contour point.
        p1 = *((CvPoint*) cvGetSeqElem(contours, 0));

        // Draw each line in the contour as an arrow.
        for (i = 1; i < contours->total; ++i)
        {
            // Get the next contour point.
            p2 = *((CvPoint*) cvGetSeqElem(contours, i));

            // Draw the arrow.
            cvDrawArrow(img, p1, p2, i == 1 ? colorFirst : colorRest, thickness, line_type, shift);

            // Save the next contour point.
            p1 = p2;
        }

        // Get the first contour again.
        p2 = *((CvPoint*) cvGetSeqElem(contours, 0));

        // Draw the final arrow.
        cvDrawArrow(img, p1, p2, i == 1 ? colorFirst : colorRest, thickness, line_type, shift);

        // Get the next contour.
        contours = contours->h_next;
    }
}

void cvDrawPositionInfo(IplImage* image, CvMat *positionMatrix, int eulerAngles)
{
    CvFont font;
    CvScalar color = CV_RGB(64, 64, 255);

    // Text buffer.
    char buffer[512];

    // Initialize a font.
    cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f, 0.0f, 2, 8);

    if (eulerAngles)
    {
        double angleX, angleY, angleZ;
        double positionX, positionY, positionZ;

        // Get the angles.
        cvMatToAngles(positionMatrix, &angleX, &angleY, &angleZ);

        // Get the positions.
        cvMatToPositions(positionMatrix, &positionX, &positionY, &positionZ);

        // Add to the angles and position to support rounding.
        angleX += 0.05;
        angleY += 0.05;
        angleZ += 0.05;
        positionX += 0.05;
        positionY += 0.05;
        positionZ += 0.05;

        cvRectangle(image, cvPoint(5, 475), cvPoint(240, 435), CV_RGB(200, 200, 200), CV_FILLED, 8, 0);

        // Print the positions.
        sprintf(buffer, "%#+7.1f  %#+7.1f  %#+7.1f", positionX, positionY, positionZ);

        // Write the buffer to the image.
        cvPutText(image, buffer, cvPoint(10, 460), &font, color);

        // Print the angles.
        sprintf(buffer, "%#+7.1f  %#+7.1f  %#+7.1f", angleX, angleY, angleZ);

        // Write the buffer to the image.
        cvPutText(image, buffer, cvPoint(10, 440), &font, color);
    }
    else
    {
        int i;
        int xText;
        int yText;

        // Print out the inverted extrinsic matrix.
        for (i = 0, xText = 10, yText = 460; i < 4; i += 1, yText -= 20)
        {
            // Print the rotation and translation information to a string.
            sprintf(buffer, "%#+7.2f %#+7.2f %#+7.2f %#+7.1f",
                    cvGetReal2D(positionMatrix, i, 0),
                    cvGetReal2D(positionMatrix, i, 1),
                    cvGetReal2D(positionMatrix, i, 2),
                    cvGetReal2D(positionMatrix, i, 3));

            // Write the buffer to the image.
            cvPutText(image, buffer, cvPoint(xText, yText), &font, color);
        }
    }
}


