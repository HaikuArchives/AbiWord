/*
    generate_page.c - Part of psiconv, a PSION 5 file formats converter
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
#include <string.h>

#include "generate_routines.h"
#include "error.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif



int psiconv_write_page_header(psiconv_buffer buf, 
                              const psiconv_page_header value,
                              psiconv_buffer *extra_buf)
{
  int res;
  psiconv_paragraph_layout basepara;
  psiconv_character_layout basechar;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null page header");
    res = -PSICONV_E_GENERATE;
    goto ERROR1;
  }

  if (!(basepara=psiconv_basic_paragraph_layout())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR1;
  }
  if (!(basechar=psiconv_basic_character_layout())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR2;
  }

  /* Unknown byte */
  if ((res = psiconv_write_u8(buf,0x01)))
    goto ERROR3;
  if ((res = psiconv_write_bool(buf,value->on_first_page)))
    goto ERROR3;
  /* Three unknown bytes */
  if ((res = psiconv_write_u8(buf,0x00)))
    goto ERROR3;
  if ((res = psiconv_write_u8(buf,0x00)))
    goto ERROR3;
  if ((res = psiconv_write_u8(buf,0x00)))
    goto ERROR3;
  if ((res = psiconv_write_paragraph_layout_list(buf,
                                    value->base_paragraph_layout,basepara)))
    goto ERROR3;
  if ((res = psiconv_write_character_layout_list(buf,
                                    value->base_character_layout,basechar)))
    goto ERROR3;
  res =  psiconv_write_texted_section(buf,value->text,
                                      value->base_character_layout,
                                      value->base_paragraph_layout,extra_buf);
ERROR3:
  psiconv_free_character_layout(basechar);
ERROR2:
  psiconv_free_paragraph_layout(basepara);
ERROR1:
  return res;
}

int psiconv_write_page_layout_section(psiconv_buffer buf, 
                                      const psiconv_page_layout_section value)
{
  int res;
  psiconv_buffer header_buf,footer_buf;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null page section");
    res = -PSICONV_E_GENERATE;
    goto ERROR1;
  }

  if ((res = psiconv_write_u32(buf,value->first_page_nr)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->header_dist)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->footer_dist)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->left_margin)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->right_margin)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->top_margin)))
    goto ERROR1;
  if ((res = psiconv_write_length(buf,value->bottom_margin)))
    goto ERROR1;
  if ((res = psiconv_write_page_header(buf,value->header,&header_buf)))
    goto ERROR1;
  if ((res = psiconv_write_page_header(buf,value->footer,&footer_buf)))
    goto ERROR2;
  if ((res = psiconv_write_u32(buf,PSICONV_ID_PAGE_DIMENSIONS2)))
    goto ERROR3;
  if ((res = psiconv_write_length(buf,value->page_width)))
    goto ERROR3;
  if ((res =  psiconv_write_length(buf,value->page_height)))
    goto ERROR3;
  if ((res = psiconv_write_bool(buf,value->landscape)))
    goto ERROR3;
  if ((res = psiconv_buffer_concat(buf,header_buf)))
    goto ERROR3;
  res = psiconv_buffer_concat(buf,footer_buf);


ERROR3:
  psiconv_buffer_free(footer_buf);
ERROR2:
  psiconv_buffer_free(header_buf);
ERROR1:
  return res;
}
