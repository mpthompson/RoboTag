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

    $Id: rvSvg.h 30 2010-03-11 17:57:43Z mike $
*/

#ifndef _RV_SVG_INCLUDED_
#define _RV_SVG_INCLUDED_

#include <stdlib.h>
#include <stdio.h>
#include "rvTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// SVG methods.
void rvSvg_TableBegin(FILE *fp, char *comment, rvUint16 width, rvUint16 height);
void rvSvg_TableEnd(FILE *fp);
void rvSvg_TableGrid8x8(FILE *fp, char *comment, rvUint16 xpos, rvUint16 ypos, rvUint8 *values, rvUint16 count);
void rvSvg_Grid8x8(char *filename, rvUint8 *values, rvUint16 count);

#ifdef __cplusplus
} // "C"
#endif

#endif // _RV_SVG_INCLUDED_

