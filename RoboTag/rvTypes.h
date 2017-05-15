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

    $Id: rvTypes.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_TYPES_INCLUDED
#define _RV_TYPES_INCLUDED

// Basic 8, 16, and 32 bit integer types.
typedef char                rvInt8;
typedef short               rvInt16;
typedef long                rvInt32;
typedef unsigned char       rvUint8;
typedef unsigned short      rvUint16;
typedef unsigned long       rvUint32;

// 64 bit integer types are platform/compiler specific.
#if defined(_MSC_VER)
typedef __int64             rvInt64;
typedef unsigned __int64    rvUint64;
#elif defined(__GNUC__)
typedef long long           rvInt64;
typedef unsigned long long  rvUint64;
#endif

// A boolean type to match the C++ definition of "bool".
#ifndef __cplusplus
typedef char bool;

#define false ((bool)0)
#define true  ((bool)1)
#endif

// Define NULL if not defined already.
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

// Platform/compiler specific handling of unreferenced parameter.
#if defined(_MSC_VER)
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)   (P)
#endif
#elif defined(__GNUC__)
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)   {;}
#endif
#endif

#endif
