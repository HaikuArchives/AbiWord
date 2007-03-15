/*
    generate_driver.c - Part of psiconv, a PSION 5 file formats converter
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

#include "error.h"
#include "generate_routines.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


int psiconv_write(psiconv_buffer *buf,const psiconv_file value)
{
  int res;

  if (!value) {
    psiconv_warn(0,0,"Can't parse to an empty buffer!");
    return -PSICONV_E_OTHER;
  }
  if (!(*buf = psiconv_buffer_new()))
    return -PSICONV_E_NOMEM;

  if (value->type == psiconv_word_file) {
    if ((res =psiconv_write_word_file(*buf,(psiconv_word_f) (value->file))))
      goto ERROR;
  } else if (value->type == psiconv_texted_file) {
    if ((res =psiconv_write_texted_file(*buf,
                                           (psiconv_texted_f) (value->file))))
      goto ERROR;
#if 0
  } else if (value->type == psiconv_sketch_file) {
    if ((res =psiconv_write_sketch_file(*buf,
                                           (psiconv_sketch_f) (value->file))))
      goto ERROR;
  } else if (value->type == psiconv_mbm_file) {
    if ((res =psiconv_write_mbm_file(*buf,
                                           (psiconv_mbm_f) (value->file))))
      goto ERROR;
  } else if (value->type == psiconv_clipart_file) {
    if ((res =psiconv_write_clipart_file(*buf,
                                           (psiconv_clipart_f) (value->file))))
      goto ERROR;
#endif
  } else {
    psiconv_warn(0,0,"Unknown or unsupported file type");
    res = -PSICONV_E_GENERATE;
    goto ERROR;
  }
  if ((res = psiconv_buffer_resolve(*buf)))
    goto ERROR;
  return -PSICONV_E_OK;

ERROR:
  psiconv_buffer_free(*buf);
  return res;
}
  
int psiconv_write_texted_file(psiconv_buffer buf,psiconv_texted_f value)
{
  psiconv_character_layout base_char;
  psiconv_paragraph_layout base_para;
  int res;
  psiconv_section_table_section section_table;
  psiconv_section_table_entry entry;
  psiconv_u32 section_table_id;
  psiconv_buffer buf_texted;

  if (!value) {
    psiconv_warn(0,0,"Null TextEd file");
    return -PSICONV_E_GENERATE;
  }

  if (!(section_table = psiconv_list_new(sizeof(*entry)))) {
    res = -PSICONV_E_NOMEM;
    goto ERROR1;
  }

  if (!(entry = malloc(sizeof(*entry)))) {
    res = -PSICONV_E_NOMEM;
    goto ERROR2;
  }
    
  if (!(base_char = psiconv_basic_character_layout())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR3;
  }
  if (!(base_para = psiconv_basic_paragraph_layout())) {
    res = -PSICONV_E_NOMEM;
    goto ERROR4;
  }

  if ((res = psiconv_write_header_section(buf,PSICONV_ID_PSION5,
                                          PSICONV_ID_DATA_FILE,
                                          PSICONV_ID_TEXTED)))
    goto ERROR5;

  section_table_id = psiconv_buffer_unique_id();
  if ((res = psiconv_write_offset(buf,section_table_id)))
    goto ERROR5;

  entry->id = PSICONV_ID_APPL_ID_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR5;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR5;
  if ((res=psiconv_write_application_id_section(buf,
                                           PSICONV_ID_TEXTED,"TextEd.app")))
    goto ERROR5;
  
  entry->id = PSICONV_ID_PAGE_LAYOUT_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR5;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR5;
  if ((res = psiconv_write_page_layout_section(buf,value->page_sec)))
    goto ERROR5;

  entry->id = PSICONV_ID_TEXTED;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR5;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR5;
  if ((res = psiconv_write_texted_section(buf,value->texted_sec,
                                           base_char,base_para,&buf_texted)))
    goto ERROR5;

  if ((res = psiconv_buffer_concat(buf,buf_texted)))
    goto ERROR6;


  if ((res = psiconv_buffer_add_target(buf,section_table_id)))
    goto ERROR6;

  res = psiconv_write_section_table_section(buf,section_table);
  
ERROR6:
  psiconv_buffer_free(buf_texted);
ERROR5:
  psiconv_free_paragraph_layout(base_para);
ERROR4:
  psiconv_free_character_layout(base_char);
ERROR3:
  free(entry);
ERROR2:
  psiconv_list_free(section_table);
ERROR1:
  return res;
}

int psiconv_write_word_file(psiconv_buffer buf,psiconv_word_f value)
{
  int res;
  psiconv_section_table_section section_table;
  psiconv_section_table_entry entry;
  psiconv_u32 section_table_id;

  if (!value) {
    psiconv_warn(0,0,"Null Word file");
    return -PSICONV_E_GENERATE;
  }

  if (!(section_table = psiconv_list_new(sizeof(*entry)))) {
    res = -PSICONV_E_NOMEM;
    goto ERROR1;
  }

  if (!(entry = malloc(sizeof(*entry)))) {
    res = -PSICONV_E_NOMEM;
    goto ERROR2;
  }
    
  if ((res = psiconv_write_header_section(buf,PSICONV_ID_PSION5,
                                          PSICONV_ID_DATA_FILE,
                                          PSICONV_ID_WORD)))
    goto ERROR3;

  section_table_id = psiconv_buffer_unique_id();
  if ((res = psiconv_write_offset(buf,section_table_id)))
    goto ERROR3;

  entry->id = PSICONV_ID_APPL_ID_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res=psiconv_write_application_id_section(buf,
                                           PSICONV_ID_WORD,"Word.app")))
    goto ERROR3;
  
  entry->id = PSICONV_ID_WORD_STATUS_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res = psiconv_write_word_status_section(buf,value->status_sec)))
    goto ERROR3;

  entry->id = PSICONV_ID_PAGE_LAYOUT_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res = psiconv_write_page_layout_section(buf,value->page_sec)))
    goto ERROR3;

  entry->id = PSICONV_ID_WORD_STYLES_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res = psiconv_write_word_styles_section(buf,value->styles_sec)))
    goto ERROR3;

  entry->id = PSICONV_ID_TEXT_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res = psiconv_write_text_section(buf,value->paragraphs)))
    goto ERROR3;

  entry->id = PSICONV_ID_LAYOUT_SECTION;
  entry->offset = psiconv_buffer_unique_id();
  if ((res = psiconv_list_add(section_table,entry)))
    goto ERROR3;
  if ((res = psiconv_buffer_add_target(buf,entry->offset)))
    goto ERROR3;
  if ((res = psiconv_write_styled_layout_section(buf,value->paragraphs,
                                                 value->styles_sec)))
    goto ERROR3;

  if ((res = psiconv_buffer_add_target(buf,section_table_id)))
    goto ERROR3;

  res = psiconv_write_section_table_section(buf,section_table);
  
ERROR3:
  free(entry);
ERROR2:
  psiconv_list_free(section_table);
ERROR1:
  return res;
}
