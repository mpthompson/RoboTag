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

    $Id: rvGrid.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include <stdio.h>
#include "cvUtil.h"
#include "rvGrid.h"
#include "rvObject.h"
#include "rvTags384.h"

int rvGrid_OpenCVErrorHandler(int status, const char* func_name, const char* err_msg, const char* file_name, int line )
{
    int i;

    // Do nothing.
    for (i = 0; i < 1; ++i);

    return 1;
}

static void rvGrid_GetCenterPoint(CvPoint2D32f corners[4], CvPoint2D32f *center)
// Fill in the center of the given quadralateral.
{
    CvPoint2D32f t1;
    CvPoint2D32f t2;

    t1.x = corners[1].x + ((corners[2].x - corners[1].x) / 2.0f);
    t1.y = corners[1].y + ((corners[2].y - corners[1].y) / 2.0f);
    t2.x = corners[0].x + ((corners[3].x - corners[0].x) / 2.0f);
    t2.y = corners[0].y + ((corners[3].y - corners[0].y) / 2.0f);

    // Write the grid position into the grid array.
    center->x = t1.x + ((t2.x - t1.x) / 2.0f);
    center->y = t1.y + ((t2.y - t1.y) / 2.0f);
}


static void rvGrid_GetReferencePoints(CvPoint2D32f corners[4], CvPoint2D32f reference[8])
// Fill in the 8 reference points associated with the given quadralateral.  The first
// 4 reference points are outside the quadralateral and the last four reference points
// are inside the quadralateral.
{
    CvPoint2D32f t1;
    CvPoint2D32f t2;

    // Determine line along upper row of rvGrid.
    t1.x = corners[1].x + (((corners[2].x - corners[1].x) * 5.0f) / 20.0f);
    t1.y = corners[1].y + (((corners[2].y - corners[1].y) * 5.0f) / 20.0f);
    t2.x = corners[0].x + (((corners[3].x - corners[0].x) * 5.0f) / 20.0f);
    t2.y = corners[0].y + (((corners[3].y - corners[0].y) * 5.0f) / 20.0f);

    // Set the outside and inside reference points along the line.
    reference[0].x = t1.x + (((t2.x - t1.x) * -1.0f) / 20.0f);
    reference[0].y = t1.y + (((t2.y - t1.y) * -1.0f) / 20.0f);
    reference[4].x = t1.x + (((t2.x - t1.x) * 1.0f) / 20.0f);
    reference[4].y = t1.y + (((t2.y - t1.y) * 1.0f) / 20.0f);
    reference[1].x = t1.x + (((t2.x - t1.x) * 21.0f) / 20.0f);
    reference[1].y = t1.y + (((t2.y - t1.y) * 21.0f) / 20.0f);
    reference[5].x = t1.x + (((t2.x - t1.x) * 19.0f) / 20.0f);
    reference[5].y = t1.y + (((t2.y - t1.y) * 19.0f) / 20.0f);

    // Determine line along lower row of rvGrid.
    t1.x = corners[1].x + (((corners[2].x - corners[1].x) * 15.0f) / 20.0f);
    t1.y = corners[1].y + (((corners[2].y - corners[1].y) * 15.0f) / 20.0f);
    t2.x = corners[0].x + (((corners[3].x - corners[0].x) * 15.0f) / 20.0f);
    t2.y = corners[0].y + (((corners[3].y - corners[0].y) * 15.0f) / 20.0f);

    // Set the outside and inside reference points along the line.
    reference[2].x = t1.x + (((t2.x - t1.x) * -1.0f) / 20.0f);
    reference[2].y = t1.y + (((t2.y - t1.y) * -1.0f) / 20.0f);
    reference[6].x = t1.x + (((t2.x - t1.x) * 1.0f) / 20.0f);
    reference[6].y = t1.y + (((t2.y - t1.y) * 1.0f) / 20.0f);
    reference[3].x = t1.x + (((t2.x - t1.x) * 21.0f) / 20.0f);
    reference[3].y = t1.y + (((t2.y - t1.y) * 21.0f) / 20.0f);
    reference[7].x = t1.x + (((t2.x - t1.x) * 19.0f) / 20.0f);
    reference[7].y = t1.y + (((t2.y - t1.y) * 19.0f) / 20.0f);
}


static void rvGrid_GetSamplePoints(CvPoint2D32f corners[4], CvPoint2D32f samples[64])
// Fill in the 64 sample points points associated with the given quadralateral.
{
    int i;
    int j;
    int k = 0;

    // Loop over the vertical axis of the rvGrid.
    // for (i = 14; i >= 0; i -= 2)
    for (i = 0; i < 16; i += 2)
    {
        float fi = (float) i;
        CvPoint2D32f t1;
        CvPoint2D32f t2;

        t1.x = corners[1].x + (((corners[2].x - corners[1].x) * (3.0f + i)) / 20.0f);
        t1.y = corners[1].y + (((corners[2].y - corners[1].y) * (3.0f + i)) / 20.0f);
        t2.x = corners[0].x + (((corners[3].x - corners[0].x) * (3.0f + i)) / 20.0f);
        t2.y = corners[0].y + (((corners[3].y - corners[0].y) * (3.0f + i)) / 20.0f);

        // Loop over the horizontal axis of the rvGrid.
        // for (j = 0; j < 16; j += 2)
        for (j = 14; j >= 0; j -= 2)
        {
            float fj = (float) j;

            // Write the rvGrid position into the rvGrid array.
            samples[k].x = t1.x + (((t2.x - t1.x) * (3.0f + j)) / 20.0f);
            samples[k].y = t1.y + (((t2.y - t1.y) * (3.0f + j)) / 20.0f);

            // Increment the array index.
            ++k;
        }
    }
}


static int rvGrid_SamplePoint(IplImage *img, CvPoint2D32f point)
// Sample five pixels at and around the indicated point returning the average value.
// This function currently assumes a single plane image.
{
    uchar *pixel;
    uchar *pixel_lt;
    uchar *pixel_rt;
    uchar *pixel_up;
    uchar *pixel_dn;
    int average = 0;
    int x = cvRound(point.x);
    int y = cvRound(point.y);

    // Sanity check the values.
    if (x < 1 || y < 1 || x >= (img->width - 1) || y >= (img->height - 1)) return -1;

    // Get the pixels to be sampled.
    pixel = &((uchar*)(img->imageData + img->widthStep * y))[x];
    pixel_lt = &((uchar*)(img->imageData + img->widthStep * y))[(x - 1)];
    pixel_rt = &((uchar*)(img->imageData + img->widthStep * y))[(x + 1)];
    pixel_up = &((uchar*)(img->imageData + img->widthStep * (y - 1)))[x];
    pixel_dn = &((uchar*)(img->imageData + img->widthStep * (y + 1)))[x];

    // Sample the pixels.
    average += (int) *pixel;
    average += (int) *pixel_lt;
    average += (int) *pixel_rt;
    average += (int) *pixel_up;
    average += (int) *pixel_dn;

    // Return the average sample.
    return average / 5;
}


static int rvGrid_SamplePoints(IplImage *img, CvPoint2D32f *points, int count)
// Sample the array of pixels and return the average value.
{
    int i;
    int value;
    int samples = 0;
    int average = 0;

    // Loop over each pixel to sample.
    for (i = 0; i < count; ++i)
    {
        // Sample the pixel.
        value = rvGrid_SamplePoint(img, points[0]);

        // Was the value obtained?
        if (value != -1)
        {
            // Add to the avarage.
            average += value;

            // Add to the sample count.
            samples += 1;
        }
    }

    // Average the samples.
    if (samples) average /= samples;

    // Return the average sample.
    return average;
}


static bool rvGrid_ProjectPoints(rvGrid *self, CvMat *rotationVector, CvMat *translationVector, CvPoint3D32f* points3d, CvPoint2D32f* points2d, rvUint16 count)
// Use the intrinsic matrix and most recent translation/rotation vectors to project the points.
{
    int i;
    CvMat *matPoints2D;
    CvMat *matPoints3D;

    // Make sure we have points.
    if (count < 1) return false;

    // There seems to be a bug (or feature) in OpenCV where it can't properly project
    // fewer than 3 points.  Therefore we fake it out below by padding zero values.
    if (count < 3)
    {
        // Create the image and object matrices.
        matPoints2D = cvCreateMat(3, 2, CV_64FC1);
        matPoints3D = cvCreateMat(3, 3, CV_64FC1);

        // Place the 3d points into the object points matrix.
        for (i = 0; i < 3; ++i)
        {
            if (i < count)
            {
                cvSetReal2D(matPoints3D, i, 0, points3d[i].x);
                cvSetReal2D(matPoints3D, i, 1, points3d[i].y);
                cvSetReal2D(matPoints3D, i, 2, points3d[i].z);

            }
            else
            {
                cvSetReal2D(matPoints3D, i, 0, 0.0);
                cvSetReal2D(matPoints3D, i, 1, 0.0);
                cvSetReal2D(matPoints3D, i, 2, 0.0);
            }
        }
    }
    else
    {
        // Create the image and object matrices.
        matPoints2D = cvCreateMat(count, 2, CV_64FC1);
        matPoints3D = cvCreateMat(count, 3, CV_64FC1);

        // Place the 3d points into the object points matrix.
        for (i = 0; i < count; ++i)
        {
            cvSetReal2D(matPoints3D, i, 0, points3d[i].x);
            cvSetReal2D(matPoints3D, i, 1, points3d[i].y);
            cvSetReal2D(matPoints3D, i, 2, points3d[i].z);
        }
    }

    // Reproject the object coordinates to image coordinates.
    cvProjectPoints2(matPoints3D, rotationVector, translationVector,
                     self->cameraMatrix, self->distortionCoeffs, matPoints2D,
                     NULL, NULL, NULL, NULL, NULL);

    // Place the image points matrix into the 2d points.
    for (i = 0; i < count; ++i)
    {
        points2d[i].x = (float) cvGetReal2D(matPoints2D, i, 0);
        points2d[i].y = (float) cvGetReal2D(matPoints2D, i, 1);
    }

    // Release the image and object matrices.
    cvReleaseMat(&matPoints2D);
    cvReleaseMat(&matPoints3D);

    return true;
}

static bool rvGrid_AddTag(rvGrid *self, rvUint16 id, CvPoint2D32f corners[RVTAG_CORNER_COUNT])
// Adds a tag id and tag corners to the list of tags to be processed in the rvGrid.
{
    // Is this a navigation tag?
    if (rvTags384_IsNavTag(id))
    {
        // Yes. Add it to the navigation tags list.
        rvGridNavTag *navTag;

        // Prevent tag buffer overflow.
        if (self->navTagCount > RVGRID_MAX_NAV_TAGS) return false;

        // Point to the tag to fill in.
        navTag = &self->navTags[self->navTagCount];

        // Fill in the tag information.
        navTag->id = id;
        navTag->corners[0] = corners[0];
        navTag->corners[1] = corners[1];
        navTag->corners[2] = corners[2];
        navTag->corners[3] = corners[3];

        // Increment the navigation tag count.
        ++self->navTagCount;
    }
    else if (rvTags384_IsObjectTag(id))
    {
        // Yes. Add it to the object tags list.
        rvGridObjTag *objTag;

        // Prevent tag buffer overflow.
        if (self->objTagCount > RVGRID_MAX_OBJ_TAGS) return false;

        // Point to the tag to fill in.
        objTag = &self->objTags[self->objTagCount];

        // Fill in the tag information.
        objTag->id = id;
        objTag->corners[0] = corners[0];
        objTag->corners[1] = corners[1];
        objTag->corners[2] = corners[2];
        objTag->corners[3] = corners[3];

        // Increment the navigation tag count.
        ++self->objTagCount;
    }
    else if (rvTags384_IsCharTag(id))
    {
        // Yes. Add it to the character tags list.
        rvGridCharTag *charTag;

        // Prevent tag buffer overflow.
        if (self->charTagCount > RVGRID_MAX_CHAR_TAGS) return false;

        // Point to the tag to fill in.
        charTag = &self->charTags[self->charTagCount];

        // Fill in the tag information.
        charTag->id = id;
        charTag->corners[0] = corners[0];
        charTag->corners[1] = corners[1];
        charTag->corners[2] = corners[2];
        charTag->corners[3] = corners[3];

        // Increment the character tag count.
        ++self->charTagCount;
    }

    return true;
}

static bool rvGrid_ProjectTag(rvGrid *self, rvUint16 id, CvPoint2D32f corners[RVTAG_CORNER_COUNT])
// Use the intrisic matrix and most recent translation/rotation vectors to project tag points.
{
    CvPoint3D32f tagCorners3d[RVTAG_CORNER_COUNT];

    // Get the position of the corners of the tag in the rvGrid.
    if (!rvTags384_GetCorners(id, tagCorners3d)) return false;

    // Project the 3d corners to 2d image points.
    return rvGrid_ProjectPoints(self, self->rotationVector, self->translationVector, tagCorners3d, corners, 4);
}


static bool rvGrid_DrawNavTags(rvGrid *self, IplImage *img)
{
    rvUint16 i;
    rvGridNavTag *navTag;
    CvPoint2D32f corners[RVTAG_CORNER_COUNT];

    // Make sure we have information.
    if (!self->results) return false;

    // Loop over each navigation tag.
    for (i = 0; i < self->navTagCount; ++i)
    {
        // Point to the navigation tag.
        navTag = &self->navTags[i];

        // Get the tag projection coordinates.
        if (rvGrid_ProjectTag(self, navTag->id, corners))
        {
            // Reproject the navigation tags onto the grid.
            cvDrawCorners(img, corners, CV_RGB(128, 128, 255), CV_RGB(0, 0, 255), 1, 8, 0);
        }
    }

    return true;
}


static bool rvGrid_DrawObject(rvGrid *self, IplImage* img, CvMat *objectVectors, CvMat *positionMatrix)
// Draw the shape defined by the pairs of vectors in the shape matrix.
{
    rvUint16 i;
    rvUint16 objPointCount;
    double identityData[16];
    double rotationData[3];
    double translationData[3];
    CvMat identityMatrix;
    CvMat rotationVector;
    CvMat translationVector;
    CvPoint2D32f *objPoints2d;
    CvPoint3D32f *objPoints3d;

    // How many points do we have to work with.
    objPointCount = (rvUint16) objectVectors->rows;

    // Allocate space on the stack to contain the transformed shape.
    objPoints2d = (CvPoint2D32f *) _alloca(objPointCount * sizeof(CvPoint2D32f));
    objPoints3d = (CvPoint3D32f *) _alloca(objPointCount * sizeof(CvPoint3D32f));

    // Loop over each row in the shape matrix.
    for (i = 0; i < objPointCount; ++i)
    {
        // Multiply each object vector by the position matrix to get the 3D coordinate.
        objPoints3d[i].x = (float) ((cvGetReal2D(positionMatrix, 0, 0) * cvGetReal2D(objectVectors, i, 0)) +
                                    (cvGetReal2D(positionMatrix, 0, 1) * cvGetReal2D(objectVectors, i, 1)) +
                                    (cvGetReal2D(positionMatrix, 0, 2) * cvGetReal2D(objectVectors, i, 2)) +
                                    cvGetReal2D(positionMatrix, 0, 3));
        objPoints3d[i].y = (float) ((cvGetReal2D(positionMatrix, 1, 0) * cvGetReal2D(objectVectors, i, 0)) +
                                    (cvGetReal2D(positionMatrix, 1, 1) * cvGetReal2D(objectVectors, i, 1)) +
                                    (cvGetReal2D(positionMatrix, 1, 2) * cvGetReal2D(objectVectors, i, 2)) +
                                    cvGetReal2D(positionMatrix, 1, 3));
        objPoints3d[i].z = (float) ((cvGetReal2D(positionMatrix, 2, 0) * cvGetReal2D(objectVectors, i, 0)) +
                                    (cvGetReal2D(positionMatrix, 2, 1) * cvGetReal2D(objectVectors, i, 1)) +
                                    (cvGetReal2D(positionMatrix, 2, 2) * cvGetReal2D(objectVectors, i, 2)) +
                                    cvGetReal2D(positionMatrix, 2, 3));
    }

    // Initialize the vector and matrix structures.
    cvInitMatHeader(&identityMatrix, 4, 4, CV_64FC1, &identityData, CV_AUTOSTEP);
    cvInitMatHeader(&rotationVector, 1, 3, CV_64FC1, &rotationData, CV_AUTOSTEP);
    cvInitMatHeader(&translationVector, 1, 3, CV_64FC1, &translationData, CV_AUTOSTEP);

    // Initialize the identity matrix.
    cvSetIdentity(&identityMatrix, cvRealScalar(1));

    // Get the rotation and translation vectors from the identity matrix.
    cvMatrixToVectors(&identityMatrix, &rotationVector, &translationVector);

    // Project the points from 3D space to the 2D image.
    rvGrid_ProjectPoints(self, &rotationVector, &translationVector, objPoints3d, objPoints2d, objPointCount);

    // Draw a line for each pair of points in the object.
    for (i = 0; i < objPointCount; i += 2)
    {
        // Draw the line represented by the pair of points.
        cvDrawLines(img, &objPoints2d[i], 2, CV_RGB(255, 0, 255), 2, 8, 0);
    }

    return true;
}


static bool rvGrid_DrawObjects(rvGrid *self, IplImage *img)
{
    rvUint16 i;
    rvGridObjTag *objTag;
    CvMat objectVectors;

    // Loop over each object tag.
    for (i = 0; i < self->objTagCount; ++i)
    {
        // Point to the object information.
        objTag = &self->objTags[i];

        // Get the object vectors and adjust the position matrix based on the tag position.
        if (rvObject_GetObjectVectors(objTag->id, &objTag->positionMatrix, &objectVectors))
        {
            // Draw the object from the object vectors.
            rvGrid_DrawObject(self, img, &objectVectors, &objTag->positionMatrix);
        }
    }

    return true;
}


static bool rvGrid_DrawCharacters(rvGrid *self, IplImage *img)
// Draw the shape defined by the pairs of vectors in the shape matrix.
{
    rvUint16 i;

    // Loop over each character tag.
    for (i = 0; i < self->charTagCount; ++i)
    {
        int ascii;
        rvGridCharTag *charTag;

        // Point to the character information.
        charTag = &self->charTags[i];

        // Derive the ascii character from the tag id.
        ascii = (int) charTag->id - 4096;

        // Make sure it is something printable.
        if ((ascii >= 32) && (ascii < 127))
        {
            char buffer[16];
            CvPoint2D32f center;

            // Print the rvGrid id into the buffer.
            _snprintf(buffer, sizeof(buffer), "%c", (int) (charTag->id - 4096));

            // Get the coordinates of the center.
            rvGrid_GetCenterPoint(charTag->corners, &center);

            // Write the buffer to the image.
            cvPutText(img, buffer, cvPointFrom32f(center), &self->charFont, CV_RGB(255, 255, 0));
        }
    }

    return true;
}


rvGrid *rvGrid_New(CvSize imageSize, int origin)
// Allocate a new rvGrid object.
{
    int i;
    rvGrid *self = NULL;
    rvTag *tag = NULL;
    IplImage *grayImage = NULL;
    IplImage *edgeImage = NULL;
    CvMemStorage *memStorage = NULL;

    // Set the OpenCV error handler.
    cvRedirectError((CvErrorCallback) rvGrid_OpenCVErrorHandler, NULL, NULL);

    // Allocate a new rvGrid object.
    self = (rvGrid*) malloc(sizeof(rvGrid));

    // Allocate internal objects.
    tag = rvTag_New();
    grayImage = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
    edgeImage = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
    memStorage = cvCreateMemStorage(0);

    // Did we allocate the object.
    if ((self != NULL) && (tag != NULL) && (grayImage != NULL) && (edgeImage != NULL) && (memStorage != NULL))
    {
        // No result yet.
        self->results = false;

        // Set the objects.
        self->tag = tag;

        // Set the images.
        self->imageSize = imageSize;
        self->grayImage = grayImage;
        self->edgeImage = edgeImage;

        // Set the image origins.
        self->grayImage->origin = origin;
        self->edgeImage->origin = origin;

        // Set the memory storage.
        self->memStorage = memStorage;

        // Set the default properties.
        self->display = RVGRID_DISPLAY_COLOR;
        self->edgeMethod = RVGRID_EDGE_ADAPTIVE;
        self->edgeDilation = 0;
        self->gaussianBlur = 0;
        self->adaptiveMethod = RVGRID_ADAPTIVE_METHOD_GAUSSIAN;
        self->adaptiveBlockSize = 45;
        self->adaptiveSubtraction = 5;

        // Set the default draw flags.
        self->drawRawContours = false;
        self->drawPolygonContours = false;
        self->drawQuadContours = false;
        self->drawTagCorners = true;
        self->drawTagReferences = true;
        self->drawTagSamples = true;
        self->drawTagIdentifiers = true;
        self->drawCameraPosition = true;
        self->drawTagReprojection = true;
        self->drawObjectReprojection = true;
        self->drawCharacters = true;

        // Create the matrices.
        self->cameraMatrix = cvCreateMat(3, 3, CV_64FC1);
        self->distortionCoeffs = cvCreateMat(4, 1, CV_64FC1);
        self->rotationVector = cvCreateMat(1, 3, CV_64FC1);
        self->translationVector = cvCreateMat(1, 3, CV_64FC1);
        self->cameraPositionMatrix = cvCreateMat(4, 4, CV_64FC1);

        // Initialize to identity.
        cvSetIdentity(self->cameraMatrix, cvRealScalar(1));
        cvSetIdentity(self->distortionCoeffs, cvRealScalar(1));
        cvSetIdentity(self->rotationVector, cvRealScalar(1));
        cvSetIdentity(self->translationVector, cvRealScalar(1));
        cvSetIdentity(self->cameraPositionMatrix, cvRealScalar(1));

        // Initialize the object and navigation tag counts.
        self->objTagCount = 0;
        self->navTagCount = 0;
        self->charTagCount = 0;

        // Initialize calibration information.
        self->calibrateTagCount = 0;
        self->calibrateImageCount = 0;

        // Initialize each object tag.
        for (i = 0; i < RVGRID_MAX_OBJ_TAGS; ++i)
        {
            // Point to the object tags.
            rvGridObjTag *tag = &self->objTags[i];

            // Initialize the object tag matrices.
            cvInitMatHeader(&tag->rotationVector, 1, 3, CV_64FC1, &tag->rotationData, CV_AUTOSTEP);
            cvInitMatHeader(&tag->translationVector, 1, 3, CV_64FC1, &tag->translationData, CV_AUTOSTEP);
            cvInitMatHeader(&tag->positionMatrix, 4, 4, CV_64FC1, &tag->positionData, CV_AUTOSTEP);
        }

        // Initialize a font for drawing tag ids and characters.
        cvInitFont(&self->idFont, CV_FONT_HERSHEY_PLAIN, 2.0f, 2.0f, 0.0f, 3, 8);
        cvInitFont(&self->charFont, CV_FONT_HERSHEY_PLAIN, 4.0f, 4.0f, 0.0f, 3, 16);
    }
    else
    {
        // Clean up.
        if (tag) rvTag_Free(tag);
        if (grayImage) cvReleaseImage(&grayImage);
        if (edgeImage) cvReleaseImage(&edgeImage);
        if (memStorage != NULL) cvReleaseMemStorage(&memStorage);
        if (self) free(self);

        return NULL;
    }

    return self;
}


void rvGrid_Free(rvGrid *self)
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the matrices.
        cvReleaseMat(&self->cameraMatrix);
        cvReleaseMat(&self->distortionCoeffs);
        cvReleaseMat(&self->rotationVector);
        cvReleaseMat(&self->translationVector);
        cvReleaseMat(&self->cameraPositionMatrix);

        // Free the internal objects.
        rvTag_Free(self->tag);
        cvReleaseImage(&self->grayImage);
        cvReleaseImage(&self->edgeImage);
        cvReleaseMemStorage(&self->memStorage);

        // Free this object.
        free(self);
    }
}


bool rvGrid_ResetIntrinsics(rvGrid *self)
// Reset the camera intrinsics.
{
    // Reset camera and distortion matrix to identity.
    cvSetIdentity(self->cameraMatrix, cvRealScalar(1));
    cvSetIdentity(self->distortionCoeffs, cvRealScalar(1));

    return true;
}


bool rvGrid_LoadIntrinsics(rvGrid *self, const char *filepath)
// Load the camera intrinsics.
{
    CvMat *cameraMatrix;
    CvMat *distortionCoeffs;
    CvFileStorage *fileStore = NULL;

    // Open the file storage.
    fileStore = cvOpenFileStorage(filepath, NULL, CV_STORAGE_READ );

    // Did we open the file storage?
    if (!fileStore) return false;

    // Get the camera matrix and distortion coefficients.  These objects are allocated
    // in temporary storate which is freed when the file store is released.
    cameraMatrix = (CvMat *) cvReadByName(fileStore, NULL, "CameraMatrix", NULL);
    distortionCoeffs = (CvMat *) cvReadByName(fileStore, NULL, "DistortionCoeffs", NULL);

    // Set the camera matrix and distortion coeffs.
    rvGrid_SetCameraMatrix(self, cameraMatrix);
    rvGrid_SetDistortionCoeffs(self, distortionCoeffs);

    // Release the file store.
    cvReleaseFileStorage(&fileStore);

    // We succeeded.
    return true;
}

bool rvGrid_SaveIntrinsics(rvGrid *self, const char *filepath)
// Save the camera intrinsics.
{
    CvMat *cameraMatrix;
    CvMat *distortionCoeffs;
    CvFileStorage *fileStore;

    // Get the camera information.
    rvGrid_GetCameraMatrix(self, &cameraMatrix);
    rvGrid_GetDistortionCoeffs(self, &distortionCoeffs);

    // Open the file store at the indicated path.
    fileStore = cvOpenFileStorage(filepath, 0, CV_STORAGE_WRITE);

    // Did we open the file storage?
    if (!fileStore) return false;

    // Write the camera matrix.
    cvWrite(fileStore, "CameraMatrix", cameraMatrix, cvAttrList(0,0));

    // Write the distortion matrix.
    cvWrite(fileStore, "DistortionCoeffs", distortionCoeffs, cvAttrList(0,0));

    // Close the file store.
    cvReleaseFileStorage(&fileStore);

    // Release the camera information.
    cvReleaseMat(&cameraMatrix);
    cvReleaseMat(&distortionCoeffs);

    // Return success.
    return true;
}

int rvGrid_GetDisplay(rvGrid *self)
{
    return self->display;
}


int rvGrid_GetEdgeMethod(rvGrid *self)
{
    return self->edgeMethod;
}


int rvGrid_GetEdgeDilation(rvGrid *self)
{
    return self->edgeDilation;
}


int rvGrid_GetGaussianBlur(rvGrid *self)
{
    return self->gaussianBlur;
}


int rvGrid_GetAdaptiveMethod(rvGrid *self)
{
    return self->adaptiveMethod;
}


int rvGrid_GetAdaptiveBlockSize(rvGrid *self)
{
    return self->adaptiveBlockSize;
}


int rvGrid_GetAdaptiveSubtraction(rvGrid *self)
{
    return self->adaptiveSubtraction;
}


bool rvGrid_GetDrawRawContours(rvGrid *self)
{
    return self->drawRawContours;
}


bool rvGrid_GetDrawPolygonContours(rvGrid *self)
{
    return self->drawPolygonContours;
}


bool rvGrid_GetDrawQuadContours(rvGrid *self)
{
    return self->drawQuadContours;
}


bool rvGrid_GetDrawTagCorners(rvGrid *self)
{
    return self->drawTagCorners;
}


bool rvGrid_GetDrawTagReferences(rvGrid *self)
{
    return self->drawTagReferences;
}


bool rvGrid_GetDrawTagSamples(rvGrid *self)
{
    return self->drawTagSamples;
}


bool rvGrid_GetDrawTagIdentifiers(rvGrid *self)
{
    return self->drawTagIdentifiers;
}


bool rvGrid_GetDrawCameraPosition(rvGrid *self)
{
    return self->drawCameraPosition;
}


bool rvGrid_GetDrawTagReprojection(rvGrid *self)
{
    return self->drawTagReprojection;
}


bool rvGrid_GetDrawObjectReprojection(rvGrid *self)
{
    return self->drawObjectReprojection;
}


bool rvGrid_GetDrawCharacters(rvGrid *self)
{
    return self->drawCharacters;
}


void rvGrid_SetDisplay(rvGrid *self, int display)
{
    // Sanity check and set the edge method to use when processing images.
    if ((display >= 0) && (display < RVGRID_DISPLAY_COUNT)) self->display = display;
}


void rvGrid_SetEdgeMethod(rvGrid *self, int edgeMethod)
{
    // Sanity check and set the edge method to use when processing images.
    if ((edgeMethod >= 0) && (edgeMethod < RVGRID_EDGE_COUNT)) self->edgeMethod = edgeMethod;
}


void rvGrid_SetEdgeDilation(rvGrid *self, int edgeDilation)
{
    // Sanity check and set the edge dilation value.
    if ((edgeDilation >= 0) && (edgeDilation <= 20)) self->edgeDilation = edgeDilation;
}


void rvGrid_SetGaussianBlur(rvGrid *self, int gaussianBlur)
{
    // Sanity check and set the gaussian blur value to use.
    if ((gaussianBlur >= 0) && (gaussianBlur <= 31)) self->gaussianBlur = gaussianBlur;
}


void rvGrid_SetAdaptiveMethod(rvGrid *self, int adaptiveMethod)
{
    // Sanity check and set the adaptive method to use.
    if ((adaptiveMethod >= 0) && (adaptiveMethod < RVGRID_ADAPTIVE_METHOD_COUNT)) self->adaptiveMethod = adaptiveMethod;
}


void rvGrid_SetAdaptiveBlockSize(rvGrid *self, int adaptiveBlockSize)
{
    // Sanity check and set the adaptive block size value.
    if ((adaptiveBlockSize >= 1) && (adaptiveBlockSize <= 255)) self->adaptiveBlockSize = adaptiveBlockSize;
}


void rvGrid_SetAdaptiveSubtraction(rvGrid *self, int adaptiveSubtraction)
{
    // Sanity check and set the adaptive subtraction value.
    if ((adaptiveSubtraction >= -255) && (adaptiveSubtraction <= 255)) self->adaptiveSubtraction = adaptiveSubtraction;
}


void rvGrid_SetDrawRawContours(rvGrid *self, bool value)
{
    self->drawRawContours = value;
}


void rvGrid_SetDrawPolygonContours(rvGrid *self, bool value)
{
    self->drawPolygonContours = value;
}


void rvGrid_SetDrawQuadContours(rvGrid *self, bool value)
{
    self->drawQuadContours = value;
}


void rvGrid_SetDrawTagCorners(rvGrid *self, bool value)
{
    self->drawTagCorners = value;
}


void rvGrid_SetDrawTagReferences(rvGrid *self, bool value)
{
    self->drawTagReferences = value;
}


void rvGrid_SetDrawTagSamples(rvGrid *self, bool value)
{
    self->drawTagSamples = value;
}


void rvGrid_SetDrawTagIdentifiers(rvGrid *self, bool value)
{
    self->drawTagIdentifiers = value;
}


void rvGrid_SetDrawCameraPosition(rvGrid *self, bool value)
{
    self->drawCameraPosition = value;
}


void rvGrid_SetDrawTagReprojection(rvGrid *self, bool value)
{
    self->drawTagReprojection = value;
}


void rvGrid_SetDrawObjectReprojection(rvGrid *self, bool value)
{
    self->drawObjectReprojection = value;
}


void rvGrid_SetDrawCharacters(rvGrid *self, bool value)
{
    self->drawCharacters = value;
}


bool rvGrid_SetCameraMatrix(rvGrid *self, CvMat *cameraMatrix)
// Set the camera matrix. A local copy of the matrix is made.
{
    // Copy the camera matrix into local matrix.
    cvCopy(cameraMatrix, self->cameraMatrix, NULL);

    return true;
}


bool rvGrid_SetDistortionCoeffs(rvGrid *self, CvMat *distortionCoeffs)
// Set the distortion coefficients matrix. A local copy of the matrix is made.
{
    // Copy the distortion coefficients into local matrix.
    cvCopy(distortionCoeffs, self->distortionCoeffs, NULL);

    return true;
}


bool rvGrid_GetCameraMatrix(rvGrid *self, CvMat **cameraMatrix)
// Get a clone of the camera matrix.  The returned matrix must be freed.
{
    // Clone the camera matrix.
    *cameraMatrix = cvCloneMat(self->cameraMatrix);

    return (*cameraMatrix != NULL);
}


bool rvGrid_GetDistortionCoeffs(rvGrid *self, CvMat **distortionCoeffs)
// Get a clone of the distortion coefficents matrix.  The returned matrix must be freed.
{
    // Clone the distortion coefficients matrix.
    *distortionCoeffs = cvCloneMat(self->distortionCoeffs);

    return (*distortionCoeffs != NULL);
}


bool rvGrid_GetRotationVector(rvGrid *self, CvMat **rotationVector)
// Get a clone of the rotation vector matrix.  The returned matrix must be freed.
{
    // Clone the rotation vector matrix.
    *rotationVector = cvCloneMat(self->rotationVector);

    return true;
}


bool rvGrid_GetTranslationVector(rvGrid *self, CvMat **translationVector)
// Get a clone of the translation vector matrix.  The returned matrix must be freed.
{
    // Clone the translation vector matrix.
    *translationVector = cvCloneMat(self->translationVector);

    return true;
}


bool rvGrid_GetCameraPositionMatrix(rvGrid *self, CvMat **cameraPositionMatrix)
// Get a clone of the position matrix.  The returned matrix must be freed.
{
    // Clone the camera position matrix.
    *cameraPositionMatrix = cvCloneMat(self->cameraPositionMatrix);

    return true;
}


bool rvGrid_CameraPosition(rvGrid *self)
// Calculates the position from the current set of tag information.
{
    int i;
    CvMat tagPoints2d;
    CvMat tagPoints3d;
    CvMat extrinsicMatrix;
    double *tagPoints2dData;
    double *tagPoints3dData;
    double extrinsicData[16];

    // Assume we failed.
    self->results = false;

    // Make sure we have some navigation tags to process.
    if (self->navTagCount == 0) return false;

    // Allocate the data for the 2D and 3D points on the stack.
    tagPoints2dData = (double *) _alloca(sizeof(double) * self->navTagCount * 4 * 2);
    tagPoints3dData = (double *) _alloca(sizeof(double) * self->navTagCount * 4 * 3);

    // Initialize the matrices used by this function.
    cvInitMatHeader(&tagPoints2d, self->navTagCount * 4, 2, CV_64FC1, tagPoints2dData, CV_AUTOSTEP);
    cvInitMatHeader(&tagPoints3d, self->navTagCount * 4, 3, CV_64FC1, tagPoints3dData, CV_AUTOSTEP);
    cvInitMatHeader(&extrinsicMatrix, 4, 4, CV_64FC1, extrinsicData, CV_AUTOSTEP);

    // Set the position of each corner in the image.
    for (i = 0; i < self->navTagCount; ++i)
    {
        rvGridNavTag *navTag;
        CvPoint3D32f corners[RVTAG_CORNER_COUNT];

        // Point to the indexed navigation tag information.
        navTag = &self->navTags[i];

        // Set the 2D position of the tag in the image.
        cvSetReal2D(&tagPoints2d, (i << 2), 0, navTag->corners[0].x);
        cvSetReal2D(&tagPoints2d, (i << 2), 1, navTag->corners[0].y);
        cvSetReal2D(&tagPoints2d, (i << 2) + 1, 0, navTag->corners[1].x);
        cvSetReal2D(&tagPoints2d, (i << 2) + 1, 1, navTag->corners[1].y);
        cvSetReal2D(&tagPoints2d, (i << 2) + 2, 0, navTag->corners[2].x);
        cvSetReal2D(&tagPoints2d, (i << 2) + 2, 1, navTag->corners[2].y);
        cvSetReal2D(&tagPoints2d, (i << 2) + 3, 0, navTag->corners[3].x);
        cvSetReal2D(&tagPoints2d, (i << 2) + 3, 1, navTag->corners[3].y);

        // Get the 3D position of the corners of the tag in the grid.
        rvTags384_GetCorners(navTag->id, corners);

        // Set the 3D position of the tag corner on the grid.
        cvSetReal2D(&tagPoints3d, (i << 2), 0, corners[0].x);
        cvSetReal2D(&tagPoints3d, (i << 2), 1, corners[0].y);
        cvSetReal2D(&tagPoints3d, (i << 2), 2, corners[0].z);
        cvSetReal2D(&tagPoints3d, (i << 2) + 1, 0, corners[1].x);
        cvSetReal2D(&tagPoints3d, (i << 2) + 1, 1, corners[1].y);
        cvSetReal2D(&tagPoints3d, (i << 2) + 1, 2, corners[1].z);
        cvSetReal2D(&tagPoints3d, (i << 2) + 2, 0, corners[2].x);
        cvSetReal2D(&tagPoints3d, (i << 2) + 2, 1, corners[2].y);
        cvSetReal2D(&tagPoints3d, (i << 2) + 2, 2, corners[2].z);
        cvSetReal2D(&tagPoints3d, (i << 2) + 3, 0, corners[3].x);
        cvSetReal2D(&tagPoints3d, (i << 2) + 3, 1, corners[3].y);
        cvSetReal2D(&tagPoints3d, (i << 2) + 3, 2, corners[3].z);
    }

    // Initialize the translation and rotation vectors to identity.
    cvSetIdentity(self->rotationVector, cvRealScalar(1));
    cvSetIdentity(self->translationVector, cvRealScalar(1));

    // Find the extrinsic camera parameters for the particular view.
    // The rotation and translation vectors are filled in by this function.
    cvFindExtrinsicCameraParams2(&tagPoints3d, &tagPoints2d,
                                 self->cameraMatrix, self->distortionCoeffs,
                                 self->rotationVector, self->translationVector);

    // Convert the output vectors into matrix form to create the extrinsic matrix.
    cvVectorsToMatrix(self->rotationVector, self->translationVector, &extrinsicMatrix);

    // Invert the extrinsic matrix to get the camera position matrix.
    cvInvert(&extrinsicMatrix, self->cameraPositionMatrix, CV_LU);

    // We succeeded.
    self->results = true;

    return self->results;
}


bool rvGrid_ObjectPositions(rvGrid *self)
// Calculates the position of objects.
{
    int i;
    CvMat tagPoints2d;
    CvMat tagPoints3d;
    double tagPoints2dData[8];
    double tagPoints3dData[12];

    // Make sure we have some object tags to process.
    if (self->objTagCount == 0) return false;

    // Initialize the matrices used by this function.
    cvInitMatHeader(&tagPoints2d, 4, 2, CV_64FC1, tagPoints2dData, CV_AUTOSTEP);
    cvInitMatHeader(&tagPoints3d, 4, 3, CV_64FC1, tagPoints3dData, CV_AUTOSTEP);

    // Loop over each object and calculate its relative position.
    for (i = 0; i < self->objTagCount; ++i)
    {
        rvGridObjTag *objTag;
        CvPoint3D32f corners[RVTAG_CORNER_COUNT];

        // Point to the indexed object tag information.
        objTag = &self->objTags[i];

        // Set the 2D position of the tag in the image.
        cvSetReal2D(&tagPoints2d, 0, 0, objTag->corners[0].x);
        cvSetReal2D(&tagPoints2d, 0, 1, objTag->corners[0].y);
        cvSetReal2D(&tagPoints2d, 1, 0, objTag->corners[1].x);
        cvSetReal2D(&tagPoints2d, 1, 1, objTag->corners[1].y);
        cvSetReal2D(&tagPoints2d, 2, 0, objTag->corners[2].x);
        cvSetReal2D(&tagPoints2d, 2, 1, objTag->corners[2].y);
        cvSetReal2D(&tagPoints2d, 3, 0, objTag->corners[3].x);
        cvSetReal2D(&tagPoints2d, 3, 1, objTag->corners[3].y);

        // Get the 3D position of the corners of the tag in the grid.
        rvTags384_GetCorners(objTag->id, corners);

        // Set the 3D position of the tag corner on the grid.
        cvSetReal2D(&tagPoints3d, 0, 0, corners[0].x);
        cvSetReal2D(&tagPoints3d, 0, 1, corners[0].y);
        cvSetReal2D(&tagPoints3d, 0, 2, corners[0].z);
        cvSetReal2D(&tagPoints3d, 1, 0, corners[1].x);
        cvSetReal2D(&tagPoints3d, 1, 1, corners[1].y);
        cvSetReal2D(&tagPoints3d, 1, 2, corners[1].z);
        cvSetReal2D(&tagPoints3d, 2, 0, corners[2].x);
        cvSetReal2D(&tagPoints3d, 2, 1, corners[2].y);
        cvSetReal2D(&tagPoints3d, 2, 2, corners[2].z);
        cvSetReal2D(&tagPoints3d, 3, 0, corners[3].x);
        cvSetReal2D(&tagPoints3d, 3, 1, corners[3].y);
        cvSetReal2D(&tagPoints3d, 3, 2, corners[3].z);

        // Initialize the translation and rotation vectors to identity.
        cvSetIdentity(&objTag->rotationVector, cvRealScalar(1));
        cvSetIdentity(&objTag->translationVector, cvRealScalar(1));

        // Find the extrinsic camera parameters for the particular view.
        // The rotation and translation vectors are filled in by this function.
        cvFindExtrinsicCameraParams2(&tagPoints3d, &tagPoints2d,
                                     self->cameraMatrix, self->distortionCoeffs,
                                     &objTag->rotationVector, &objTag->translationVector);

        // Convert the output vectors into matrix form to create the extrinsic matrix.
        cvVectorsToMatrix(&objTag->rotationVector, &objTag->translationVector, &objTag->positionMatrix);
    }

    return true;
}


bool rvGrid_ProcessImage(rvGrid *self, IplImage *image)
// Process the indicated image to obtain the position.
{
    int blur;
    CvSeq *contours = NULL;
    CvPoint2D32f reference[8];
    CvPoint2D32f samples[RVTAG_SAMPLE_COUNT];
    CvPoint2D32f corners[RVTAG_CORNER_COUNT];
    rvUint8 tagSamples[RVTAG_SAMPLE_COUNT];
    bool rv = false;

    // Clear the memory storage.
    cvClearMemStorage(self->memStorage);

    // Convert the image to gray scale.
    cvCvtColor(image, self->grayImage, CV_RGB2GRAY);

    // Adjust the blur to prevent passing in an even number.  If the number is
    // not zero and even, the number is rounded down the previous negative number.
    blur = self->gaussianBlur < 1 ? 0 : (((self->gaussianBlur - 1) / 2) * 2) + 1;

    // Smooth the gray scale image.
    if (blur) cvSmooth(self->grayImage, self->grayImage, CV_GAUSSIAN, blur, 0, 0.0, 0.0);

    // Should we write the gray image back?
    if (self->display == RVGRID_DISPLAY_GRAY) cvCvtColor(self->grayImage, image, CV_GRAY2RGB);

    // Handle the edge method for creating contours.
    if (self->edgeMethod == RVGRID_EDGE_CANNY)
    {
        // Apply the Canny algorithm for edge detection.
        cvCanny(self->grayImage, self->edgeImage, 50, 200, 3);

        // Dialate the edge output to remove holes between edge segments.
        if (self->edgeDilation) cvDilate(self->edgeImage, self->edgeImage, NULL, self->edgeDilation);
    }
    else if (self->edgeMethod == RVGRID_EDGE_SUZAN)
    {
        // Apply the Suzan algorithm for edge detection.
        cvSusan(self->grayImage, self->edgeImage, 10, 1);

        // Dialate the edge output to remove holes between edge segments.
        if (self->edgeDilation) cvDilate(self->edgeImage, self->edgeImage, NULL, self->edgeDilation);
    }
    else  // Default is adaptive threshold.
    {
        // Adjust the block size to prevent passing in an even number.  If the number is
        // even, the number is rounded down the previous negative number.
        int blockSize = self->adaptiveBlockSize <= 1 ? 1 : (((self->adaptiveBlockSize - 1) / 2) * 2) + 1;

        // Apply the adaptive threshold algorithm for edge detection.
        cvAdaptiveThreshold(self->grayImage, self->edgeImage, 255.0,
                            !self->adaptiveMethod ? CV_ADAPTIVE_THRESH_MEAN_C : CV_ADAPTIVE_THRESH_GAUSSIAN_C,
                            CV_THRESH_BINARY, blockSize, (double) self->adaptiveSubtraction);
    }

    // Should we write the edge image back?
    if (self->display == RVGRID_DISPLAY_EDGE) cvCvtColor(self->edgeImage, image, CV_GRAY2RGB);

    // Convert the edges in the edge image to a sequence of contours.
    cvFindContours(self->edgeImage, self->memStorage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

    // Reset the position results.
    self->results = false;

    // Reset the object and navigation tag count.
    self->objTagCount = 0;
    self->navTagCount = 0;
    self->charTagCount = 0;

    // Loop over all the countours.
    while (contours)
    {
        int i;
        int whiteReference;
        int blackReference;
        CvSeq *result;

        // Draw the contours found in the image.
        // if (self->drawRawContours) cvDrawContourArrows(image, contours, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, 8, 0);
        if (self->drawRawContours) cvDrawContours(image, contours, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 0, 2, 8, cvPoint(0,0));

        // Approximates polygonal curve with precision proportional to the contour perimeter.
        result = cvApproxPoly(contours, sizeof(CvContour), self->memStorage, CV_POLY_APPROX_DP, cvArcLength(contours, CV_WHOLE_SEQ, 1) * 0.02, 0);

        // Draw the polygons within the image.
        // if (self->drawPolygonContours) cvDrawContourArrows(image, result, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, 8, 0);
        if (self->drawPolygonContours) cvDrawContours(image, result, CV_RGB(0, 255, 0), CV_RGB(0, 255, 0), 0, 2, 8, cvPoint(0,0));

        // Square contours should have:
        //
        //   o 4 vertices after approximation
        //   o relatively large area (to filter out noisy contours)
        //   o and be convex
        //
        // Note: Absolute value of an area is used because area may be positive or
        // negative - in accordance with the contour orientation.
        if ((result->total == 4) && cvCheckContourConvexity(result) && (fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 500))
        {
            // Draw the four sided polygons within the image.
            // if (self->drawQuadContours) cvDrawContourArrows(image, result, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 1, 8, 0);
            if (self->drawQuadContours) cvDrawContours(image, result, CV_RGB(0, 255, 0), CV_RGB(0, 255, 0), 0, 2, 8, cvPoint(0,0));

            // Convert the polygon to an array of corners.
            corners[0] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 0)));
            corners[1] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 1)));
            corners[2] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 2)));
            corners[3] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 3)));

            // Refine the corner coordinates to sub-pixel values.
            cvFindCornerSubPix(self->grayImage, &corners[0], 4, cvSize(5, 5), cvSize(-1, -1),
                               cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 5, 0.2f));

            // The polygon may be going in a counter-clockwise direction which will
            // defeat encoding.  Normalize the polygon to follow a clockwise direction.
            cvNormalizeCorners(corners);

            // Get the reference points.
            rvGrid_GetReferencePoints(corners, reference);

            // Get the black and white pixel values from the reference points.
            whiteReference = rvGrid_SamplePoints(self->grayImage, &reference[0], 4);
            blackReference = rvGrid_SamplePoints(self->grayImage, &reference[4], 4);

            // Our markers consist of a black border against a white background. If the black
            // reference is not less bright than the white reference we can skip the tag.
            if (blackReference < whiteReference)
            {
                // Should we draw sample reference points?
                if (self->drawTagReferences)
                {
                    // Draw sample reference points.
                    cvDrawCrosses(image, &reference[0], 4, CV_RGB(255, 0, 0));
                    cvDrawCrosses(image, &reference[4], 4, CV_RGB(0, 255, 0));
                }

                // Get the sample points.
                rvGrid_GetSamplePoints(corners, samples);

                // Sample each point.
                for (i = 0; i < RVTAG_SAMPLE_COUNT; ++i)
                {
                    int gridSample;
                    int blackDifference;
                    int whiteDifference;
                    CvPoint pt;

                    // Sample the point.
                    gridSample = rvGrid_SamplePoint(self->grayImage, samples[i]);

                    // Get the black and white differences.
                    blackDifference = gridSample - blackReference;
                    whiteDifference = gridSample - whiteReference;
                    if (blackDifference < 0) blackDifference = -blackDifference;
                    if (whiteDifference < 0) whiteDifference = -whiteDifference;

                    pt.x = cvRound(samples[i].x);
                    pt.y = cvRound(samples[i].y);

                    if (whiteDifference < blackDifference)
                    {
                        // We found a white square.
                        tagSamples[i] = 1;

                        // Draw the red cross.
                        if (self->drawTagSamples) cvDrawCross(image, pt, CV_RGB(255, 0, 0));
                    }
                    else
                    {
                        // We found a black square.
                        tagSamples[i] = 0;

                        // Draw the green cross.
                        if (self->drawTagSamples) cvDrawCross(image, pt, CV_RGB(0, 255, 0));
                    }
                }

                // Decode the bits and see if we found a valid pattern.
                if (rvTag_DecodeSamples(self->tag, corners, tagSamples))
                {
                    rvUint16 tag_id;

                    // Get the decoded tag id.
                    rvTag_GetDecodedId(self->tag, &tag_id);

                    // Get the decoded tag corners.  These are the 2D positions of
                    // corners of the tag within the image.
                    rvTag_GetDecodedCorners(self->tag, corners);

                    // Place the tag id and tag corners in the rvGrid object.
                    rvGrid_AddTag(self, tag_id, corners);

                    // Draw the corners of the tag.
                    if (self->drawTagCorners) cvDrawCorners(image, corners, CV_RGB(0, 255, 0), CV_RGB(255, 0, 0), 1, 8, 0);

                    // Draw the identifiers of the tag.
                    if (self->drawTagIdentifiers)
                    {
                        char buffer[16];
                        CvPoint2D32f center;

                        // Print the rvGrid id into the buffer.
                        _snprintf(buffer, sizeof(buffer), "%d", (int) tag_id);

                        // Get the coordinates of the center.
                        rvGrid_GetCenterPoint(corners, &center);

                        // Write the buffer to the image.
                        cvPutText(image, buffer, cvPointFrom32f(center), &self->idFont, CV_RGB(255, 0, 0));
                    }
                }
            }
        }

        // Get the next contour.
        contours = contours->h_next;
    }

    // Determine the camera position relative to the navigation tags.
    if (rvGrid_CameraPosition(self))
    {
        // Reproject the navigation tags.
        if (self->drawTagReprojection) rvGrid_DrawNavTags(self, image);

        // Should we draw the camera position?
        if (self->drawCameraPosition)
        {
            CvMat *positionMatrix;

            // Get the position matrix.
            rvGrid_GetCameraPositionMatrix(self, &positionMatrix);

            // Print the position matrix.
            cvDrawPositionInfo(image, positionMatrix, true);

            // Release the position matrix.
            cvReleaseMat(&positionMatrix);
        }

        // We succeeded.
        rv = true;
    }

    // Determine the object positions relative to the camera.
    if (rvGrid_ObjectPositions(self))
    {
        // Should we reproject the objects?
        if (self->drawObjectReprojection)
        {
            // Draw the objects.
            rvGrid_DrawObjects(self, image);
        }

        // We succeeded.
        rv = true;
    }

    // Draw any characters.
    if (self->drawCharacters) rvGrid_DrawCharacters(self, image);

    return rv;
}


// Add the last successful set of navigation tags to the set of tags accumulated
// for calibration purposes.
bool rvGrid_CalibrateAdd(rvGrid *self)
{
    rvUint16 i;
    rvGridNavTag *navTag;

    // Make sure we have information.
    if (!self->results) return false;
    if (self->navTagCount == 0) return false;

    // Make sure we don't overflow the calibration image buffer.
    if (self->calibrateImageCount >= RVGRID_MAX_CALIBRATE_IMAGES) return false;

    // Reset the count of tags in this images.
    self->calibrateImages[self->calibrateImageCount] = 0;

    // Loop over each navigation tag.
    for (i = 0; i < self->navTagCount; ++i)
    {
        // Make sure we don't overflow the calibration tag buffer.
        if (self->calibrateTagCount >= RVGRID_MAX_CALIBRATE_TAGS) break;

        // Point to the navigation tag.
        navTag = &self->navTags[i];

        // Add the tag id to the list of tag ids.
        self->calibrateTags[self->calibrateTagCount] = navTag->id;

        // Add the corners to the list of corners.
        self->calibrateCorners[(self->calibrateTagCount << 2)] = navTag->corners[0];
        self->calibrateCorners[(self->calibrateTagCount << 2) + 1] = navTag->corners[1];
        self->calibrateCorners[(self->calibrateTagCount << 2) + 2] = navTag->corners[2];
        self->calibrateCorners[(self->calibrateTagCount << 2) + 3] = navTag->corners[3];

        // Increment the tag count.
        ++self->calibrateTagCount;

        // Increment the count of tags in this image.
        ++self->calibrateImages[self->calibrateImageCount];
    }

    // We processed another set of image tags.
    ++self->calibrateImageCount;

    return true;
}


bool rvGrid_CalibrateReset(rvGrid *self)
{
    // Reset the calibrate tag and image counts.
    self->calibrateTagCount = 0;
    self->calibrateImageCount = 0;

    return true;
}


int rvGrid_GetCalibrateTagCount(rvGrid *self)
{
    return (int) self->calibrateTagCount;
}


int rvGrid_GetCalibrateImageCount(rvGrid *self)
{
    return (int) self->calibrateImageCount;
}


// Determine the camera matrix and distortion matrix from the set of tags
// accumulated so far for callibration purposes.
bool rvGrid_Calibrate(rvGrid *self)
{
    int i;
    CvMat *imageCounts;
    CvMat *imagePoints2D;
    CvMat *imagePoints3D;

    // We must have tags and views.
    if (!self->calibrateTagCount || !self->calibrateImageCount) return false;

    // Create the matrices that count the 2D and 3D points found in each image.
    imageCounts = cvCreateMat(self->calibrateImageCount, 1, CV_32SC1);
    imagePoints2D = cvCreateMat(self->calibrateTagCount << 2, 2, CV_64FC1);
    imagePoints3D = cvCreateMat(self->calibrateTagCount << 2, 3, CV_64FC1);

    // Set the position of each corner across all images.
    for (i = 0; i < self->calibrateTagCount; ++i)
    {
        cvSetReal2D(imagePoints2D, (i << 2), 0, self->calibrateCorners[(i << 2)].x);
        cvSetReal2D(imagePoints2D, (i << 2), 1, self->calibrateCorners[(i << 2)].y);
        cvSetReal2D(imagePoints2D, (i << 2) + 1, 0, self->calibrateCorners[(i << 2) + 1].x);
        cvSetReal2D(imagePoints2D, (i << 2) + 1, 1, self->calibrateCorners[(i << 2) + 1].y);
        cvSetReal2D(imagePoints2D, (i << 2) + 2, 0, self->calibrateCorners[(i << 2) + 2].x);
        cvSetReal2D(imagePoints2D, (i << 2) + 2, 1, self->calibrateCorners[(i << 2) + 2].y);
        cvSetReal2D(imagePoints2D, (i << 2) + 3, 0, self->calibrateCorners[(i << 2) + 3].x);
        cvSetReal2D(imagePoints2D, (i << 2) + 3, 1, self->calibrateCorners[(i << 2) + 3].y);
    }

    // Set the 3D position of each tag corner on the grid.
    for (i = 0; i < self->calibrateTagCount; ++i)
    {
        CvPoint3D32f corners[RVTAG_CORNER_COUNT];

        // Get the position of the corners of the tag in the grid.
        rvTags384_GetCorners(self->calibrateTags[i], corners);

        // Set the 3D position of the tag corner on the grid.
        cvSetReal2D(imagePoints3D, (i << 2), 0, corners[0].x);
        cvSetReal2D(imagePoints3D, (i << 2), 1, corners[0].y);
        cvSetReal2D(imagePoints3D, (i << 2), 2, 0.0);
        cvSetReal2D(imagePoints3D, (i << 2) + 1, 0, corners[1].x);
        cvSetReal2D(imagePoints3D, (i << 2) + 1, 1, corners[1].y);
        cvSetReal2D(imagePoints3D, (i << 2) + 1, 2, 0.0);
        cvSetReal2D(imagePoints3D, (i << 2) + 2, 0, corners[2].x);
        cvSetReal2D(imagePoints3D, (i << 2) + 2, 1, corners[2].y);
        cvSetReal2D(imagePoints3D, (i << 2) + 2, 2, 0.0);
        cvSetReal2D(imagePoints3D, (i << 2) + 3, 0, corners[3].x);
        cvSetReal2D(imagePoints3D, (i << 2) + 3, 1, corners[3].y);
        cvSetReal2D(imagePoints3D, (i << 2) + 3, 2, 0.0);
    }

    // Set the count of points in each image.
    for (i = 0; i < self->calibrateImageCount; ++i)
    {
        // Set the count of points in this image.
        cvSet1D(imageCounts, i, cvScalar((double) (self->calibrateImages[i] << 2), 0, 0, 0));
    }

    // Calibrate the camera based on the point information in the images.
    cvCalibrateCamera2(imagePoints3D, imagePoints2D, imageCounts, self->imageSize,
                       self->cameraMatrix, self->distortionCoeffs, NULL, NULL, 0);

    // Release the matrices.
    cvReleaseMat(&imageCounts);
    cvReleaseMat(&imagePoints2D);
    cvReleaseMat(&imagePoints3D);

    // Reset the tag and view count.
    self->calibrateTagCount = 0;
    self->calibrateImageCount = 0;

    return true;
}
