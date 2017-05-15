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

    $Id: rvCalibrate.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include <stdio.h>
#include "cvUtil.h"
#include "rvCalibrate.h"
#include "rvTags384.h"

static void rvCalibrate_GetCenterPoint(CvPoint2D32f corners[4], CvPoint2D32f *center)
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


static void rvCalibrate_GetReferencePoints(CvPoint2D32f corners[4], CvPoint2D32f reference[8])
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


static void rvCalibrate_GetSamplePoints(CvPoint2D32f corners[4], CvPoint2D32f samples[64])
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


static int rvCalibrate_SamplePoint(IplImage *img, CvPoint2D32f point)
// Sample five pixels at and around the indicated point returning the average value.
// This function currently assumes a single plane image.
{
    uchar *pixel;
    uchar *pixelLt;
    uchar *pixelRt;
    uchar *pixelUp;
    uchar *pixelDn;
    int average = 0;
    int x = cvRound(point.x);
    int y = cvRound(point.y);

    // Sanity check the values.
    if (x < 1 || y < 1 || x >= (img->width - 1) || y >= (img->height - 1)) return -1;

    // Get the pixels to be sampled.
    pixel = &((uchar*)(img->imageData + img->widthStep * y))[x];
    pixelLt = &((uchar*)(img->imageData + img->widthStep * y))[(x - 1)];
    pixelRt = &((uchar*)(img->imageData + img->widthStep * y))[(x + 1)];
    pixelUp = &((uchar*)(img->imageData + img->widthStep * (y - 1)))[x];
    pixelDn = &((uchar*)(img->imageData + img->widthStep * (y + 1)))[x];

    // Sample the pixels.
    average += (int) *pixel;
    average += (int) *pixelLt;
    average += (int) *pixelRt;
    average += (int) *pixelUp;
    average += (int) *pixelDn;

    // Return the average sample.
    return average / 5;
}


static int rvCalibrate_SamplePoints(IplImage *img, CvPoint2D32f *points, int count)
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
        value = rvCalibrate_SamplePoint(img, points[0]);

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


rvCalibrate *rvCalibrate_New(CvSize imageSize, int origin)
// Allocate a new rvCalibrate object.
{
    rvCalibrate *self = NULL;
    rvTag* tag = NULL;
    IplImage *grayImage = NULL;
    IplImage *edgeImage = NULL;
    CvMemStorage *memStorage = NULL;

    // Allocate a new calibrate object.
    self = (rvCalibrate*) malloc(sizeof(rvCalibrate));

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
        self->grayImage = grayImage;
        self->edgeImage = edgeImage;

        // Set the image origins.
        self->grayImage->origin = origin;
        self->edgeImage->origin = origin;

        // Set the memory storage.
        self->memStorage = memStorage;

        // Create the matrices.
        self->cameraMatrix = cvCreateMat(3, 3, CV_64FC1);
        self->distortionCoeffs = cvCreateMat(4, 1, CV_64FC1);

        // Initialize to identity.
        cvSetIdentity(self->cameraMatrix, cvRealScalar(1));
        cvSetIdentity(self->distortionCoeffs, cvRealScalar(1));

        // Save the image size.
        self->imageSize = imageSize;

        // Reset the tag and view counts.
        self->tagCount = 0;
        self->viewCount = 0;

#ifdef _DEBUG
        // Initialize a font.
        cvInitFont(&self->font, CV_FONT_HERSHEY_PLAIN, 2.0f, 2.0f, 0.0f, 3, 8);
#endif
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


void rvCalibrate_Free(rvCalibrate *self)
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the matrices.
        cvReleaseMat(&self->cameraMatrix);
        cvReleaseMat(&self->distortionCoeffs);

        // Free the internal objects.
        rvTag_Free(self->tag);
        cvReleaseImage(&self->grayImage);
        cvReleaseImage(&self->edgeImage);
        cvReleaseMemStorage(&self->memStorage);

        // Free this object.
        free(self);
    }
}


bool rvCalibrate_SaveIntrinsics(rvCalibrate *self, const char *filepath)
// Save the camera intrinsics.
{
    CvMat *cameraMatrix;
    CvMat *distortionCoeffs;
    CvFileStorage *fileStore;

    // Get the camera information.
    rvCalibrate_GetCameraMatrix(self, &cameraMatrix);
    rvCalibrate_GetDistortionCoeffs(self, &distortionCoeffs);

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


bool rvCalibrate_GetCameraMatrix(rvCalibrate *self, CvMat **cameraMatrix)
// Get a clone of the camera matrix.  The returned matrix must be freed.
{
    // Clone the camera matrix.
    *cameraMatrix = cvCloneMat(self->cameraMatrix);

    return (*cameraMatrix != NULL);
}


bool rvCalibrate_GetDistortionCoeffs(rvCalibrate *self, CvMat **distortionCoeffs)
// Get a clone of the distortion coefficents matrix.  The returned matrix must be freed.
{
    // Clone the distortion coefficients matrix.
    *distortionCoeffs = cvCloneMat(self->distortionCoeffs);

    return (*distortionCoeffs != NULL);
}


bool rvCalibrate_Reset(rvCalibrate *self)
// Reset the calibration information.
{
    // Reset the tag and view counts.
    self->tagCount = 0;
    self->viewCount = 0;

    // Reset to identity.
    cvSetIdentity(self->cameraMatrix, cvRealScalar(1));
    cvSetIdentity(self->distortionCoeffs, cvRealScalar(1));

    return 1;
}


bool rvCalibrate_GetTagCount(rvCalibrate *self, rvUint16* count)
{
    *count = self->tagCount;

    return true;
}


bool rvCalibrate_GetViewCount(rvCalibrate *self, rvUint16* count)
{
    *count = self->viewCount;

    return true;
}


bool rvCalibrate_AddView(rvCalibrate *self, IplImage *image, bool add)
{
    CvSeq* contours = NULL;
    CvPoint2D32f reference[8];
    CvPoint2D32f samples[RVTAG_SAMPLE_COUNT];
    CvPoint2D32f corners[RVTAG_CORNER_COUNT];
    rvUint8 tagSamples[RVTAG_SAMPLE_COUNT];

    // Sanity check the view count.
    if (self->viewCount >= RVCALIBRATE_MAX_VIEWS) return false;

    // Reset the count of tags in this view.
    self->views[self->viewCount] = 0;

    // Clear the memory storage.
    cvClearMemStorage(self->memStorage);

    // Convert the image to gray scale.
    cvCvtColor(image, self->grayImage, CV_RGB2GRAY);

    // Apply the Susan algorithm for edge detection.
    cvSusan(self->grayImage, self->edgeImage, 10, 1);

    // Dialate the edge output to remove holes between edge segments.
    cvDilate(self->edgeImage, self->edgeImage, 0, 1);

    // Get the lines from the edge image.
    cvFindContours(self->edgeImage, self->memStorage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

    // Draw the contours found in the image as red arrows.
    // XXX cvDrawContourArrows(image, contours, CV_RGB(255, 0, 0), 1, 8);

    // Loop over all the countours.
    while (contours)
    {
        int i;
        int whiteReference;
        int blackReference;

        // Approximates polygonal curve with precision proportional to the contour perimeter.
        CvSeq *result = cvApproxPoly(contours, sizeof(CvContour), self->memStorage, CV_POLY_APPROX_DP, cvArcLength(contours, CV_WHOLE_SEQ, 1) * 0.02, 0);

        // Draw the contours found in the image as red arrows.
        // XXX cvDrawContourArrows(image, result, CV_RGB(255, 0, 0), 1, 8);
        // XXX cvDrawContours(image, result, CV_RGB(255, 0, 0), CV_RGB(0, 0, 255), 0, 1, 8);

        // Square contours should have:
        //
        //   o 4 vertices after approximation
        //   o relatively large area (to filter out noisy contours)
        //   o and be convex
        //
        // Note: Absolute value of an area is used because area may be positive or
        // negative - in accordance with the contour orientation.
        if ((result->total == 4) && (fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 1000) && cvCheckContourConvexity(result))
        {
            // Draw the contours found in the image as red arrows.
            // XXX cvDrawContourArrows(image, result, CV_RGB(255, 0, 0), 1, 8);

            // Get each grid point.
            corners[0] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 0)));
            corners[1] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 1)));
            corners[2] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 2)));
            corners[3] = cvPointTo32f(*((CvPoint*) cvGetSeqElem(result, 3)));

            // Refine the corner coordinates to sub-pixel values.
            cvFindCornerSubPix(self->grayImage, &corners[0], 4, cvSize(5, 5), cvSize(-1, -1),
                                cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 5, 0.2f));

            // Get the reference points.
            rvCalibrate_GetReferencePoints(corners, reference);

            // Get the black and white pixel values from the reference points.
            whiteReference = rvCalibrate_SamplePoints(self->grayImage, &reference[0], 4);
            blackReference = rvCalibrate_SamplePoints(self->grayImage, &reference[4], 4);

            // Draw the reference points.
            cvDrawCrosses(image, &reference[0], 4, CV_RGB(255, 0, 0));
            cvDrawCrosses(image, &reference[4], 4, CV_RGB(0, 255, 0));

            // Get the sample points.
            rvCalibrate_GetSamplePoints(corners, samples);

            // Sample each point.
            for (i = 0; i < RVTAG_SAMPLE_COUNT; ++i)
            {
                int gridSample;
                int blackDifference;
                int whiteDifference;
                CvPoint pt;

                // Sample the point.
                gridSample = rvCalibrate_SamplePoint(self->grayImage, samples[i]);

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
                    cvDrawCross(image, pt, CV_RGB(255, 0, 0));
                }
                else
                {
                    // We found a black square.
                    tagSamples[i] = 0;

                    // Draw the green cross.
                    cvDrawCross(image, pt, CV_RGB(0, 255, 0));
                }
            }

            // Decode the bits and see if we found a valid pattern.
            if (rvTag_DecodeSamples(self->tag, corners, tagSamples))
            {
                // Make sure we can add another tag.
                if (self->tagCount < RVCALIBRATE_MAX_TAGS)
                {
                    rvUint16 tagId;

                    // Get the decoded tag id.
                    rvTag_GetDecodedId(self->tag, &tagId);

                    // Get the decoded tag corners.  These are the 2D positions of
                    // corners of the tag within the image.
                    rvTag_GetDecodedCorners(self->tag, corners);

                    // Should we add this tag?
                    if (add)
                    {
                        // Add the tag id to the list of tag ids.
                        self->tags[self->tagCount] = tagId;

                        // Add the corners to the list of corners.
                        self->corners[(self->tagCount << 2)] = corners[0];
                        self->corners[(self->tagCount << 2) + 1] = corners[1];
                        self->corners[(self->tagCount << 2) + 2] = corners[2];
                        self->corners[(self->tagCount << 2) + 3] = corners[3];

                        // Increment the tag count.
                        ++self->tagCount;

                        // Increment the count of tags in this view.
                        ++self->views[self->viewCount];
                    }

                    // Draw the corners.
                    cvDrawCorners(image, corners, CV_RGB(0, 255, 0), CV_RGB(255, 0, 0), 1, 8, 0);

#ifdef _DEBUG
                    {
                        char buffer[16];
                        CvPoint2D32f center;

                        // Print the grid id into the buffer.
                        _snprintf(buffer, sizeof(buffer), "%d", (int) tagId);

                        // Get the coordinates of the center.
                        rvCalibrate_GetCenterPoint(corners, &center);

                        // Write the buffer to the image.
                        cvPutText(image, buffer, cvPointFrom32f(center), &self->font, CV_RGB(255, 0, 0));
                    }
#endif
                }
            }
        }

        // Get the next contour.
        contours = contours->h_next;
    }

    // Increment the view count.
    if (add) ++self->viewCount;

    return true;
}


bool rvCalibrate_ProcessViews(rvCalibrate *self)
{
    int i;
    CvMat *imagePoints;
    CvMat *objectPoints;
    CvMat *pointCounts;

    // Assume we failed.
    self->results = false;

    // Make sure we have some tags to process.
    if (self->tagCount == 0) return false;

    // Create the image, object and point count matrices.
    imagePoints = cvCreateMat(self->tagCount << 2, 2, CV_64FC1);
    objectPoints = cvCreateMat(self->tagCount << 2, 3, CV_64FC1);
    pointCounts = cvCreateMat(self->viewCount, 1, CV_32SC1);

    // Set the position of each corner across all views.
    for (i = 0; i < self->tagCount; ++i)
    {
        cvSetReal2D(imagePoints, (i << 2), 0, self->corners[(i << 2)].x);
        cvSetReal2D(imagePoints, (i << 2), 1, self->corners[(i << 2)].y);
        cvSetReal2D(imagePoints, (i << 2) + 1, 0, self->corners[(i << 2) + 1].x);
        cvSetReal2D(imagePoints, (i << 2) + 1, 1, self->corners[(i << 2) + 1].y);
        cvSetReal2D(imagePoints, (i << 2) + 2, 0, self->corners[(i << 2) + 2].x);
        cvSetReal2D(imagePoints, (i << 2) + 2, 1, self->corners[(i << 2) + 2].y);
        cvSetReal2D(imagePoints, (i << 2) + 3, 0, self->corners[(i << 2) + 3].x);
        cvSetReal2D(imagePoints, (i << 2) + 3, 1, self->corners[(i << 2) + 3].y);
    }

    // Set the 3D position of each tag corner on the grid.
    for (i = 0; i < self->tagCount; ++i)
    {
        CvPoint3D32f corners[RVTAG_CORNER_COUNT];

        // Get the position of the corners of the tag in the grid.
        rvTags384_GetCorners(self->tags[i], corners);

        // Set the 3D position of the tag corner on the grid.
        cvSetReal2D(objectPoints, (i << 2), 0, corners[0].x);
        cvSetReal2D(objectPoints, (i << 2), 1, corners[0].y);
        cvSetReal2D(objectPoints, (i << 2), 2, 0.0);
        cvSetReal2D(objectPoints, (i << 2) + 1, 0, corners[1].x);
        cvSetReal2D(objectPoints, (i << 2) + 1, 1, corners[1].y);
        cvSetReal2D(objectPoints, (i << 2) + 1, 2, 0.0);
        cvSetReal2D(objectPoints, (i << 2) + 2, 0, corners[2].x);
        cvSetReal2D(objectPoints, (i << 2) + 2, 1, corners[2].y);
        cvSetReal2D(objectPoints, (i << 2) + 2, 2, 0.0);
        cvSetReal2D(objectPoints, (i << 2) + 3, 0, corners[3].x);
        cvSetReal2D(objectPoints, (i << 2) + 3, 1, corners[3].y);
        cvSetReal2D(objectPoints, (i << 2) + 3, 2, 0.0);
    }

    // Set the count of points in each view.
    for (i = 0; i < self->viewCount; ++i)
    {
        // Set the count of points in this view.
        cvSet1D(pointCounts, i, cvScalar((double) (self->views[i] << 2), 0, 0, 0));
    }

    // Calibrate the camera based on the point information in the views.
    cvCalibrateCamera2(objectPoints, imagePoints, pointCounts, self->imageSize,
                       self->cameraMatrix, self->distortionCoeffs, NULL, NULL, 0);

    // Release the image and object matrices.
    cvReleaseMat(&imagePoints);
    cvReleaseMat(&objectPoints);
    cvReleaseMat(&pointCounts);

    // We succeeded.
    self->results = true;

    return self->results;
}

