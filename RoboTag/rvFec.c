/*
    This code implements general purpose Reed-Solomon forward error correction
    algorithm for buffers of 3-bit to 8-bit symbols.  It is a based on code by
    Phil Karn, which is in turn a rewrite of code by Robert Morelos-Zaragoza
    and Hari Thirumoorthy, which was in turn based on an earlier program by
    Simon Rockliff.  Further information on Phil Karn's implementation can be
    found at:

    http://www.piclist.com/techref/method/error/rs-gp-pk-uoh-199609/index.htm

    This code is woefully undocumented and uncommented.  Hopefully someone
    who can understand the math within the Reed Solomon algorithm can comment
    and optimize this code further.

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

    $Id: rvFec.c 28 2010-03-09 23:49:39Z mike $
*/

#include <stdlib.h>
#include "rvFec.h"

// No legal value in index form represents zero, so
// we use nn as a special value for this purpose.
#define ALPHA_ZERO  ((uintGF) self->nn)

// Alpha exponent for the first root of the generator polynomial.
#define ALPHA_ROOT  1

// Helper macro to find minimum value.
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

// Primitive polynomial tables.  See Lin & Costello, Error Control Coding
// Appendix A  and  Lee & Messerschmitt, Digital Communication p. 453.

// 1 + x + x^3
rvUint8 pp_3[4] = { 1, 1, 0, 1 };

// 1 + x + x^4
rvUint8 pp_4[5] = { 1, 1, 0, 0, 1 };

// 1 + x^2 + x^5
rvUint8 pp_5[6] = { 1, 0, 1, 0, 0, 1 };

// 1 + x + x^6
rvUint8 pp_6[7] = { 1, 1, 0, 0, 0, 0, 1 };

// 1 + x^3 + x^7
rvUint8 pp_7[8] = { 1, 0, 0, 1, 0, 0, 0, 1 };

// 1+x^2+x^3+x^4+x^8
rvUint8 pp_8[9] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 };

#if 0
// The following primitive polynomial tables are currently unused, but
// copied here for future reference.

// 1+x^4+x^9
rvUint8 pp_9[10] = { 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

// 1+x^3+x^10
rvUint8 pp_10[11] = { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 };

// 1+x^2+x^11
rvUint8 pp_11[12] = { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

// 1+x+x^4+x^6+x^12
rvUint8 pp_12[13] = { 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1 };

// 1+x+x^3+x^4+x^13
rvUint8 pp_13[14] = { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

// 1+x+x^6+x^10+x^14
rvUint8 pp_14[15] = { 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 };

// 1+x+x^15
rvUint8 pp_15[16] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

// 1+x+x^3+x^12+x^16
rvUint8 pp_16[17] = { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 };
#endif

// Helper macro to compute x mod nn.
#define modnn(x) ((x) % self->nn)

static void rvFec_InitGaloisField(rvFec* self, rvUint8 *pp)
// Initialize GF(2**mm) table from the irreducible polynomial p(X) in pp[0]..pp[MM - 1].
//
// The following tables are created:
//
//   index form to polynomial form alphaTo[] contains j=alpha**i;
//   polynomial form to index form indexOf[j=alpha**i] = i
//
// The representation of the elements of GF(2**mm) is either in index form,
// where the number is the power of the primitive element alpha, which is
// convenient for multiplication (add the powers modulo 2**mm-1) or in
// polynomial form, where the bits represent the coefficients of the
// polynomial representation of the number, which is the most convenient form
// for addition.  The two forms are swapped between via lookup tables.
// This leads to fairly messy looking expressions, but unfortunately, there
// is no easy alternative when working with Galois arithmetic.
{
    rvUint16 i;
    rvUint16 mask;

    // Local references so code is easier on the eyes.
    uintGF mm = (uintGF) self->mm;
    uintGF nn = (uintGF) self->nn;
    uintGF *alphaTo = self->alphaTo;
    uintGF *indexOf = self->indexOf;

    mask = 1;
    alphaTo[mm] = 0;
    for (i = 0; i < mm; i++)
    {
        alphaTo[i] = (uintGF) mask;
        indexOf[alphaTo[i]] = (uintGF) i;
        if (pp[i] != 0) alphaTo[mm] ^= (uintGF) mask;
        mask <<= 1;
    }
    indexOf[alphaTo[mm]] = mm;
    mask >>= 1;
    for (i = mm + 1; i < nn; i++)
    {
        if (alphaTo[i - 1] >= mask)
            alphaTo[i] = alphaTo[mm] ^ ((alphaTo[i - 1] ^ mask) << 1);
        else
            alphaTo[i] = alphaTo[i - 1] << 1;
        indexOf[alphaTo[i]] = (uintGF) i;
    }
    indexOf[0] = ALPHA_ZERO;
    alphaTo[nn] = 0;
}


static void rvFec_InitPolynomial(rvFec* self)
// Initialize the generator polynomial table.
{
    rvInt16 i;
    rvInt16 j;

    // Local references so code is easier on the eyes.
    uintGF *gg = self->gg;
    uintGF *alphaTo = self->alphaTo;
    uintGF *indexOf = self->indexOf;

    gg[0] = alphaTo[ALPHA_ROOT];
    gg[1] = 1;
    for (i = 2; i <= self->paritySize; i++)
    {
        gg[i] = 1;
        for (j = i - 1; j > 0; --j)
        {
            if (gg[j] != 0)
                gg[j] = gg[j - 1] ^ alphaTo[modnn((indexOf[gg[j]]) + ALPHA_ROOT + i - 1)];
            else
                gg[j] = gg[j - 1];
        }
        gg[0] = alphaTo[modnn((indexOf[gg[0]]) + ALPHA_ROOT + i - 1)];
    }

    // Convert gg[] to index form for quicker encoding.
    for (i = 0; i <= self->paritySize; i++) gg[i] = indexOf[gg[i]];
}


rvFec* rvFec_New(rvInt16 symbolSize, rvInt16 dataSize, rvInt16 paritySize)
// Initialize the encoder object.
{
    rvInt16 mm;
    rvInt16 nn;
    rvInt16 kk;
    rvInt16 zeroSize;
    rvInt16 blockSize;
    rvUint8* pp = NULL;
    uintGF* gg = NULL;
    uintGF* alphaTo = NULL;
    uintGF* indexOf = NULL;
    rvFec* self = NULL;

    // Set the Reed Solomon properties.
    mm = symbolSize;
    nn = ((1 << mm) - 1);
    kk = nn - paritySize;

    // The block size is the combined data and parity size.
    blockSize = dataSize + paritySize;

    // The zero size is the size of the zero padded data.
    zeroSize = nn - blockSize;

    // Sanity check block and data size.
    if ((blockSize < 1) || (blockSize > nn)) return NULL;
    if ((dataSize < 1) || (dataSize >= blockSize)) return NULL;

    // Point to primative polynomial table.
    if (symbolSize == 3) pp = pp_3;
    else if (symbolSize == 4) pp = pp_4;
    else if (symbolSize == 5) pp = pp_5;
    else if (symbolSize == 6) pp = pp_6;
    else if (symbolSize == 7) pp = pp_7;
    else if (symbolSize == 8) pp = pp_8;
    else return NULL;

    // Allocate a new ecc object and tables.
    self = (rvFec *) malloc(sizeof(rvFec));
    gg = (uintGF*) malloc(sizeof(uintGF) * (paritySize + 1));
    alphaTo = (uintGF*) malloc(sizeof(uintGF) * (nn + 1));
    indexOf = (uintGF*) malloc(sizeof(uintGF) * (nn + 1));

    // Did we allocate the object.
    if ((self != NULL) && (gg != NULL) && (alphaTo != NULL) && (indexOf != NULL))
    {
        // Set the Reed Solomon properties.
        self->mm = mm;
        self->nn = nn;
        self->kk = kk;

        // Set the buffer properties.
        self->zeroSize = zeroSize;
        self->dataSize = dataSize;
        self->paritySize = paritySize;
        self->blockSize = blockSize;

        // Set the allocated tables.
        self->gg = gg;
        self->alphaTo = alphaTo;
        self->indexOf = indexOf;

        // Initialize Galois Field table
        rvFec_InitGaloisField(self, pp);

        // Initialize generator polynomial.
        rvFec_InitPolynomial(self);
    }
    else
    {
        // Clean up.
        if (self) free(self);
        if (gg) free(gg);
        if (alphaTo) free(alphaTo);
        if (indexOf) free(indexOf);
    }

    return self;
}


void rvFec_Free(rvFec* self)
// Free the encoder object.
{
    // Sanity check the object pointer.
    if (self != NULL)
    {
        // Free the ecc object.
        free(self->gg);
        free(self->alphaTo);
        free(self->indexOf);
        free(self);
    }
}


rvInt16 rvFec_Parity(rvFec* self, rvUint8* dataBuffer, rvUint8* parityBuffer)
// Encode parity for the data buffer.  Returns 1 if success or 0 if
// unable to encode.  Encoding is done by using a feedback shift register
// with appropriate connections specified by the elements of gg[], which
// was generated by the ecc_generate_galois_field() function.
{
    rvInt16 i;
    rvInt16 j;
    uintGF feedback;

    // Local references so code is easier on the eyes.
    uintGF *gg = self->gg;
    uintGF *alphaTo = self->alphaTo;
    uintGF *indexOf = self->indexOf;

    // Zero the parity data.
    for (i = 0; i < self->paritySize; ++i) parityBuffer[i] = 0;

    // Loop over the data portion of the encode buffer.
    for (i = self->zeroSize + self->dataSize; i >= 0; --i)
    {
        // Pad data with zero to account for shortened buffer when determining feedback.
        feedback = indexOf[((i >= self->zeroSize) ? dataBuffer[i - self->zeroSize] : 0) ^ parityBuffer[self->paritySize - 1]];

        // Is the feedback term non-zero?
        if (feedback != ALPHA_ZERO)
        {
            for (j = self->paritySize - 1; j > 0; --j)
            {
                if (gg[j] != ALPHA_ZERO)
                    parityBuffer[j] = parityBuffer[j - 1] ^ alphaTo[modnn(gg[j] + feedback)];
                else
                    parityBuffer[j] = parityBuffer[j - 1];
            }

            parityBuffer[0] = alphaTo[modnn(gg[0] + feedback)];
        }
        else
        {
            for (j = self->paritySize - 1; j > 0; --j)
            {
                parityBuffer[j] = parityBuffer[j - 1];
            }

            parityBuffer[0] = 0;
        }
    }

    return 1;
}


rvInt16 rvFec_Correct(rvFec* self, rvUint8* blockBuffer)
// Correct the block buffer.  Returns 1 if success or 0 if unable to correct.
{
    rvInt16 i;
    rvInt16 j;
    rvInt16 r;
    rvInt16 el;
    rvInt16 count;
    rvInt16 syn_error;
    rvInt16 deg_omega;
    rvInt16 deg_lambda;
    rvInt16 *loc;
    uintGF q;
    uintGF *b;
    uintGF *t;
    uintGF *reg;
    uintGF *recd;
    uintGF *root;
    uintGF *omega;
    uintGF *lambda;
    uintGF *syndromes;

    // Local references so code is easier on the eyes.
    rvInt16 nn = self->nn;
    uintGF *gg = self->gg;
    uintGF *alphaTo = self->alphaTo;
    uintGF *indexOf = self->indexOf;

    // Allocate the recieve buffer off the stack.
    recd = (uintGF*) _alloca(sizeof(uintGF) * nn);

    // Copy and convert the encoded buffer from polynomial form to index form.
    for (i = nn - 1; i >= 0; --i)
    {
        // Pad data with zero to account for shortened buffer.
        recd[i] = indexOf[(i >= self->zeroSize) ? blockBuffer[i - self->zeroSize] : 0];
    }

    // Allocate the syndromes buffer off the stack.
    syndromes = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));

    // Initialize the syndrome error flag.
    syn_error = 0;

    // Form and store each syndrome by evaluating recd(x) at roots of g(x)
    // namely @**(ALPHA_ROOT+i), i = 0, ... ,(nn-KK-1)
    for (i = 1; i <= self->paritySize; i++)
    {
        uintGF syndrome = 0;

        // Loop over each data byte.
        for (j = 0; j < nn; j++)
        {
            // All non-zero receive values added to the syndrome.
            if (recd[j] != ALPHA_ZERO)
            {
                syndrome ^= alphaTo[modnn(recd[j] + (ALPHA_ROOT + i - 1) * j)];
            }
        }

        // Set flag if non-zero syndrome.
        syn_error |= syndrome;

        // Store syndrome in index form.
        syndromes[i] = indexOf[syndrome];
    }

    if (!syn_error)
    {
        // If all syndromes are zero there are no errors to correct.
        return 1;
    }

    // Allocate the lambda buffer off the stack.
    lambda = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));

    // Clear the lampda buffer.
    for (i = 1; i < self->paritySize + 1; ++i) lambda[i] = 0;

    lambda[0] = 1;

    b = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));
    t = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));

    for (i = 0; i < self->paritySize + 1; i++) b[i] = indexOf[lambda[i]];

    // Begin Berlekamp-Massey algorithm to determine error locator polynomial.
    el = 0;

    for (r = 1; r <= self->paritySize; ++r)
    {
        uintGF discrepancy = 0;

        // Compute discrepancy at the r-th step in poly-form.
        for (i = 0; i < r; i++)
        {
            if ((lambda[i] != 0) && (syndromes[r - i] != ALPHA_ZERO))
            {
                discrepancy ^= alphaTo[modnn(indexOf[lambda[i]] + syndromes[r - i])];
            }
        }

        // Convert the discrepancy to index form.
        discrepancy = indexOf[discrepancy];

        // Is the discrepancy zero?
        if (discrepancy == ALPHA_ZERO)
        {
            // Move all array elements up.
            for (i = self->paritySize - 1; i >= 0; --i) b[i + 1] = b[i];

            // The first array element is zero.
            b[0] = ALPHA_ZERO;
        }
        else
        {
            // 7 lines below: T(x) <-- lambda(x) - discrepancy*x*b(x)
            t[0] = lambda[0];

            for (i = 0 ; i < self->paritySize; i++)
            {
                if (b[i] != ALPHA_ZERO)
                    t[i+1] = lambda[i+1] ^ alphaTo[modnn(discrepancy + b[i])];
                else
                    t[i+1] = lambda[i+1];
            }

            if ((2 * el) <= (r - 1))
            {
                el = r - el;

                // 2 lines below: B(x) <-- inv(discrepancy) * lambda(x)
                for (i = 0; i <= self->paritySize; i++)
                {
                    b[i] = (lambda[i] == 0) ? ALPHA_ZERO : modnn(indexOf[lambda[i]] - discrepancy + nn);
                }
            }
            else
            {
                // Move all array elements up.
                for (i = self->paritySize - 1; i >= 0; --i) b[i + 1] = b[i];

                // The first array element is zero.
                b[0] = ALPHA_ZERO;
            }

            for (i = 0; i < self->paritySize + 1; ++i) lambda[i] = t[i];
        }
    }

    deg_lambda = 0;

    // Convert lambda to index form and compute deg(lambda(x)).
    for (i = 0; i < self->paritySize + 1; ++i)
    {
        lambda[i] = indexOf[lambda[i]];

        if (lambda[i] != ALPHA_ZERO) deg_lambda = i;
    }

    loc = (rvInt16*) _alloca(sizeof(rvInt16) * self->paritySize);
    reg = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));
    root = (uintGF*) _alloca(sizeof(uintGF) * self->paritySize);

    // Find roots of the error locator polynomial by Chien search.
    for (i = 1; i < self->paritySize + 1; ++i) reg[i] = lambda[i];

    // Number of roots of lambda(x).
    count = 0;

    for (i = 1; i <= nn; i++)
    {
        q = 1;

        for (j = deg_lambda; j > 0; j--)
        {
            if (reg[j] != ALPHA_ZERO)
            {
                reg[j] = modnn(reg[j] + j);
                q ^= alphaTo[reg[j]];
            }
        }

        if (!q)
        {
            // Store root (index-form) and error location number.
            root[count] = (uintGF) i;
            loc[count] = nn - i;
            count++;
        }
    }

#ifdef DEBUG
    printf("\n Final error positions:\t");
    for (i = 0; i < count; i++)
        printf("%d ", loc[i]);
    printf("\n");
#endif

    // If deg(lambda) unequal to number of roots then uncorrectable error detected.
    if (deg_lambda != count)
    {
        return 0;
    }

    deg_omega = 0;

    omega = (uintGF*) _alloca(sizeof(uintGF) * (self->paritySize + 1));

    // Compute error evaluator poly omega(x) = s(x) * lambda(x) (modulo x**(NN-KK))
    // in index form. Also find deg(omega).
    for (i = 0; i < self->paritySize; ++i)
    {
        uintGF tmp = 0;

        for (j = (deg_lambda < i) ? deg_lambda : i; j >= 0; --j)
        {
            if ((syndromes[i + 1 - j] != ALPHA_ZERO) && (lambda[j] != ALPHA_ZERO))
                tmp ^= alphaTo[modnn(syndromes[i + 1 - j] + lambda[j])];
        }

        if (tmp != 0) deg_omega = i;

        omega[i] = indexOf[tmp];
    }

    omega[self->paritySize] = ALPHA_ZERO;

    // Compute error values in poly-form.
    for (j = count - 1; j >= 0; j--)
    {
        uintGF den;
        uintGF num1;
        uintGF num2;

        num1 = 0;

        for (i = deg_omega; i >= 0; i--)
        {
            if (omega[i] != ALPHA_ZERO) num1  ^= alphaTo[modnn(omega[i] + i * root[j])];
        }

        num2 = alphaTo[modnn(root[j] * (ALPHA_ROOT - 1) + nn)];

        den = 0;

        // lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i]
        for (i = MIN(deg_lambda, self->paritySize - 1) & ~1; i >= 0; i -=2)
        {
            if (lambda[i + 1] != ALPHA_ZERO) den ^= alphaTo[modnn(lambda[i+1] + i * root[j])];
        }

        if (den == 0)
        {
#ifdef DEBUG
            printf("\n ERROR: denominator = 0\n");
#endif
            return 0;
        }

        // Apply error to data.
        if (num1 != 0)
        {
            // We should never need to correct in the zero padding.
            if (loc[j] < self->zeroSize)
            {
                return 0;
            }

            // Account for shortened data.
            if ((loc[j] >= self->zeroSize) && (loc[j] < self->zeroSize + self->dataSize))
            {
                blockBuffer[loc[j] - self->zeroSize] ^= alphaTo[modnn(indexOf[num1] + indexOf[num2] + nn - indexOf[den])];
            }
        }
    }

    return 1;
}

