/*
    generate_layout.c - Part of psiconv, a PSION 5 file formats converter
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


int psiconv_write_color(psiconv_buffer buf, const psiconv_color value)
{
  int res;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null color");
    return -PSICONV_E_GENERATE;
  }
  if ((res = psiconv_write_u8(buf,value->red))) 
    return res;
  if ((res = psiconv_write_u8(buf,value->green))) 
    return res;
  return  psiconv_write_u8(buf,value->blue);
}

int psiconv_write_font(psiconv_buffer buf, const psiconv_font value)
{
  int res,i;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null font");
    return -PSICONV_E_GENERATE;
  }
  if ((res = psiconv_write_u8(buf,strlen(value->name)+1)))
    return res;
  for (i = 0; i < strlen(value->name); i++)
    if ((res = psiconv_write_u8(buf,value->name[i])))
      return res;
  return psiconv_write_u8(buf,value->screenfont);
}

int psiconv_write_border(psiconv_buffer buf, const psiconv_border value)
{
  int res;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null border");
    return -PSICONV_E_GENERATE;
  }
  if (value->kind > psiconv_border_dotdotdashed) 
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "Unknown border kind (%d); assuming none",value->kind);
  if ((res =psiconv_write_u8(buf,value->kind == psiconv_border_none?0:
                                 value->kind == psiconv_border_solid?1:
                                 value->kind == psiconv_border_double?2:
                                 value->kind == psiconv_border_dotted?3:
                                 value->kind == psiconv_border_dashed?4:
                                 value->kind == psiconv_border_dotdashed?5:
                                 value->kind == psiconv_border_dotdotdashed?6:
                                                0)))
    return res;
  if ((res = psiconv_write_size(buf,(value->kind == psiconv_border_solid) ||
                                    (value->kind == psiconv_border_double) ?
                                    value->thickness:1.0/20.0)))
    return res;
  if ((res = psiconv_write_color(buf,value->color)))
    return res;
  /* Unknown byte */
  return psiconv_write_u8(buf,1);
}

int psiconv_write_bullet(psiconv_buffer buf, const psiconv_bullet value)
{
  int res;
  psiconv_buffer extra_buf;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null bullet");
    return -PSICONV_E_GENERATE;
  }

  if (!(extra_buf = psiconv_buffer_new()))
    return -PSICONV_E_NOMEM;
  if ((res = psiconv_write_size(extra_buf,value->font_size)))
    goto ERROR;
  if ((res = psiconv_write_u8(extra_buf,value->character)))
    goto ERROR;
  if ((res = psiconv_write_bool(extra_buf,value->indent)))
    goto ERROR;
  if ((res = psiconv_write_color(extra_buf,value->color)))
    goto ERROR;
  if ((res = psiconv_write_font(extra_buf,value->font)))
    goto ERROR;

  if ((res = psiconv_write_u8(buf,psiconv_buffer_length(extra_buf))))
    goto ERROR;
  res = psiconv_buffer_concat(buf,extra_buf);
  
ERROR:
  psiconv_buffer_free(extra_buf);
  return res;
}

int psiconv_write_tab(psiconv_buffer buf,psiconv_tab value)
{
  int res;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null tab");
    return -PSICONV_E_GENERATE;
  }
  if ((res = psiconv_write_length(buf,value->location)))
    return res;
  if ((value->kind != psiconv_tab_left) && 
      (value->kind != psiconv_tab_right) &&
      (value->kind != psiconv_tab_centre)) 
    psiconv_warn(0,psiconv_buffer_length(buf),
                 "Unknown tab kind (%d); assuming left",value->kind);
  return psiconv_write_u8(buf, value->kind == psiconv_tab_right?2:
                               value->kind == psiconv_tab_centre?3:1);
}

int psiconv_write_paragraph_layout_list(psiconv_buffer buf, 
                                        psiconv_paragraph_layout value,
                                        psiconv_paragraph_layout base)
{
  int res,i;
  psiconv_buffer extra_buf;
  psiconv_tab tab;
  
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null paragraph layout list");
    return -PSICONV_E_GENERATE;
  }
  if (!(extra_buf = psiconv_buffer_new()))
    return -PSICONV_E_NOMEM;
  
  if (!base || psiconv_compare_color(base->back_color,value->back_color)) {
    if ((res = psiconv_write_u8(extra_buf,0x01)))
      goto ERROR;
    if ((res = psiconv_write_color(extra_buf,value->back_color)))
      goto ERROR;
  }

  if (!base || (value->indent_left != base->indent_left)) {
    if ((res = psiconv_write_u8(extra_buf,0x02)))
      goto ERROR;
    if ((res = psiconv_write_length(extra_buf,value->indent_left)))
      goto ERROR;
  }

  if (!base || (value->indent_right != base->indent_right)) {
    if ((res = psiconv_write_u8(extra_buf,0x03)))
      goto ERROR;
    if ((res = psiconv_write_length(extra_buf,value->indent_right)))
      goto ERROR;
  }

  if (!base || (value->indent_first != base->indent_first)) {
    if ((res = psiconv_write_u8(extra_buf,0x04)))
      goto ERROR;
    if ((res = psiconv_write_length(extra_buf,value->indent_first)))
      goto ERROR;
  }

  if (!base || (value->justify_hor != base->justify_hor)) {
    if ((res = psiconv_write_u8(extra_buf,0x05)))
      goto ERROR;
    if ((value->justify_hor < psiconv_justify_left) ||
        (value->justify_hor > psiconv_justify_full))
      psiconv_warn(0,psiconv_buffer_length(buf),
                   "Unknown horizontal justify (%d); assuming left",
                   value->justify_hor);
    if ((res = psiconv_write_u8(extra_buf,
               value->justify_hor == psiconv_justify_centre?1:
               value->justify_hor == psiconv_justify_right?2:
               value->justify_hor == psiconv_justify_full?3:0)))
      goto ERROR;
  }

  if (!base || (value->justify_ver != base->justify_ver)) {
    if ((res = psiconv_write_u8(extra_buf,0x06)))
      goto ERROR;
    if ((value->justify_ver < psiconv_justify_top) ||
        (value->justify_ver > psiconv_justify_bottom))
      psiconv_warn(0,psiconv_buffer_length(buf),
                   "Unknown vertical justify (%d); assuming middle",
                    value->justify_ver);
    if ((res = psiconv_write_u8(extra_buf,
               value->justify_ver == psiconv_justify_centre?1:
               value->justify_ver == psiconv_justify_right?2:0)))
      goto ERROR;
  }

  if (!base || (value->linespacing != base->linespacing)) {
    if ((res = psiconv_write_u8(extra_buf,0x07)))
      goto ERROR;
    if ((res = psiconv_write_size(extra_buf,value->linespacing)))
      goto ERROR;
  }

  if (!base || (value->linespacing_exact != base->linespacing_exact)) {
    if ((res = psiconv_write_u8(extra_buf,0x08)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->linespacing_exact)))
      goto ERROR;
  }

  if (!base || (value->space_above != base->space_above)) {
    if ((res = psiconv_write_u8(extra_buf,0x09)))
      goto ERROR;
    if ((res = psiconv_write_size(extra_buf,value->space_above)))
      goto ERROR;
  }

  if (!base || (value->space_below != base->space_below)) {
    if ((res = psiconv_write_u8(extra_buf,0x0a)))
      goto ERROR;
    if ((res = psiconv_write_size(extra_buf,value->space_below)))
      goto ERROR;
  }

  if (!base || (value->keep_together != base->keep_together)) {
    if ((res = psiconv_write_u8(extra_buf,0x0b)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->keep_together)))
      goto ERROR;
  }

  if (!base || (value->keep_with_next != base->keep_with_next)) {
    if ((res = psiconv_write_u8(extra_buf,0x0c)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->keep_with_next)))
      goto ERROR;
  }

  if (!base || (value->on_next_page != base->on_next_page)) {
    if ((res = psiconv_write_u8(extra_buf,0x0d)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->on_next_page)))
      goto ERROR;
  }

  if (!base || (value->no_widow_protection != base->no_widow_protection)) {
    if ((res = psiconv_write_u8(extra_buf,0x0e)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->no_widow_protection)))
      goto ERROR;
  }

  if (!base || (value->wrap_to_fit_cell != base->wrap_to_fit_cell)) {
    if ((res = psiconv_write_u8(extra_buf,0x0f)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->wrap_to_fit_cell)))
      goto ERROR;
  }

  if (!base || (value->border_distance != base->border_distance)) {
    if ((res = psiconv_write_u8(extra_buf,0x10)))
      goto ERROR;
    if ((res = psiconv_write_length(extra_buf,value->border_distance)))
      goto ERROR;
  }

  if (!base || psiconv_compare_border(value->top_border,base->top_border)) {
    if ((res = psiconv_write_u8(extra_buf,0x11)))
      goto ERROR;
    if ((res = psiconv_write_border(extra_buf,value->top_border))) 
      goto ERROR;
  }

  if (!base || psiconv_compare_border(value->bottom_border,
                                       base->bottom_border)) {
    if ((res = psiconv_write_u8(extra_buf,0x12)))
      goto ERROR;
    if ((res = psiconv_write_border(extra_buf,value->bottom_border)))
      goto ERROR;
  }

  if (!base || psiconv_compare_border(value->left_border,
                                       base->left_border)) {
    if ((res = psiconv_write_u8(extra_buf,0x13)))
      goto ERROR;
    if ((res = psiconv_write_border(extra_buf,value->left_border)))
      goto ERROR;
  }

  if (!base || psiconv_compare_border(value->right_border,
                                       base->right_border)) {
    if ((res = psiconv_write_u8(extra_buf,0x14))) 
      goto ERROR;
    if ((res = psiconv_write_border(extra_buf,value->right_border)))
      goto ERROR;
  }

  if (!base || psiconv_compare_bullet(value->bullet,
                                       base->bullet)) {
    if ((res = psiconv_write_u8(extra_buf,0x15))) 
      goto ERROR;
    if ((res = psiconv_write_bullet(extra_buf,value->bullet)))
      goto ERROR;
  }

  if (!value->tabs || !value->tabs->extras) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null tabs");
    res = -PSICONV_E_GENERATE;
    goto ERROR;
  } 

  /* It is not entirely clear how tabs are inherited. For now, I assume
     if there is any difference at all, we will have to generate both
     the normal tab-interval, and all specific tabs */
  if (!base || psiconv_compare_all_tabs(value->tabs,base->tabs)) {
    if ((res = psiconv_write_u8(extra_buf,0x16))) 
      goto ERROR;
    if ((res = psiconv_write_length(extra_buf,value->tabs->normal)))
      goto ERROR;
    for (i = 0; i < psiconv_list_length(value->tabs->extras); i++) {
      if (!(tab = psiconv_list_get(value->tabs->extras,i))) {
        psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
        res = -PSICONV_E_NOMEM;
        goto ERROR;
      }
      if ((res = psiconv_write_u8(extra_buf,0x17)))
        goto ERROR;
      if ((res = psiconv_write_tab(extra_buf,tab)))
        goto ERROR;
    }
  }

  if ((res = psiconv_write_u32(buf,psiconv_buffer_length(extra_buf))))
    goto ERROR;

  res = psiconv_buffer_concat(buf,extra_buf);

ERROR:
  psiconv_buffer_free(extra_buf);
  return res;
}

int psiconv_write_character_layout_list(psiconv_buffer buf, 
                                        psiconv_character_layout value,
                                        psiconv_character_layout base)
{
  int res;
  psiconv_buffer extra_buf;
  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null character layout list");
    return -PSICONV_E_GENERATE;
  }
  if (!(extra_buf = psiconv_buffer_new()))
    return -PSICONV_E_NOMEM;

  if (!base || psiconv_compare_color(base->color,value->color)) {
    if ((res = psiconv_write_u8(extra_buf,0x19)))
      goto ERROR;
    if ((res = psiconv_write_color(extra_buf,value->color)))
      goto ERROR;
  }

  if (!base || psiconv_compare_color(base->back_color,value->back_color)) {
    if ((res = psiconv_write_u8(extra_buf,0x1a)))
      goto ERROR;
    if ((res = psiconv_write_color(extra_buf,value->back_color)))
      goto ERROR;
  }

  if (!base || (value->font_size != base->font_size)) {
    if ((res = psiconv_write_u8(extra_buf,0x1c)))
      goto ERROR;
    if ((res = psiconv_write_size(extra_buf,value->font_size)))
      goto ERROR;
  }

  if (!base || (value->italic != base->italic)) {
    if ((res = psiconv_write_u8(extra_buf,0x1d)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->italic)))
      goto ERROR;
  }

  if (!base || (value->bold != base->bold)) {
    if ((res = psiconv_write_u8(extra_buf,0x1e)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->bold)))
      goto ERROR;
  }

  if (!base || (value->super_sub != base->super_sub)) {
    if ((value->super_sub != psiconv_superscript) &&
        (value->super_sub != psiconv_subscript) &&
        (value->super_sub != psiconv_normalscript))
      psiconv_warn(0,psiconv_buffer_length(buf),
                   "Unknown supersubscript (%d); assuming normal",
                   value->super_sub);
    if ((res = psiconv_write_u8(extra_buf,0x1f)))
      goto ERROR;
    if ((res = psiconv_write_u8(extra_buf,
                                value->super_sub == psiconv_superscript?1:
                                value->super_sub == psiconv_subscript?2:0)))
      goto ERROR;
  }

  if (!base || (value->underline != base->underline)) {
    if ((res = psiconv_write_u8(extra_buf,0x20)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->underline)))
      goto ERROR;
  }

  if (!base || (value->strikethrough != base->strikethrough)) {
    if ((res = psiconv_write_u8(extra_buf,0x21)))
      goto ERROR;
    if ((res = psiconv_write_bool(extra_buf,value->strikethrough)))
      goto ERROR;
  }

  if (!base || psiconv_compare_font(base->font,value->font)) {
    if ((res = psiconv_write_u8(extra_buf,0x22)))
      goto ERROR;
    if ((res = psiconv_write_font(extra_buf,value->font)))
      goto ERROR;
  }

  if ((res = psiconv_write_u32(buf,psiconv_buffer_length(extra_buf))))
    goto ERROR;

  res = psiconv_buffer_concat(buf,extra_buf);

ERROR:
  psiconv_buffer_free(extra_buf);
  return res;
}
