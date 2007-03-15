/*
    generate_simple.c - Part of psiconv, a PSION 5 file formats converter
    Copyright (c) 2000  Frodo Looijaard <frodol@dds.nl>

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

#include "config.h"
#include "compat.h"

#include "generate_routines.h"
#include "error.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


int psiconv_write_u8(psiconv_buffer buf,const psiconv_u8 value)
{
  return psiconv_buffer_add(buf,value);
}

int psiconv_write_u16(psiconv_buffer buf,const psiconv_u16 value)
{
  int res;
  if ((res = psiconv_buffer_add(buf,value & 0xff))) 
	return res;
  return psiconv_buffer_add(buf,(value & 0xff00) >> 8);
}

int psiconv_write_u32(psiconv_buffer buf,const psiconv_u32 value)
{
  int res;
  if ((res = psiconv_buffer_add(buf,value & 0xff))) 
	return res;
  if ((res = psiconv_buffer_add(buf,(value & 0xff00) >> 8))) 
	return res;
  if ((res = psiconv_buffer_add(buf,(value & 0xff0000) >> 16))) 
	return res;
  return psiconv_buffer_add(buf,(value & 0xff000000) >> 24);
}

int psiconv_write_S(psiconv_buffer buf, const psiconv_u32 value)
{
  if (value < 0x40) 
    return psiconv_write_u8(buf,value * 4 + 2);
  else if (value < 0x2000) 
    return psiconv_write_u16(buf,value * 8 + 3);
  else {
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "Don't know how to write S value larger than 0x2000 "
                 "(trying %x)",value);
    return -PSICONV_E_GENERATE;
  }
}

int psiconv_write_X(psiconv_buffer buf, const psiconv_u32 value)
{
  if (value < 0x80) 
    return psiconv_write_u8(buf,value * 2);
  else if (value < 0x4000) 
    return psiconv_write_u16(buf,value * 4 + 1);
  else if (value < 0x20000000) 
    return psiconv_write_u16(buf,value * 8 + 3);
  else {
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "Don't know how to write X value larger than 0x20000000 "
                 "(trying %x)",value);
    return -PSICONV_E_GENERATE;
  }
}

int psiconv_write_length(psiconv_buffer buf, const psiconv_length_t value)
{
  return psiconv_write_u32(buf,value * (1440.0/2.54) + 0.5);
}

int psiconv_write_size(psiconv_buffer buf, psiconv_size_t value)
{
  return psiconv_write_u32(buf,value * 20.0 + 0.5);
}

int psiconv_write_bool(psiconv_buffer buf, const psiconv_bool_t value)
{
  if ((value != psiconv_bool_true) && (value != psiconv_bool_false)) 
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "Boolean has non-enum value (found %d)",value);
  return psiconv_write_u8(buf,value == psiconv_bool_false?0:1);
}

int psiconv_write_string(psiconv_buffer buf, const psiconv_string_t value)
{
  int res,i;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "NULL string");
    return -PSICONV_E_GENERATE;
  }
  if ((res = psiconv_write_S(buf,strlen(value))))
    return res;
  for (i = 0; i < strlen(value); i++) 
    if ((res = psiconv_write_u8(buf,value[i])))
      return res;
  return -PSICONV_E_OK;
}

int psiconv_write_offset(psiconv_buffer buf, psiconv_u32 id)
{
  return psiconv_buffer_add_reference(buf,id);
}
