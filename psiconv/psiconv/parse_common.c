/*
    parse_common.c - Part of psiconv, a PSION 5 file formats converter
    Copyright (c) 1999, 2000  Frodo Looijaard <frodol@dds.nl>

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

#include "parse_routines.h"
#include "error.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


static int psiconv_parse_layout_section(const psiconv_buffer buf,
                                 int lev,psiconv_u32 off,
                                 int *length,
                                 psiconv_text_and_layout result,
                                 psiconv_word_styles_section styles,
                                 int with_styles);

int psiconv_parse_header_section(const psiconv_buffer buf,int lev,
                                 psiconv_u32 off, int *length, 
                                 psiconv_header_section *result)
{
  int res=0;
  int len=0;
  psiconv_u32 temp;

  psiconv_progress(lev+1,off+len,"Going to read the header section");
  if (!((*result) = malloc(sizeof(**result))))
    goto ERROR1;
  
  psiconv_progress(lev+2,off+len,"Going to read UID1 to UID3");
  (*result)->uid1 = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"UID1: %08x",(*result)->uid1);
  if ((*result)->uid1 == PSICONV_ID_CLIPART) {
    /* That's all folks... */
    (*result)->file = psiconv_clipart_file;
    (*result)->uid2 = 0;
    (*result)->uid3 = 0;
    (*result)->checksum = 0;
    len += 4;
    psiconv_debug(lev+2,off+len,"File is a Clipart file");
    goto DONE;
  }
  if ((*result)->uid1 != PSICONV_ID_PSION5) {
    psiconv_warn(lev+2,off+len,"UID1 has unknown value. This is probably "
                               "not a (parsable) Psion 5 file");
    res = -PSICONV_E_PARSE;
    goto ERROR2;
  }
  len += 4;
  (*result)->uid2 = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"UID2: %08x",(*result)->uid2);
  len += 4;
  (*result)->uid3 = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"UID3: %08x",(*result)->uid3);
  len += 4;

  (*result)->file = psiconv_unknown_file;
  if ((*result)->uid1 == PSICONV_ID_PSION5) {
    if ((*result)->uid2 == PSICONV_ID_DATA_FILE) {
      if ((*result)->uid3 == PSICONV_ID_WORD) {
        (*result)->file = psiconv_word_file;
        psiconv_debug(lev+2,off+len,"File is a Word file");
      } else if ((*result)->uid3 == PSICONV_ID_TEXTED) {
        (*result)->file = psiconv_texted_file;
        psiconv_debug(lev+2,off+len,"File is a TextEd file");
      } else if ((*result)->uid3 == PSICONV_ID_SKETCH) {
        (*result)->file = psiconv_sketch_file;
        psiconv_debug(lev+2,off+len,"File is a Sketch file");
      } else if ((*result)->uid3 == PSICONV_ID_SHEET) {
        (*result)->file = psiconv_sheet_file;
        psiconv_debug(lev+2,off+len,"File is a Sheet file");
      }
    } else if ((*result)->uid2 == PSICONV_ID_MBM_FILE) {
      (*result)->file = psiconv_mbm_file;
      if ((*result)->uid3 != 0x00)
        psiconv_warn(lev+2,off+len,"UID3 set in MBM file?!?");
      psiconv_debug(lev+2,off+len,"File is a MBM file");
    }  
  }
  if ((*result)->file == psiconv_unknown_file) {
    psiconv_warn(lev+2,off+len,"Unknown file type");
    (*result)->file = psiconv_unknown_file;
  }

  psiconv_progress(lev+2,off+len,"Checking UID4");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (temp == psiconv_checkuid((*result)->uid1,(*result)->uid2,
                               (*result)->uid3)) 
    psiconv_debug(lev+2,off+len,"Checksum %08x is correct",temp);
  else {
    psiconv_warn(lev+2,off+len,"Checksum failed, file corrupted!");
    psiconv_debug(lev+2,off+len,"Expected checksum %08x, found %08x",
                  psiconv_checkuid((*result)->uid1,(*result)->uid2,
                                   (*result)->uid3),temp);
    res = -PSICONV_E_PARSE;
    goto ERROR2;
  }
  len += 4;

DONE:
  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,
                   "End of Header Section (total length: %08x)",len);
 
  return res;

ERROR2:
  free(*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Header Section failed");
  if (length)
    *length = 0;
  if (res == 0)
    return -PSICONV_E_NOMEM;
  else
    return res;
}

int psiconv_parse_section_table_section(const psiconv_buffer buf, int lev,
                                        psiconv_u32 off, int *length,
                                        psiconv_section_table_section *result)
{
  int res=0;
  int len=0;
  psiconv_section_table_entry entry;

  int i;
  psiconv_u8 nr;

  psiconv_progress(lev+1,off+len,"Going to read the section table section");
  if (!(*result = psiconv_list_new(sizeof(*entry))))
    goto ERROR1;

  psiconv_progress(lev+2,off+len,"Going to read the section table length");
  nr = psiconv_read_u8(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Length: %08x",nr);
  if (nr & 0x01) {
    psiconv_warn(lev+2,off+len,
                 "Section table length odd - ignoring last entry");
  }
  len ++;

  psiconv_progress(lev+2,off+len,"Going to read the section table entries");
  entry = malloc(sizeof(*entry));
  for (i = 0; i < nr / 2; i++) {
    entry->id = psiconv_read_u32(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR3;
    psiconv_debug(lev+2,off + len,"Entry %d: ID = %08x",i,entry->id);
    len += 0x04;
    entry->offset = psiconv_read_u32(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR3;
    psiconv_debug(lev+2,off +len,"Entry %d: Offset = %08x",i,entry->offset);
    len += 0x04;
    if ((res=psiconv_list_add(*result,entry)))
      goto ERROR3;
  }

  free(entry);

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of section table section "
                   "(total length: %08x)", len);

  return 0;
ERROR3:
  free(entry);
ERROR2:
  psiconv_list_free(*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Section Table Section failed");
  if (length)
    *length = 0;
  if (res == 0)
    return -PSICONV_E_NOMEM;
  else
    return res;
}

int psiconv_parse_application_id_section(const psiconv_buffer buf, int lev, 
                                         psiconv_u32 off, int *length,
                                         psiconv_application_id_section *result)
{
  int res=0;
  int len=0;
  int leng;

  psiconv_progress(lev+1,off,"Going to read the application id section");
  if (!(*result = malloc(sizeof(**result))))
    goto ERROR1;

  psiconv_progress(lev+2,off+len,"Going to read the type identifier");
  (*result)->id = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Identifier: %08x",(*result)->id);
  len += 4;

  psiconv_progress(lev+2,off+len,"Going to read the application id string");
  (*result)->name = psiconv_read_string(buf,lev+2,off+len,&leng,&res);
  if (res)
    goto ERROR2;
  len += leng;

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of application id section "
                   "(total length: %08x", len);

  return res;
ERROR2:
  free(*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Application ID Section failed");
  if (length)
    *length = 0;
  if (res == 0)
    return -PSICONV_E_NOMEM;
  else
    return res;
}

int psiconv_parse_text_section(const psiconv_buffer buf,int lev,psiconv_u32 off,
                               int *length,psiconv_text_and_layout *result)
{

  int res = 0;
  int len=0;

  psiconv_u32 text_len;
  psiconv_paragraph para;

  int nr;
  int i,j,start,leng,temp;
  char *str_copy;
 
  psiconv_progress(lev+1,off,"Going to parse the text section");
  psiconv_progress(lev+2,off,"Reading the text length");

  if(!(*result = psiconv_list_new(sizeof(*para))))
    goto ERROR1;
  if (!(para = malloc(sizeof(*para))))
    goto ERROR2;

  text_len = psiconv_read_X(buf,lev+2,off,&leng,&res);
  if (res)
    goto ERROR3;
  psiconv_debug(lev+2,off,"Length: %08x",text_len);
  len += leng;

  psiconv_progress(lev+2,off+len,"Going to read all paragraph text");
  nr = 0;
  start = 0;
  for (i = 0; i < text_len; i++) {
    temp = psiconv_read_u8(buf,lev+2,off+len+i,&res);
    if (res)
      goto ERROR3;
    if (temp == 0x06) {
      if (!(para->text = malloc(i - start + 1)))
        goto ERROR3;
      for (j = 0; j < i - start; j++) {
        temp = psiconv_read_u8(buf,lev+1,off + len + start + j,&res);
        if (res) 
          goto ERROR4;
        para->text[j] = temp;
      }
      para->text[j] = 0;
      
      if ((res = psiconv_list_add(*result,para)))
        goto ERROR4;

      if (!(str_copy = psiconv_make_printable(para->text)))
        goto ERROR3;
      psiconv_debug(lev+2,off+i+len,"Line %d: %d characters",nr,
                    strlen(str_copy) +1);
      psiconv_debug(lev+2,off+i+len,"Line %d: `%s'",nr,str_copy);
      free(str_copy);

      start = i + 1;
      nr ++;
    }
  }
  
  if (start != text_len) {
    psiconv_warn(lev+2,off+start+len,
         "Last line does not end on EOL (%d characters left)", len - start);
    if (!(para->text = malloc(text_len - start + 1)))
      goto ERROR3;
    for (j = 0; j < text_len - start; j++) {
      temp = psiconv_read_u8(buf,lev+2,off + start + j + len, &res);
      if (res)
        goto ERROR4;
      para->text[j] = temp;
    }
    para->text[text_len - start] = 0;
    if ((res = psiconv_list_add(*result,para)))
      goto ERROR4;
    if (!(str_copy = psiconv_make_printable(para->text)))
      goto ERROR3;
    psiconv_debug(lev+2,off+start+len,"Last line: %d characters",nr,
                   strlen(str_copy)+1);
    psiconv_debug(lev+2,off+start+len,"Last line: `%s'",str_copy);
    free(str_copy);
  }
 
  free(para);

  /* Initialize the remaining parts of each paragraph */
  for (i = 0; i < psiconv_list_length(*result); i ++) {
    if (!(para = psiconv_list_get(*result,i))) {
      psiconv_warn(lev+2,off+len,"Massive memory corruption");
      goto ERROR2_0;
    }
    if (!(para->in_lines = psiconv_list_new(sizeof(
                              struct psiconv_in_line_layout_s))))
      goto ERROR2_0;
    if (!(para->replacements = psiconv_list_new(sizeof(
                              struct psiconv_replacement_s)))) 
      goto ERROR2_1;
    if (!(para->base_character = psiconv_basic_character_layout()))
       goto ERROR2_2;
    if (!(para->base_paragraph = psiconv_basic_paragraph_layout()))
       goto ERROR2_3;
    para->base_style = 0;
  }
 

  len += text_len;

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of text section (total length: %08x",
                   len);

  return res;

ERROR2_3:
  psiconv_free_character_layout(para->base_character);
ERROR2_2:
  psiconv_list_free(para->replacements);
ERROR2_1:
  psiconv_list_free(para->in_lines);
ERROR2_0:
  for (j = 0; j < i; j++) {
    if (!(para = psiconv_list_get(*result,j))) {
      psiconv_warn(lev+1,off,"Massive memory corruption...");
      break;
    }
    psiconv_list_free(para->in_lines);
    psiconv_list_free(para->replacements);
    psiconv_free_character_layout(para->base_character);
    psiconv_free_paragraph_layout(para->base_paragraph);
  }
  goto ERROR2;

ERROR4:
  free(para->text);
ERROR3:
  free(para);
ERROR2:
  for (i = 0; i < psiconv_list_length(*result);i++) {
    if (!(para = psiconv_list_get(*result,i))) {
      psiconv_warn(lev+1,off,"Massive memory corruption...");
      break;
    }
    free(para->text);
  }
  psiconv_list_free(*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Text Section failed");
  if (length)
    *length = 0;
  if (!res)
    return -PSICONV_E_NOMEM;
  else
    return res;
}

/* First do a parse_text_section, or you will get into trouble here */
int psiconv_parse_layout_section(const psiconv_buffer buf,
                                 int lev,psiconv_u32 off,
                                 int *length,
                                 psiconv_text_and_layout result,
                                 psiconv_word_styles_section styles,
                                 int with_styles)
{
  int res = 0;
  int len = 0;
  psiconv_u32 temp;
  int parse_styles,nr,i,j,total,leng,line_length;

  typedef struct anon_style_s
  {
    int nr;
    psiconv_s16 base_style;
    psiconv_character_layout character;
    psiconv_paragraph_layout paragraph;
  } *anon_style;

  typedef psiconv_list anon_style_list; /* of struct anon_style */

  anon_style_list anon_styles;
  struct anon_style_s anon;
  anon_style anon_ptr=NULL;

  psiconv_character_layout temp_char;
  psiconv_paragraph_layout temp_para;
  psiconv_word_style temp_style;
  psiconv_paragraph para;
  struct psiconv_in_line_layout_s in_line;

  int *inline_count;


  psiconv_progress(lev+1,off,"Going to read the layout section");

  psiconv_progress(lev+2,off,"Going to read the section type");
  temp = psiconv_read_u16(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR1;
  psiconv_debug(lev+2,off+len,"Type: %02x",temp);
  parse_styles = with_styles;
  if ((temp == 0x0001) && !with_styles) {
    psiconv_warn(lev+2,off+len,"Styleless layout section expected, "
                 "but styled section found!");
    parse_styles = 1;
  } else if ((temp == 0x0000) && (with_styles)) {
    psiconv_warn(lev+2,off+len,"Styled layout section expected, "
                 "but styleless section found!");
    parse_styles = 0;
  } else if ((temp != 0x0000) && (temp != 0x0001)) {
    psiconv_warn(lev+2,off+len,
                 "Layout section type indicator has unknown value!");
  }
  len += 0x02;

  psiconv_progress(lev+2,off+len,"Going to read paragraph type list");
  if (!(anon_styles = psiconv_list_new(sizeof(anon))))
    goto ERROR1;
  psiconv_progress(lev+3,off+len,"Going to read paragraph type list length");
  nr = psiconv_read_u8(buf,lev+3,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+3,off+len,"Length: %02x",nr);
  len ++;

  psiconv_progress(lev+3,off+len,
                   "Going to read the paragraph type list elements");
  for (i = 0; i < nr; i ++) {
    psiconv_progress(lev+3,off+len,"Element %d",i);
    anon.nr = psiconv_read_u32(buf,lev+4,off+len,&res);
    if (res) 
      goto ERROR3;
    psiconv_debug(lev+4,off+len,"Number: %08x",anon.nr);
    len += 0x04;
  
    psiconv_progress(lev+4,off,"Going to determine the base style");
    if (parse_styles) {
      temp = psiconv_read_u32(buf,lev+4, off+len,&res);
      if (res)
        goto ERROR3;
      anon.base_style = psiconv_read_u8(buf,lev+3, off+len+4+temp,&res);
      if (res)
        goto ERROR3;
      psiconv_debug(lev+4,off+len+temp,
                    "Style indicator: %02x",anon.base_style);
    } else
      anon.base_style = 0;
    if (!(temp_style = psiconv_get_style(styles,anon.base_style))) {
      psiconv_warn(lev+4,off,"Unknown Style referenced");
      if (!(temp_style = psiconv_get_style(styles,anon.base_style))) {
        psiconv_warn(lev+4,off,"Base style unknown");
        goto ERROR3;
      }
    }
    if (!(anon.paragraph = psiconv_clone_paragraph_layout
                                              (temp_style->paragraph)))
      goto ERROR3;
    if (!(anon.character = psiconv_clone_character_layout
                                              (temp_style->character)))
      goto ERROR3_1;

    psiconv_progress(lev+4,off+len,"Going to read the paragraph layout");
    if ((res = psiconv_parse_paragraph_layout_list(buf,lev+4,off+len,&leng,
                                               anon.paragraph)))
       goto ERROR3_2;
    len += leng;
    if (parse_styles)
      len ++;

    psiconv_progress(lev+4,off+len,"Going to read the character layout");
    if ((res = psiconv_parse_character_layout_list(buf,lev+4,off+len,&leng,
                                               anon.character)))
      goto ERROR3_2;
    len += leng;
    if ((res = psiconv_list_add(anon_styles,&anon)))
      goto ERROR3_2;
  }

  psiconv_progress(lev+2,off+len,"Going to parse the paragraph element list");
  psiconv_progress(lev+3,off+len,"Going to read the number of paragraphs");
  nr = psiconv_read_u32(buf,lev+3,off+len,&res);
  if (res)
    goto ERROR3;
  if (nr != psiconv_list_length(result)) {
    psiconv_warn(lev+3,off+len,
         "Number of text paragraphs and paragraph elements does not match");
    psiconv_debug(lev+3,off+len,
          "%d text paragraphs, %d paragraph elements",
          psiconv_list_length(result),nr);
  }
  psiconv_debug(lev+3,off+len,"Number of paragraphs: %d",nr);
  len += 4;
  if (!(inline_count = malloc(nr * sizeof(*inline_count))))
    goto ERROR3;

  psiconv_progress(lev+3,off+len,"Going to read the paragraph elements");
  for (i = 0; i < nr; i ++) {
    psiconv_progress(lev+3,off+len,"Element %d",i);
    if (i >= psiconv_list_length(result)) {
      psiconv_debug(lev+4,off+len,"Going to allocate a new element");
      if (!(para = malloc(sizeof(*para))))
        goto ERROR4;
      if (!(para->in_lines = psiconv_list_new(sizeof(
                              struct psiconv_in_line_layout_s))))
        goto ERROR4_1;
      para->base_style = 0;
      if (!(para->base_character = psiconv_basic_character_layout()))
        goto ERROR4_2;
      if (!(para->base_paragraph = psiconv_basic_paragraph_layout()))
        goto ERROR4_3;
      if ((res = psiconv_list_add(result,para)))
        goto ERROR4_4;
      free(para);
    }
    if (!(para = psiconv_list_get(result,i)))
      goto ERROR4;

    psiconv_progress(lev+4,off+len,"Going to read the paragraph length");
    temp = psiconv_read_u32(buf,lev+4,off+len,&res);
    if (res)
      goto ERROR4;
    if (temp != strlen(para->text)+1) {
      psiconv_warn(lev+4,off+len,
                   "Disagreement of the length of paragraph in layout section");
      psiconv_debug(lev+4,off+len,
                    "Paragraph length: layout section says %d, counted %d",
                    temp,strlen(para->text)+1);
    } else
      psiconv_debug(lev+4,off+len,"Paragraph length: %d",temp);
    len += 4;

    psiconv_progress(lev+4,off+len,"Going to read the paragraph type");
    temp = psiconv_read_u8(buf,lev+4,off+len,&res);
    if (res)
       goto ERROR4;
    if (temp != 0x00) {
      psiconv_debug(lev+4,off+len,"Type: %02x",temp);
      for (j = 0; j < psiconv_list_length(anon_styles); j++) {
        if (!(anon_ptr = psiconv_list_get(anon_styles,j))) {
          psiconv_warn(lev+4,off+len,"Massive memory curruption");
          goto ERROR4;
        }
        if (temp == anon_ptr->nr)
          break;
      }
      if (j == psiconv_list_length(anon_styles)) {
        psiconv_warn(lev+4,off+len,"Layout section paragraph type unknown");
        psiconv_debug(lev+4,off+len,"Unknown type - using base styles instead");
        para->base_style = 0;
        if (!(temp_style = psiconv_get_style(styles,0))) {
          psiconv_warn(lev+4,off,"Base style unknown");
          goto ERROR4;
        }
        if (!(temp_para = psiconv_clone_paragraph_layout
                                               (temp_style->paragraph)))
          goto ERROR4;
        psiconv_free_paragraph_layout(para->base_paragraph);
        para->base_paragraph = temp_para;

        if (!(temp_char = psiconv_clone_character_layout
                                               (temp_style->character)))
          goto ERROR4;
        psiconv_free_character_layout(para->base_character);
        para->base_character = temp_char;
      } else {
        para->base_style = anon_ptr->base_style;
        if (!(temp_para = psiconv_clone_paragraph_layout (anon_ptr->paragraph)))
          goto ERROR4;
        psiconv_free_paragraph_layout(para->base_paragraph);
        para->base_paragraph = temp_para;

        if (!(temp_char = psiconv_clone_character_layout (anon_ptr->character)))
          goto ERROR4;
        psiconv_free_character_layout(para->base_character);
        para->base_character = temp_char;
      }
      inline_count[i] = 0;
      len += 0x01;
    } else {
      psiconv_debug(lev+4,off+len,"Type: %02x (not based on a paragraph type)"
                    ,temp);
      len += 0x01;
      if (parse_styles) {
        temp = psiconv_read_u32(buf,lev+4,off+len,&res);
        if (res)
          goto ERROR4;
        psiconv_progress(lev+4,off+len+temp+4,
                         "Going to read the paragraph element base style");
        temp = psiconv_read_u8(buf,lev+4, off+len+temp+4,&res);
        if (res)
          goto ERROR4;
        psiconv_debug(lev+4,off+len+temp+4, "Style: %02x",temp);
      } else
        temp = 0x00;

      if (!(temp_style = psiconv_get_style (styles,temp))) {
        psiconv_warn(lev+4,off,"Unknown Style referenced");
        if (!(temp_style = psiconv_get_style(styles,0))) {
          psiconv_warn(lev+4,off,"Base style unknown");
          goto ERROR4;
        }
      }

      if (!(temp_para = psiconv_clone_paragraph_layout(temp_style->paragraph)))
        goto ERROR4;
      psiconv_free_paragraph_layout(para->base_paragraph);
      para->base_paragraph = temp_para;

      if (!(temp_char = psiconv_clone_character_layout(temp_style->character)))
        goto ERROR4;
      psiconv_free_character_layout(para->base_character);
      para->base_character = temp_char;

      para->base_style = temp;
      psiconv_progress(lev+4,off+len,"Going to read paragraph layout");
      if ((res = psiconv_parse_paragraph_layout_list(buf,lev+4,off+len,&leng,
                                                para->base_paragraph)))
        goto ERROR4;
      len += leng;
      if (parse_styles)
        len += 1;
      psiconv_progress(lev+4,off+len,"Going to read number of in-line "
                       "layout elements");
      inline_count[i] = psiconv_read_u32(buf,lev+4,off+len,&res);
      if (res)
        goto ERROR4;
      psiconv_debug(lev+4,off+len,"Nr: %08x",inline_count[i]);
      len += 4;
    }
  }
        
  psiconv_progress(lev+2,off+len,"Going to read the text layout inline list");

  psiconv_progress(lev+3,off+len,"Going to read the number of elements");
  nr = psiconv_read_u32(buf,lev+3,off+len,&res);
  if (res)
    goto ERROR4;
  psiconv_debug(lev+3,off+len,"Elements: %08x",nr);
  len += 0x04;

  psiconv_progress(lev+3,off+len,
                   "Going to read the text layout inline elements");
  total = 0;
  for (i = 0; i < psiconv_list_length(result); i++) {
    if (!(para = psiconv_list_get(result,i))) {
      psiconv_warn(lev+3,off+len,"Massive memory corruption");
      goto ERROR4;
    }
    line_length = -1;
    for (j = 0; j < inline_count[i]; j++) {
      psiconv_progress(lev+3,off+len,"Element %d: Paragraph %d, element %d",
                        total,i,j);
      if (total >= nr) {
        psiconv_warn(lev+3,off+len,
                     "Layout section inlines: not enough element");
        psiconv_debug(lev+3,off+len,"Can't read element!");
      } else {
        total ++;
        if (!(in_line.layout = psiconv_clone_character_layout
                 (para->base_character)))
          goto ERROR4;
        psiconv_progress(lev+4,off+len,"Going to read the element type");
        temp = psiconv_read_u8(buf,lev+4,len+off,&res);
        if (res)
          goto ERROR4;
        len += 1;
        psiconv_debug(lev+4,off+len,"Type: %02x",temp);
        psiconv_progress(lev+4,off+len,
                      "Going to read the number of characters it applies to");
        in_line.length = psiconv_read_u32(buf,lev+4,len+off,&res);
        if (res)
          goto ERROR4;
        psiconv_debug(lev+4,off+len,"Length: %02x",in_line.length);
        len += 4;
        psiconv_progress(lev+4,off+len,"Going to read the character layout");
        if ((res = psiconv_parse_character_layout_list(buf,lev+4,off+len,&leng,
                                                   in_line.layout)))
          goto ERROR4;
        len += leng;

        if (temp == 0x01) {
          psiconv_debug(lev+4,off+len,"Skipping object data");
          len += 0x10;
        } else if (temp != 0x00) {
          psiconv_warn(lev+4,off+len,"Layout section unknown inline type");
        }
        if (line_length + in_line.length > strlen(para->text)) {
          psiconv_warn(lev+4,off+len,
                       "Layout section inlines: line length mismatch");
          res = -1;
          in_line.length = strlen(para->text) - line_length;
        }
        line_length += in_line.length;
        if ((res = psiconv_list_add(para->in_lines,&in_line)))
          goto ERROR4;
      }
    }
  }    
  
  if (total != nr) {
    psiconv_warn(lev+4,off+len,
                 "Layout section too many inlines, skipping remaining");
  }

  free(inline_count);

  for (i = 0 ; i < psiconv_list_length(anon_styles); i ++) {
    if (!(anon_ptr = psiconv_list_get(anon_styles,i))) {
      psiconv_warn(lev+4,off+len,"Massive memory corruption");
      goto ERROR2;
    }
    psiconv_free_character_layout(anon_ptr->character);
    psiconv_free_paragraph_layout(anon_ptr->paragraph);
  }
  psiconv_list_free(anon_styles);

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of layout section (total length: %08x)",
                   len);

  return 0;

ERROR4_4:
  psiconv_free_paragraph_layout(para->base_paragraph);
ERROR4_3:
  psiconv_free_character_layout(para->base_character);
ERROR4_2:
  psiconv_list_free(para->in_lines);
ERROR4_1:
  free(para);
  goto ERROR4;

ERROR3_2:
  psiconv_free_character_layout(anon.character);
ERROR3_1:
  psiconv_free_paragraph_layout(anon.paragraph);
  goto ERROR3;

ERROR4:
  free(inline_count);
ERROR3:
  for (i = 0; i < psiconv_list_length(anon_styles); i++) {
    if (!(anon_ptr = psiconv_list_get(anon_styles,i))) {
      psiconv_warn(lev+1,off,"Massive memory corruption");
      break;
    }
    psiconv_free_paragraph_layout(anon_ptr->paragraph);
    psiconv_free_character_layout(anon_ptr->character);
  }

ERROR2:
  psiconv_list_free(anon_styles);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Layout Section failed");
  if (length)
    *length = 0;
  if (!res)
    return -PSICONV_E_NOMEM;
  else
    return res;
}
      
int psiconv_parse_styled_layout_section(const psiconv_buffer buf,
                                        int lev,psiconv_u32 off,
                                        int *length,
                                        psiconv_text_and_layout result,
                                        psiconv_word_styles_section styles)
{
  return psiconv_parse_layout_section(buf,lev,off,length,result,styles,1);
}

int psiconv_parse_styleless_layout_section(const psiconv_buffer buf,
                                     int lev,psiconv_u32 off,
                                     int *length,
                                     psiconv_text_and_layout result,
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
  
  res = psiconv_parse_layout_section(buf,lev,off,length,result,
                                     styles_section,0);

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
  psiconv_warn(lev+1,off,"Reading of Styleless Layout Section failed");
  if (length)
    *length = 0;
  if (!res)
    return -PSICONV_E_NOMEM;
  else
    return res;
}

