/*
    generate_texted.c - Part of psiconv, a PSION 5 file formats converter
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

#include <stdlib.h>

#include "generate_routines.h"
#include "error.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif



int psiconv_write_texted_section(psiconv_buffer buf, 
                                 const psiconv_texted_section value,
                                 const psiconv_character_layout base_char,
                                 const psiconv_paragraph_layout base_para,
                                 psiconv_buffer *extra_buf)
{
  int res,with_layout_section=0;
  psiconv_u32 layout_id;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null TextEd section");
    res = -PSICONV_E_GENERATE;
    goto ERROR1;
  }

  if (!(*extra_buf = psiconv_buffer_new())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR1;
  }

  layout_id = psiconv_buffer_unique_id();
  psiconv_buffer_add_target(*extra_buf,layout_id);

  if (psiconv_list_length(value->paragraphs)) {
    with_layout_section = 1;
    if ((res = psiconv_write_styleless_layout_section(*extra_buf,
                       value->paragraphs,
                       base_char,base_para)))
      goto ERROR2;
  } 

  if ((res = psiconv_write_u32(buf,PSICONV_ID_TEXTED_BODY)))
    goto ERROR2;
  
  /* Partly dummy TextEd Jumptable */
  if ((res = psiconv_write_u32(buf,PSICONV_ID_TEXTED_REPLACEMENT)))
    goto ERROR2;
  if ((res = psiconv_write_u32(buf,0)))
    goto ERROR2;
  if ((res = psiconv_write_u32(buf,PSICONV_ID_TEXTED_UNKNOWN)))
    goto ERROR2;
  if ((res = psiconv_write_u32(buf,0)))
    goto ERROR2;
  if ((res = psiconv_write_u32(buf,PSICONV_ID_TEXTED_LAYOUT)))
    goto ERROR2;
  if (with_layout_section) {
    if ((res = psiconv_write_offset(buf,layout_id)))
      goto ERROR2;
  } else {
    if ((res = psiconv_write_u32(buf,0)))
      goto ERROR2;
  }

  if ((res = psiconv_write_u32(buf,PSICONV_ID_TEXTED_TEXT)))
    goto ERROR2;

  return psiconv_write_text_section(buf,value->paragraphs);

ERROR2:
  psiconv_buffer_free(*extra_buf);
ERROR1:
  return res;
}
