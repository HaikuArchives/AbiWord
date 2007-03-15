/*
    data.h - Part of psiconv, a PSION 5 file formats converter
    Copyright (c) 1999  Frodo Looijaard <frodol@dds.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* All declarations which you may need to edit are here. At a later time,
   this will be superseded by some automatic configuration script. */

#ifndef PSICONV_GENERAL_H
#define PSICONV_GENERAL_H

/* Data types; s8 means `signed 8-bit integer', u32 means `unsigned 32-bits
   integer'. Configure figures out which types to use.
*/

typedef signed char psiconv_s8;
typedef unsigned char psiconv_u8;

typedef signed short psiconv_s16;
typedef unsigned short psiconv_u16;

typedef signed int psiconv_s32;
typedef unsigned int psiconv_u32;

#endif /* def PSICONV_GENERAL_H */
