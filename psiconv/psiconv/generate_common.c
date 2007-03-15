/*
    generate_common.c - Part of psiconv, a PSION 5 file formats converter
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

static int psiconv_write_layout_section(psiconv_buffer buf,
                           const psiconv_text_and_layout value,
                           const psiconv_word_styles_section styles,
                           int with_styles);

/* Maybe use a psiconv_header_section variable instead? */
int psiconv_write_header_section(psiconv_buffer buf,psiconv_u32 uid1,
                                 psiconv_u32 uid2, psiconv_u32 uid3)
{
  int res;
  if ((res = psiconv_write_u32(buf,uid1)))
    return res;
  if ((res = psiconv_write_u32(buf,uid2)))
    return res;
  if ((res = psiconv_write_u32(buf,uid3)))
    return res;
  return psiconv_write_u32(buf,psiconv_checkuid(uid1,uid2,uid3));
}

int psiconv_write_section_table_section(psiconv_buffer buf, 
                                    const psiconv_section_table_section value)
{
  int res,i;
  psiconv_section_table_entry entry;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null section table section");
    return -PSICONV_E_GENERATE;
  }

  if ((res = psiconv_write_u8(buf,2 * psiconv_list_length(value))))
    return res;
  for (i = 0; i < psiconv_list_length(value); i++) {
    if (!(entry = psiconv_list_get(value,i))) {
      psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
      return -PSICONV_E_NOMEM;
    }
    if ((res = psiconv_write_u32(buf,entry->id)))
      return res;
    if ((res = psiconv_write_offset(buf,entry->offset)))
      return res;
  }
  return -PSICONV_E_OK;
}

int psiconv_write_application_id_section(psiconv_buffer buf,psiconv_u32 id,
                                        const psiconv_string_t text)
{
  int res;
  if ((res = psiconv_write_u32(buf,id)))
    return res;
  return psiconv_write_string(buf,text);
}

int psiconv_write_text_section(psiconv_buffer buf,
                           const psiconv_text_and_layout value)
{
  int res;
  psiconv_buffer extra_buf;
  int i,j;
  psiconv_paragraph paragraph;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null text section");
    return -PSICONV_E_GENERATE;
  }

  if (psiconv_list_length(value)) {
    if (!(extra_buf = psiconv_buffer_new())) 
      return -PSICONV_E_NOMEM;
    for (i = 0; i < psiconv_list_length(value); i++) {
      if (!(paragraph = psiconv_list_get(value,i))) {
        psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
        res = -PSICONV_E_OTHER;
        goto ERROR;
      }
      for (j = 0; j < strlen(paragraph->text); j++) 
        if ((res = psiconv_write_u8(extra_buf,paragraph->text[j])))
          goto ERROR;
      psiconv_write_u8(extra_buf,0x06);
    }
    if ((res = psiconv_write_X(buf,psiconv_buffer_length(extra_buf))))
      goto ERROR;
    res = psiconv_buffer_concat(buf,extra_buf);
  } else 
    /* Hack: empty text sections are just not allowed */
    return psiconv_write_u16(buf,0x0602);

ERROR:  
  psiconv_buffer_free(extra_buf);
  return res;
}

int psiconv_write_layout_section(psiconv_buffer buf,
                           const psiconv_text_and_layout value,
                           const psiconv_word_styles_section styles,
                           int with_styles)
{
  typedef struct psiconv_paragraph_type_list_s
  {
    psiconv_character_layout character;
    psiconv_paragraph_layout paragraph;
    psiconv_u8 style;
    psiconv_u8 nr;
  } *psiconv_paragraph_type_list;
  psiconv_list paragraph_type_list; /* Of psiconv_paragraph_type_list_s */
  psiconv_paragraph_type_list paragraph_type;
  struct psiconv_paragraph_type_list_s new_type;
  psiconv_buffer buf_types,buf_elements,buf_inlines;
  psiconv_paragraph paragraph;
  psiconv_in_line_layout in_line;
  psiconv_word_style style;
  psiconv_character_layout para_charlayout;
  int i,j,para_type,nr_of_inlines=0,res,ptl_length,pel_length,thislen,paralen;

  if (!value) {
    psiconv_warn(0,psiconv_buffer_length(buf),"Null text section");
    return -PSICONV_E_GENERATE;
  }

  if (!(paragraph_type_list = psiconv_list_new(sizeof(new_type)))) {
    res = -PSICONV_E_NOMEM;
    goto ERROR1;
  }

  if (!(buf_types = psiconv_buffer_new())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR2;
  }

  if (!(buf_elements = psiconv_buffer_new())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR3;
  }

  if (!(buf_inlines = psiconv_buffer_new())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR4;
  }

  for (i = 0; i < psiconv_list_length(value); i++) {
    if (!(paragraph = psiconv_list_get(value,i))) {
      psiconv_warn(0,psiconv_buffer_length(buf),"Massive memory corruption");
      res = -PSICONV_E_OTHER;
      goto ERROR5;
    }
    if ((res = psiconv_write_u32(buf_elements,strlen(paragraph->text)+1)))
      goto ERROR5;

    if (psiconv_list_length(paragraph->in_lines) > 1) {
      /* Inline layouts, so we generate a paragraph element and inline 
         elements */
      if ((res = psiconv_write_u8(buf_elements,0x00)))
        goto ERROR5;
      if (!(style = psiconv_get_style(styles,paragraph->base_style))) {
        psiconv_warn(0,psiconv_buffer_length(buf),"Unknown style");
        res = -PSICONV_E_GENERATE;
        goto ERROR5;
      }
      if ((res = psiconv_write_paragraph_layout_list(buf_elements,
                                                     paragraph->base_paragraph,
                                                     style->paragraph)))
        goto ERROR5;
      if (with_styles)
        if ((res = psiconv_write_u8(buf_elements,paragraph->base_style)))
          goto ERROR5;
      if ((res = psiconv_write_u32(buf_elements,
                                   psiconv_list_length(paragraph->in_lines))))
         goto ERROR5;

      /* Generate the inlines. NB: Against what are all settings relative?!? */
      paralen = 0;
      for (j = 0; j < psiconv_list_length(paragraph->in_lines); j++) {
        nr_of_inlines ++;
        if (!(in_line = psiconv_list_get(paragraph->in_lines,j))) {
          psiconv_warn(0,psiconv_buffer_length(buf),
                       "Massive memory corruption");
          res = -PSICONV_E_OTHER;
          goto ERROR5;
        }
        if ((res = psiconv_write_u8(buf_inlines,0x00)))
          goto ERROR5;
        thislen = in_line->length;
        paralen += thislen;
        /* If this is the last in_line, we need to make sure that the
           complete length of all inlines equals the text length */
        if (j == psiconv_list_length(paragraph->in_lines)-1) {
          if (paralen > strlen(paragraph->text)+1) {
            res = -PSICONV_E_GENERATE;
            goto ERROR5;
          }
          thislen += strlen(paragraph->text)+1-paralen;
        }
        if ((res = psiconv_write_u32(buf_inlines,thislen)))
          goto ERROR5;
        if ((res = psiconv_write_character_layout_list(buf_inlines,
                                                     in_line->layout,
                                                     style->character)))
          goto ERROR5;
      } 
    } else {
      /* No inline layouts (or only 1), so we generate a paragraph type list */
      para_type = 0;
      /* Set para_charlayout to the correct character-level layout */
      if (psiconv_list_length(paragraph->in_lines) == 0)
        para_charlayout = paragraph->base_character;
      else {
        if (!(in_line = psiconv_list_get(paragraph->in_lines,0))) {
          psiconv_warn(0,psiconv_buffer_length(buf),
                       "Massive memory corruption");
          res = -PSICONV_E_OTHER;
          goto ERROR5;
        } 
        para_charlayout = in_line->layout;
      }
      for (j = 0; j < psiconv_list_length(paragraph_type_list); j++) {
        if (!(paragraph_type = psiconv_list_get(paragraph_type_list,j))) {
          psiconv_warn(0,psiconv_buffer_length(buf),
                       "Massive memory corruption");
          res = -PSICONV_E_OTHER;
          goto ERROR5;
        }
        if ((paragraph->base_style == paragraph_type->style) &&
            !psiconv_compare_character_layout(para_charlayout,
                                              paragraph_type->character) &&
            !psiconv_compare_paragraph_layout(paragraph->base_paragraph,
                                              paragraph_type->paragraph)) {
          para_type = paragraph_type->nr;
          break;
        }
      }
      if (!para_type) {
        /* We need to add a new entry */
        para_type = new_type.nr = j+1;
        /* No need to copy them, we won't change them anyway */
        new_type.paragraph = paragraph->base_paragraph;
        new_type.character = para_charlayout;
        new_type.style = paragraph->base_style;
        paragraph_type = &new_type;
        if ((res = psiconv_list_add(paragraph_type_list,paragraph_type)))
          goto ERROR5;
        if ((res = psiconv_write_u32(buf_types,paragraph_type->nr))) 
          goto ERROR5;
        if (!(style = psiconv_get_style(styles,paragraph_type->style))) {
          psiconv_warn(0,psiconv_buffer_length(buf),"Unknown style");
          res = -PSICONV_E_GENERATE;
          goto ERROR5;
        }
        if ((res = psiconv_write_paragraph_layout_list(buf_types,
                                paragraph_type->paragraph,style->paragraph)))
          goto ERROR5;
        if (with_styles)
          if ((res = psiconv_write_u8(buf_types,paragraph_type->style)))
            goto ERROR5;
        if ((res = psiconv_write_character_layout_list(buf_types,
                                paragraph_type->character,style->character)))
          goto ERROR5;
      }
      if ((res = psiconv_write_u8(buf_elements,para_type)))
        goto ERROR5;
    }
  }

  /* HACK: special case: no paragraphs at all. We need to improvize. */
  if (!psiconv_list_length(value)) {
    if ((res = psiconv_write_u32(buf_types,1)))
      goto ERROR5;
    if ((res = psiconv_write_u32(buf_types,0)))
      goto ERROR5;
    if (with_styles)
      if ((res = psiconv_write_u8(buf_types,0)))
        goto ERROR5;
    if ((res = psiconv_write_u32(buf_types,0)))
      goto ERROR5;

    if ((res = psiconv_write_u32(buf_elements,1)))
      goto ERROR5;
    if ((res = psiconv_write_u8(buf_elements,1)))
      goto ERROR5;
    pel_length = 1;
    ptl_length = 1;
  } else  {
    pel_length = psiconv_list_length(value);
    ptl_length = psiconv_list_length(paragraph_type_list);
  }

  /* Now append everything */
  if ((res = psiconv_write_u16(buf,with_styles?0x0001:0x0000)))
    goto ERROR5;
  if ((res = psiconv_write_u8(buf, ptl_length)))
    goto ERROR5;
  if ((res = psiconv_buffer_concat(buf,buf_types)))
    goto ERROR5;
  if ((res = psiconv_write_u32(buf,pel_length)))
    goto ERROR5;
  if ((res = psiconv_buffer_concat(buf,buf_elements)))
    goto ERROR5;
  if ((res = psiconv_write_u32(buf,nr_of_inlines)))
     goto ERROR5;
  res = psiconv_buffer_concat(buf,buf_inlines);

ERROR5:
  psiconv_buffer_free(buf_inlines);
ERROR4:
  psiconv_buffer_free(buf_elements);
ERROR3:
  psiconv_buffer_free(buf_types);
ERROR2:
  psiconv_list_free(paragraph_type_list);
ERROR1:
  return res;
}

int psiconv_write_styled_layout_section(psiconv_buffer buf,
                                    psiconv_text_and_layout result,
                                    psiconv_word_styles_section styles)
{
  return psiconv_write_layout_section(buf,result,styles,1);
}

int psiconv_write_styleless_layout_section(psiconv_buffer buf,
                                       const psiconv_text_and_layout value,
                                       const psiconv_character_layout base_char,
                                       const psiconv_paragraph_layout base_para)
{
  int res = 0;
  psiconv_word_styles_section styles_section;

  if (!(styles_section = malloc(sizeof(*styles_section))))
    goto ERROR1;
  if (!(styles_section->normal = malloc(sizeof(*styles_section->normal))))
    goto ERROR2;
  if (!(styles_section->normal->character =
                            psiconv_clone_character_layout(base_char)))
    goto ERROR3;
  if (!(styles_section->normal->paragraph =
                            psiconv_clone_paragraph_layout(base_para)))
    goto ERROR4;
  styles_section->normal->hotkey = 0;
  if (!(styles_section->normal->name = strdup("")))
    goto ERROR5;
  if (!(styles_section->styles = psiconv_list_new(sizeof(
                                        struct psiconv_word_style_s))))
    goto ERROR6;

  res = psiconv_write_layout_section(buf,value,styles_section,0);
  psiconv_free_word_styles_section(styles_section);
  return res;

ERROR6:
  free(styles_section->normal->name);
ERROR5:
  psiconv_free_paragraph_layout(styles_section->normal->paragraph);
ERROR4:
  psiconv_free_character_layout(styles_section->normal->character);
ERROR3:
  free(styles_section->normal);
ERROR2:
  free(styles_section);
ERROR1:
  if (!res)
    return -PSICONV_E_NOMEM;
  else
    return res;
}
