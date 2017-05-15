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

    $Id: rvSvg.c 30 2010-03-11 17:57:43Z mike $
*/

#include "rvSvg.h"

static char* rvSvgHeader =   "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" 
						     "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\" [\n"
						     "    <!ENTITY ns_svg \"http://www.w3.org/2000/svg\">\n"
						     "    <!ENTITY ns_ai \"http://ns.adobe.com/AdobeIllustrator/10.0/\">\n"
						     "]>\n";

static char* rvSvgComment =  "<!-- %s -->\n";

static char* rvSvgStart =    "<svg width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" xmlns=\"&ns_svg;\" xmlns:i=\"&ns_ai;\" "
                             "i:viewOrigin=\"0 %d\" i:rulerOrigin=\"0 0\" i:pageBounds=\"0 %d %d 0\">\n"
						     "    <g id=\"grid\">\n";

static char* rvSvgComment2 = "        <!-- %s -->\n";

static char* rvSvgRect =     "        <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>\n";

static char* rvSvgEnd =      "    </g>\n"
                             "</svg>\n";

void rvSvg_TableBegin(FILE *fp, char *comment, rvUint16 width, rvUint16 height)
// Output the SVG header information.
{
	rvUint16 pt_width = (width * 135) - 45;
	rvUint16 pt_height = (height * 135) - 45;

	// Print the svg header.
	fprintf(fp, rvSvgHeader);

	// Print the svg comment.
	fprintf(fp, rvSvgComment, comment);

	// Print the svg start.
	fprintf(fp, rvSvgStart, pt_width, pt_height, pt_width, pt_height, pt_height, pt_height, pt_width);
}


void rvSvg_TableEnd(FILE *fp)
// Output the SVG header information.
{
	// Print the svg end.
	fputs(rvSvgEnd, fp);
}


void rvSvg_TableGrid8x8(FILE *fp, char *comment, rvUint16 xpos, rvUint16 ypos, rvUint8 *values, rvUint16 count)
// Output an 8x8 grid from the values.  64 values must be present to
// fully populate the grid.
{
	rvUint16 i;
	rvUint16 ptXpos = (xpos * 135);
	rvUint16 ptYpos = (ypos * 135); 

	// Print the comment if we have one.
	if (comment != NULL) fprintf(fp, rvSvgComment2, comment);

	// Put the top, bottom, left and right border rectangles.
	fprintf(fp, rvSvgRect, ptXpos, ptYpos, 90, 9);
	fprintf(fp, rvSvgRect, ptXpos, ptYpos + 81, 90, 9);
	fprintf(fp, rvSvgRect, ptXpos, ptYpos + 9, 9, 72);
	fprintf(fp, rvSvgRect, ptXpos + 81, ptYpos + 9, 9, 72);

	// Fill in the grid rectangles.
	for (i = 0; i < count; ++i)
	{
		// Is there a bit position at this value?
		if (!(values[i] & 0x01))
		{
			rvUint16 x = ((i % 8) * 9) + 9;
			rvUint16 y = ((i / 8) * 9) + 9;
			
			// No. Output a black rectangle for each zero value.
			fprintf(fp, rvSvgRect, (int) ptXpos + x, (int) ptYpos + y, 9, 9);
		}
	}

	fprintf(fp, "\n");
}


void rvSvg_Grid8x8(char *filename, rvUint8 *values, rvUint16 count)
// Output an 8x8 grid from the values.  64 values must be present to
// fully populate the grid.
{
	FILE *fp = NULL;

	// Open the file for writing.
	if (fopen_s(&fp, filename, "w") == 0)
	{
		// Print the grid header.
		rvSvg_TableBegin(fp, filename, 1, 1);

		// Print the grid.
		rvSvg_TableGrid8x8(fp, NULL, 0, 0, values, count);

		// Print the grid footer.
		rvSvg_TableEnd(fp);

		// Close the file.
		fclose(fp);
	}
}

