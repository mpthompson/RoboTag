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

    $Id: MakeTag.cpp 30 2010-03-11 17:57:43Z mike $
*/

#include "stdafx.h"
#include "rvBitfield.h"
#include "rvCrc16.h"
#include "rvDecode.h"
#include "rvFec.h"
#include "rvSvg.h"

static char tagTemplate[] = "tag%05d.svg";
static char tableTemplate[] = "table%dx%d.svg";

static void usage(void)
{
    fprintf(stderr, "MakeTag.exe - Output SVG file for the specified tag(s).  A single tag\n");
    fprintf(stderr, "(1x1) table is output by default with the filename tagXXXXX.svg.  If the\n");
    fprintf(stderr, "width or height is greater than one then a table is output with the filename\n");
    fprintf(stderr, "tableXXXxXXX.svg.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Command Arguments:\n");
    fprintf(stderr, " -h        Display help.\n");
    fprintf(stderr, " -i n      Use specified tag id.\n");
    fprintf(stderr, " -r n      Table rows - must be greater than zero.\n");
    fprintf(stderr, " -c n      Table colums - must be greater than zero.\n");
}

int _tmain(int argc, _TCHAR **argv)
{
    char filename[80];
    rvUint8 tagBytes[8];
    rvUint8 tagValues[64];
    rvUint16 tagId = 0;
    rvUint16 tableCols = 1;
    rvUint16 tableRows = 1;
    FILE *fp = NULL;
    rvFec *tagFec = NULL;
    rvBitfield *tagBits = NULL;

    // Discard the first command argument.
    if (argc > 0) --argc, ++argv;

    // Process the command argument.
    while (argc > 0)
    {
        // Do we recognize this command?
        if (!_tcscmp(*argv, _T("-h")))
        {
            // Get usage.
            usage();

            return -1;
        }
        else if (!_tcscmp(*argv, _T("-i")))
        {
            rvUint32 tempId;

            // Make sure we have the id.
            if (argc < 2)
            {
                fprintf(stderr, "ERROR: Missing tag id.\n");
                usage();
                return -1;
            }

            // Get the temp tag id.
            tempId = _wtoi(*(argv + 1));

            // Make sure it is a valid tag id.
            if (tempId > 65536)
            {
                fprintf(stderr, "ERROR: Invalid camera id %d.\n", tempId);
            }

            // Set the tag id.
            tagId = (rvUint16) tempId;

            // Move to the next argument.
            argc -= 2; argv += 2;
        }
        else if (!_tcscmp(*argv, _T("-r")))
        {
            // Make sure we have the table height.
            if (argc < 2)
            {
                fprintf(stderr, "ERROR: Missing table height.\n");
                usage();
                return -1;
            }

            // Get the table rows.
            tableRows = (rvUint16) _wtoi(*(argv + 1));

            // Move to the next argument.
            argc -= 2; argv += 2;
        }
        else if (!_tcscmp(*argv, _T("-c")))
        {
            // Make sure we have the table width.
            if (argc < 2)
            {
                fprintf(stderr, "ERROR: Missing table width.\n");
                usage();
                return -1;
            }

            // Get the table rows.
            tableCols = (rvUint16) _wtoi(*(argv + 1));

            // Move to the next argument.
            argc -= 2; argv += 2;
        }
        else
        {
            char tmpStr[32];
            size_t charsConverted;
            wcstombs_s(&charsConverted, tmpStr, sizeof(tmpStr), (const wchar_t *) *argv, _TRUNCATE);
            fprintf(stderr, "ERROR: Unknown argument %s.\n", tmpStr);
            usage();
            return -1;
        }
    }

    // Fill in the file name.
    if ((tableCols == 1) && (tableRows == 1))
    {
        _snprintf_s(filename, sizeof(filename), tagTemplate, (int) tagId);
    }
    else
    {
        _snprintf_s(filename, sizeof(filename), tableTemplate, (int) tableCols, (int) tableRows);
    }

    // Create a forward error correction object.
    tagFec = rvFec_New(8, 4, 4);

    // Create a 64-bit bitfield object.
    tagBits = rvBitfield_New(64);

    // Open the file for writing.
    if (fopen_s(&fp, filename, "w") == 0)
    {
        rvUint16 id;
        rvUint16 tableCol;
        rvUint16 tableRow;

        // Print the tag header.
        rvSvg_TableBegin(fp, filename, tableCols, tableRows);

        // Loop over each row.
        for (tableRow = 0; tableRow < tableRows; ++tableRow)
        {
            // Loop over each column.
            for (tableCol = 0; tableCol < tableCols; ++tableCol)
            {
                // Zero out the tag id buffer.
                memset(tagBytes, 0, sizeof(tagBytes));

                // XOR the tag id.
                id = tagId ^ 0xa5a5;

                // Place the tag id into the tag id buffer.
                tagBytes[0] = (id >> 8) & 0xff; 
                tagBytes[1] = id & 0xff; 

                // Calculate the CRC CCITT portion of the tag id buffer.
                rvCrc16_CCITT(tagBytes, 2, tagBytes + 2);

                // Calculate the FEC portion of the tag id buffer.
                rvFec_Parity(tagFec, tagBytes, tagBytes + 4);

                // Set the tag id buffer into the bit fields.
                rvBitfield_SetBytes(tagBits, sizeof(tagBytes), tagBytes);

                // Get the bitfield values as an array of bytes.
                rvBitfield_GetBits(tagBits, sizeof(tagValues), tagValues, rvDecodeMappingNorth);

                // Print the tag.
                rvSvg_TableGrid8x8(fp, NULL, tableCol, tableRow, tagValues, sizeof(tagValues));

                // Increment the tag id.
                ++tagId;
            }
        }

        // Print the tag footer.
        rvSvg_TableEnd(fp);

        // Close the file.
        fclose(fp);
    }

    // Free the bitfield object.
    rvBitfield_Free(tagBits);

    // Free the tag forward error correction.
    rvFec_Free(tagFec);

    return 0;
}

