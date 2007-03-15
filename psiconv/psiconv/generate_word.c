/*
    generate_word.c - Part of psiconv, a PSION 5 file formats converter
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


int psiconv_write_word_status_section(psiconv_buffer buf,
                                         psiconv_word_status_section value)
{
  int res;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null word status section");
    return -PSICONV_E_GENERATE;
  }

  if ((res = psiconv_write_u8(buf,0x02)))
    return res;
  if ((res = psiconv_write_u8(buf,(value->show_tabs?0x01:0x00) |
                              (value->show_spaces?0x02:0x00) |
                              (value->show_paragraph_ends?0x04:0x00) |
                              (value->show_line_breaks?0x08:0x00) |
                              (value->show_hard_minus?0x20:0x00) |
                              (value->show_hard_space?0x40:0x00))))
    return res;
  if ((res = psiconv_write_u8(buf,(value->show_full_pictures?0x01:0x00) | 
                              (value->show_full_graphs?0x02:0x00))))
    return res;
  if ((res = psiconv_write_bool(buf,value->show_top_toolbar)))
    return res;
  if ((res = psiconv_write_bool(buf,value->show_side_toolbar)))
    return res;
  if ((res = psiconv_write_u8(buf,(value->fit_lines_to_screen?0x08:0x00))))
    return res;
  if ((res = psiconv_write_u32(buf,value->cursor_position)))
    return res;
  return psiconv_write_u32(buf,value->display_size);
}

int psiconv_write_word_styles_section(psiconv_buffer buf,
                                      psiconv_word_styles_section value)
{
  int res,i;
  psiconv_word_style style;
  psiconv_paragraph_layout basepara;
  psiconv_character_layout basechar;
  psiconv_font font;


  if (!value || !value->normal || !value->styles) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null word styles section");
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


  if ((res = psiconv_write_paragraph_layout_list(buf,value->normal->paragraph,
                                                 basepara)))
    goto ERROR3;
  /* Always generate the font for Normal */
  font = basechar->font;
  basechar->font = NULL;
  res = psiconv_write_character_layout_list(buf,value->normal->character,
                                                 basechar);
  basechar->font = font;
  if (res)
    goto ERROR3;
  if ((res = psiconv_write_u32(buf,value->normal->hotkey)))
    goto ERROR3;
  if ((res = psiconv_write_u8(buf,psiconv_list_length(value->styles))))
    goto ERROR3;



  for (i = 0; i < psiconv_list_length(value->styles); i++) {
    if (!(style = psiconv_list_get(value->styles,i))) {
      psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
      res = -PSICONV_E_OTHER;
      goto ERROR3;
    }
    if ((res = psiconv_write_u32(buf,style->hotkey)))
      goto ERROR3;
  }
  if ((res = psiconv_write_u8(buf,psiconv_list_length(value->styles))))
    goto ERROR3;
  for (i = 0; i < psiconv_list_length(value->styles); i++) {
    if (!(style = psiconv_list_get(value->styles,i))) {
      psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
      res = -PSICONV_E_OTHER;
      goto ERROR3;
    }
    if (!style->name) {
      psiconv_warn(0,psiconv_buffer_length(buf),"Null style name");
      res = -PSICONV_E_GENERATE;
      goto ERROR3;
    }
    if ((res = psiconv_write_string(buf,style->name)))
      goto ERROR3;
    if ((res = psiconv_write_u32(buf,style->built_in?PSICONV_ID_STYLE_BUILT_IN:
                                                 PSICONV_ID_STYLE_REMOVABLE)))
      goto ERROR3;
    if ((res = psiconv_write_u32(buf,style->outline_level)))
      goto ERROR3;
    if ((res = psiconv_write_character_layout_list(buf,style->character,
                                                   value->normal->character)))
      goto ERROR3;
    if ((res = psiconv_write_paragraph_layout_list(buf,style->paragraph,
                                                   value->normal->paragraph)))
      goto ERROR3;
  }

  for (i = 0; i < psiconv_list_length(value->styles); i++)
    if ((res = psiconv_write_u8(buf,0xff)))
      goto ERROR3;
  res = -PSICONV_E_OK;

ERROR3:
  psiconv_free_character_layout(basechar);
ERROR2:
  psiconv_free_paragraph_layout(basepara);
ERROR1:
  return res;
}
