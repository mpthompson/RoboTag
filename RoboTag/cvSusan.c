/*
    SUSAN is an acronym for Smallest Univalue Segment Assimilating Nucleus.
    The SUSAN algorithms cover image noise filtering, edge finding and
    corner finding.

    Additional information: http://users.fmrib.ox.ac.uk/~steve/susan/

    Original source code: http://users.fmrib.ox.ac.uk/~steve/susan/susan2l.c

    (C) Crown Copyright (1995-1999), Defence Evaluation and Research Agency,
    Farnborough, Hampshire, GU14 6TD, UK
    DERA WWW site:
    http://www.dera.gov.uk/
    DERA Computer Vision and Electro Optics Group WWW site:
    http://www.dera.gov.uk/imageprocessing/dera/group_home.html
    DERA Computer Vision and Electro Optics Group point of contact:
    Dr. John Savage, jtsavage@dera.gov.uk, +44 1344 633203

    A UK patent has been granted: "Method for digitally processing
    images to determine the position of edges and/or corners therein for
    guidance of unmanned vehicle", UK Patent 2272285. Proprietor:
    Secretary of State for Defence, UK. 15 January 1997

    This code is issued for research purposes only and remains the
    property of the UK Secretary of State for Defence. This code must
    not be passed on without this header information being kept
    intact. This code must not be sold.

    $Id: cvSusan.c 28 2010-03-09 23:49:39Z mike $
*/

#include <ctype.h>
#include "cvSusan.h"

static uchar susanBrightTable[516] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 5, 9, 17, 26, 36, 47,
    58, 68, 76, 83, 88, 92, 95, 97,
    98, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 100, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 98, 97, 95, 92,
    88, 83, 76, 68, 58, 47, 36, 26,
    17, 9, 5, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

static void icvSusanThin(int *r, uchar *mid, int x_size, int y_size)
// Only one pass is needed as i,j are decremented if
// necessary to go back and do bits again.
{
    int l[9], centre,
        b01, b12, b21, b10,
        p1, p2, p3, p4,
        b00, b02, b20, b22,
        m, n, a, b, x, y, i, j;
    uchar *mp;

    for (i = 4; i < y_size - 4; ++i)
    {
        for (j = 4; j < x_size - 4; ++j)
        {
            if (mid[i * x_size + j] < 8)
            {
                centre = r[i * x_size + j];

                /* count number of neighbours */
                mp = mid + (i - 1) * x_size + j - 1;

                n = (*mp < 8) +
                    (*(mp + 1) < 8) +
                    (*(mp + 2) < 8) +
                    (*(mp + x_size) < 8) +
                    (*(mp + x_size + 2) < 8) +
                    (*(mp + x_size + x_size) < 8) +
                    (*(mp + x_size + x_size + 1) < 8) +
                    (*(mp + x_size + x_size + 2) < 8);

                /* n==0 no neighbours - remove point */
                if (n == 0) mid[i * x_size + j] = 100;

                /* n == 1 - extend line if I can */
                /* extension is only allowed a few times - the value of mid is used to control this */
                if ((n == 1) && (mid[i * x_size + j] < 6))
                {
                    /* find maximum neighbour weighted in direction opposite the
                         neighbour already present. e.g.
                         have: O O O  weight r by 0 2 3
                                     X X O              0 0 4
                                     O O O              0 2 3     */

                    l[0] = r[(i - 1) * x_size + j - 1];
                    l[1] = r[(i - 1) * x_size + j];
                    l[2] = r[(i - 1) * x_size + j + 1];
                    l[3] = r[(i) * x_size + j - 1];
                    l[4] = 0;
                    l[5] = r[(i) * x_size + j + 1];
                    l[6] = r[(i + 1) * x_size + j - 1];
                    l[7] = r[(i + 1) * x_size + j];
                    l[8] = r[(i + 1) * x_size + j + 1];

                    if (mid[(i - 1) * x_size + j - 1] < 8)
                    {
                        l[0] = 0; l[1] = 0; l[3] = 0; l[2] *= 2;
                        l[6] *= 2; l[5] *= 3; l[7] *= 3; l[8] *= 4;
                    }
                    else if (mid[(i - 1) * x_size + j] < 8)
                    {
                        l[1] = 0; l[0] = 0; l[2] = 0; l[3] *= 2;
                        l[5] *= 2; l[6] *= 3; l[8] *= 3; l[7] *= 4;
                    }
                    else if (mid[(i-1) * x_size + j + 1] < 8)
                    {
                        l[2] = 0; l[1] = 0; l[5] = 0; l[0] *= 2;
                        l[8] *= 2; l[3] *= 3; l[7] *= 3; l[6] *= 4;
                    }
                    else if (mid[(i) * x_size + j - 1] < 8)
                    {
                        l[3] = 0; l[0] = 0; l[6] = 0; l[1] *= 2;
                        l[7] *= 2; l[2] *= 3; l[8] *= 3; l[5] *= 4;
                    }
                    else if (mid[(i) * x_size + j + 1] < 8)
                    {
                        l[5] = 0; l[2] = 0; l[8] = 0; l[1] *= 2;
                        l[7] *= 2; l[0] *= 3; l[6] *= 3; l[3] *= 4;
                    }
                    else if (mid[(i + 1) * x_size + j - 1] < 8)
                    {
                        l[6] = 0; l[3] = 0; l[7] = 0; l[0] *= 2;
                        l[8] *= 2; l[1] *= 3; l[5] *= 3; l[2] *= 4;
                    }
                    else if (mid[(i + 1) * x_size + j] < 8)
                    {
                        l[7] = 0; l[6] = 0; l[8] = 0; l[3] *= 2;
                        l[5] *= 2; l[0] *= 3; l[2] *= 3; l[1] *= 4;
                    }
                    else if (mid[(i + 1) * x_size + j + 1] < 8)
                    {
                        l[8] = 0; l[5] = 0; l[7] = 0; l[6] *= 2;
                        l[2] *= 2; l[1] *= 3; l[3] *= 3; l[0] *= 4;
                    }

                    // Find the highest point.
                    m = 0;
                    for (y = 0; y < 3; ++y)
                    {
                        for (x = 0; x < 3; ++x)
                        {
                            if (l[y + y + y + x] > m) { m = l[y + y + y + x]; a = y; b = x; }
                        }
                    }

                    // Did we find the highest point?
                    if (m > 0)
                    {
                        if (mid[i * x_size + j] < 4)
                        {
                            mid[(i + a - 1) * x_size + j + b - 1] = 4;
                        }
                        else
                        {
                            mid[(i + a - 1) * x_size + j + b - 1] = mid[i * x_size + j] + 1;
                        }
                        // Do we need to jump back in image?
                        if ((a + a + b) < 3)
                        {
                            i += a - 1;
                            j += b - 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }

                if (n == 2)
                {
                    /* put in a bit here to straighten edges */
                    b00 = mid[(i - 1) * x_size + j - 1] < 8; /* corners of 3x3 */
                    b02 = mid[(i - 1) * x_size + j + 1] < 8;
                    b20 = mid[(i + 1) * x_size + j - 1] < 8;
                    b22 = mid[(i + 1) * x_size + j + 1] < 8;

                    if (((b00 + b02 + b20 + b22) == 2) && ((b00 | b22) & (b02 | b20)))
                    {
                        /* case: move a point back into line.
                                e.g. X O X  CAN  become X X X
                                         O X O              O O O
                                         O O O              O O O    */
                        if (b00)
                        {
                            if (b02) { x = 0; y = -1; }
                            else     { x = -1; y = 0; }
                        }
                        else
                        {
                            if (b02) { x = 1; y = 0; }
                            else     { x = 0; y = 1; }
                        }
                        if (((double) r[(i + y) * x_size + j + x] / (float) centre) > 0.7)
                        {
                            if (((x == 0) &&
                                 (mid[(i + (2 * y)) * x_size + j] > 7) &&
                                 (mid[(i + (2 * y)) * x_size + j - 1] > 7) &&
                                 (mid[(i + (2 * y)) * x_size + j + 1] > 7)) ||
                                ((y == 0) &&
                                 (mid[(i) * x_size + j + (2 * x)] > 7) &&
                                 (mid[(i + 1) * x_size + j + (2 * x)] > 7) &&
                                 (mid[(i - 1) * x_size + j + (2 * x)] > 7)))
                            {
                                mid[(i) * x_size + j] = 100;
                                mid[(i + y) * x_size + j + x] = 3;  /* no jumping needed */
                            }
                        }
                    }
                    else
                    {
                        b01 = mid[(i - 1) * x_size + j ] < 8;
                        b12 = mid[(i) * x_size + j + 1] < 8;
                        b21 = mid[(i + 1) * x_size + j] < 8;
                        b10 = mid[(i) * x_size + j - 1] < 8;

                        if (((b01 + b12 + b21 + b10) == 2) &&
                            ((b10 | b12) & (b01 | b21)) &&
                            ((b01 & ((mid[(i - 2) * x_size + j - 1] < 8) | (mid[(i - 2) * x_size + j + 1] < 8))) |
                             (b10 & ((mid[(i - 1) * x_size + j - 2] < 8) | (mid[(i + 1) * x_size + j - 2] < 8))) |
                             (b12 & ((mid[(i - 1) * x_size + j + 2] < 8) | (mid[(i + 1) * x_size + j + 2] < 8))) |
                             (b21 & ((mid[(i + 2) * x_size + j - 1] < 8) | (mid[(i + 2) * x_size + j + 1] < 8)))))
                        { /* case; clears odd right angles.
                                 e.g.; O O O  becomes O O O
                                             X X O          X O O
                                             O X O          O X O     */
                            mid[(i) * x_size + j] = 100;
                            i -= 1;               /* jump back */
                            j -= 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }

                /* n>2 the thinning is done here without breaking connectivity */
                if (n > 2)
                {
                    b01 = mid[(i - 1) * x_size + j] < 8;
                    b12 = mid[(i) * x_size + j + 1] < 8;
                    b21 = mid[(i + 1) * x_size + j] < 8;
                    b10 = mid[(i) * x_size + j - 1] < 8;
                    if((b01 + b12 + b21 + b10) > 1)
                    {
                        b00 = mid[(i - 1) * x_size + j - 1] < 8;
                        b02 = mid[(i - 1) * x_size + j + 1] < 8;
                        b20 = mid[(i + 1) * x_size + j - 1] < 8;
                        b22 = mid[(i + 1) * x_size + j + 1] < 8;
                        p1 = b00 | b01;
                        p2 = b02 | b12;
                        p3 = b22 | b21;
                        p4 = b20 | b10;

                        if(((p1 + p2 + p3 + p4) - ((b01 & p2) + (b12 & p3) + (b21 & p4)+(b10 & p1))) < 2)
                        {
                            mid[(i) * x_size + j] = 100;
                            i -= 1;
                            j -= 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }
            }
        }
    }
}

void cvSusan(const CvArr* srcarr, CvArr* dstarr, int thresh, int thin)
{
    int a;
    int b;
    int c;
    int i;
    int j;
    int m;
    int n;
    int w;
    int x;
    int y;
    int do_symmetry;
    int* r = 0;
    uchar *o = 0;
    uchar *p = 0;
    uchar *cp = 0;
    uchar *mid = 0;
    double z;

    CV_FUNCNAME("cvSusan");

    __BEGIN__

    CvMat srcstub, *src = (CvMat *) srcarr;
    CvMat dststub, *dst = (CvMat *) dstarr;
    CvSize size;
    uchar *btbl;
    int max_no;

    CV_CALL(src = cvGetMat(src, &srcstub, NULL, 0));
    CV_CALL(dst = cvGetMat(dst, &dststub, NULL, 0));

    if (CV_MAT_TYPE( src->type ) != CV_8UC1 ||
        CV_MAT_TYPE( dst->type ) != CV_8UC1)
        CV_ERROR( CV_StsUnsupportedFormat, "");

    if (!((src)->rows == (dst)->rows && (src)->cols == (dst)->cols))
        CV_ERROR( CV_StsUnmatchedSizes, "");

    size = cvSize(src->cols, src->rows);

    // Point to the middle of the susan brightness table.
    btbl = susanBrightTable + 258;

    // Allocate the mid table.
    CV_CALL(mid = (uchar*) cvAlloc(sizeof(uchar) * size.width * size.height));

    // Initialize the mid table.  Note that it is not set to zero.
    memset(mid, 100, sizeof(uchar) * size.width * size.height);

    // Allocate the r table.
    CV_CALL(r = (int*) cvAlloc(sizeof(int) * size.width * size.height));

    // Initialize the r table.
    memset(r, 0, sizeof(int) * size.width * size.height);

    max_no = 730;

    for (i = 1; i < size.height - 1; i++)
    {
        for (j = 1; j < size.width - 1; j++)
        {
            n = 100;
            o = src->data.ptr + (i * src->step) + j;
            cp = btbl + *o;

            p = (o - (src->step + 1));
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            p = (o - 1);
            n += *(cp - *p);

            p = (o + 1);
            n += *(cp - *p);

            p = (o + (src->step - 1));
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            if (n <= max_no) r[i * size.width + j] = max_no - n;
        }
    }

    for (i = 2; i < size.height - 2; i++)
    {
        for (j = 2; j < size.width - 2; j++)
        {
            if (r[(i * size.width) + j] > 0)
            {
                m = r[(i * size.width) + j];
                n = max_no - m;
                o = src->data.ptr + (i * src->step) + j;
                cp = btbl + *o;

                if (n > 250)
                {
                    x = 0; y = 0;

                    p = (o - (src->step + 1));
                    c = *(cp - *p++); x -= c; y -= c;
                    c = *(cp - *p++); y -= c;
                    c = *(cp - *p); x += c; y -= c;

                    p = (o - 1);
                    c = *(cp - *p); x -= c;

                    p = (o + 1);
                    c = *(cp - *p); x += c;

                    p = (o + (src->step - 1));
                    c = *(cp - *p++); x -= c; y += c;
                    c = *(cp - *p++); y += c;
                    c = *(cp - *p); x += c; y += c;

                    // z = sqrt((float)((x * x) + (y * y)));
                    z = (double) ((x * x) + (y * y));

                    // if (z > (0.4 * (float) n)) /* 0.6 */
                    if (z > (.16 * (double) (n * n)))
                    {
                        do_symmetry = 0;
                        z = (x == 0) ? 1000000.0 : ((double) y) / ((double) x);

                        if (z < 0)
                        {
                            z = -z; w = -1;
                        }
                        else
                        {
                            w = 1;
                        }
                        if (z < 0.5)
                        {
                            /* vert_edge */
                            a = 0; b = 1;
                        }
                        else if (z > 2.0)
                        {
                            /* hor_edge */
                            a = 1; b = 0;
                        }
                        else
                        {
                            /* diag_edge */
                            if (w > 0)
                            {
                                a = 1; b = 1;
                            }
                            else
                            {
                                a = -1; b = 1;
                            }
                        }
                        if ((m > r[((i + a) * size.width) + j + b]) && (m >= r[((i - a) * size.width) + j - b])) mid[(i * size.width) + j] = 1;
                    }
                    else
                    {
                        do_symmetry = 1;
                    }
                }
                else
                {
                    do_symmetry = 1;
                }

                if (do_symmetry == 1)
                {
                    x = 0; y = 0; w = 0;

                    o = src->data.ptr + (i * src->step) + j;

                    p = (o - (src->step + 1));
                    c = *(cp - *p++); x += c; y += c; w += c;
                    c = *(cp - *p++); y += c;
                    c = *(cp - *p); x += c; y += c; w -= c;

                    p = (o - 1);
                    c = *(cp - *p); x += c;

                    p = (o + 1);
                    c = *(cp - *p); x += c;

                    p = (o + (src->step - 1));
                    c = *(cp - *p++); x += c; y += c; w -= c;
                    c = *(cp - *p++); y += c;
                    c = *(cp - *p); x += c; y += c; w += c;

                    z = (y == 0) ? 1000000.0 : ((double) x) / ((double) y);

                    if (z < 0.5)
                    {
                        /* vertical */
                        a = 0; b = 1;
                    }
                    else if (z > 2.0)
                    {
                        /* horizontal */
                        a = 1; b = 0;
                    }
                    else
                    {
                        /* diagonal */
                        if (w > 0)
                        {
                            a = -1; b = 1;
                        }
                        else
                        {
                            a = 1; b = 1;
                        }
                    }

                    if ((m > r[((i + a ) * size.width) + j + b]) && (m >= r[((i - a) * size.width) + j - b])) mid[(i * size.width) + j] = 2;
                }
            }
        }
    }

    // Should the image be thinned?
    if (thin) icvSusanThin(r, mid, size.width, size.height);

    // Fill in the desination image.
    for (i = 1; i < size.height - 1; ++i)
    {
        o = dst->data.ptr + (i * dst->step);

        for (j = 1; j < size.width - 1; ++j)
        {
            *(o++) = (mid[i * size.width + j] < 8) ? 255 : 0;
        }
    }

    __END__;
    cvFree((void**)&r);
    cvFree((void**)&mid);
}