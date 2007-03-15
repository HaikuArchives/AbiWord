/*
    generate_routines.h - Part of psiconv, a PSION 5 file formats converter
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

#ifndef PSICONV_GENERATE_ROUTINES_H
#define PSICONV_GENERATE_ROUTINES_H

#include <psiconv/general.h>
#include <psiconv/data.h>
#include <psiconv/generate.h>
#include <psiconv/buffer.h>
#include <psiconv/common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/* *********************
   * generate_simple.c *
   ********************* */

extern int psiconv_write_u8(psiconv_buffer buf,const psiconv_u8 value);
extern int psiconv_write_u16(psiconv_buffer buf,const psiconv_u16 value);
extern int psiconv_write_u32(psiconv_buffer buf,const psiconv_u32 value);
extern int psiconv_write_S(psiconv_buffer buf, const psiconv_u32 value);
extern int psiconv_write_X(psiconv_buffer buf, const psiconv_u32 value);
extern int psiconv_write_length(psiconv_buffer buf, 
                                const psiconv_length_t value);
extern int psiconv_write_size(psiconv_buffer buf, const psiconv_size_t value);
extern int psiconv_write_bool(psiconv_buffer buf, const psiconv_bool_t value);
extern int psiconv_write_string(psiconv_buffer buf, 
                                const psiconv_string_t value);
extern int psiconv_write_offset(psiconv_buffer buf, psiconv_u32 id);

/* *********************
   * generate_layout.c *
   ********************* */

extern int psiconv_write_color(psiconv_buffer buf, const psiconv_color value);
extern int psiconv_write_font(psiconv_buffer buf, const psiconv_font value);
extern int psiconv_write_border(psiconv_buffer buf, const psiconv_border value);
extern int psiconv_write_bullet(psiconv_buffer buf, const psiconv_bullet value);
extern int psiconv_write_tab(psiconv_buffer buf,psiconv_tab value);
extern int psiconv_write_paragraph_layout_list(psiconv_buffer buf,
                                        psiconv_paragraph_layout value,
                                        psiconv_paragraph_layout base);
extern int psiconv_write_character_layout_list(psiconv_buffer buf,
                                        psiconv_character_layout value,
                                        psiconv_character_layout base);

/* *******************
   * generate_page.c *
   ******************* */

extern int psiconv_write_page_header(psiconv_buffer buf,
                                     const psiconv_page_header value,
                                     psiconv_buffer *extra_buf);
extern int psiconv_write_page_layout_section(psiconv_buffer buf,
                                     const psiconv_page_layout_section value);

/* *********************
   * generate_common.c *
   ********************* */

extern int psiconv_write_header_section(psiconv_buffer buf,psiconv_u32 uid1,
                                        psiconv_u32 uid2, psiconv_u32 uid3);
extern int psiconv_write_section_table_section(psiconv_buffer buf,
                                    const psiconv_section_table_section value);
extern int psiconv_write_application_id_section(psiconv_buffer buf,
                                  psiconv_u32 id, const psiconv_string_t text);
extern int psiconv_write_text_section(psiconv_buffer buf,
                                      const psiconv_text_and_layout value);
extern int psiconv_write_styled_layout_section(psiconv_buffer buf,
                                    psiconv_text_and_layout result,
                                    psiconv_word_styles_section styles);
extern int psiconv_write_styleless_layout_section(psiconv_buffer buf,
                                     const psiconv_text_and_layout value,
                                     const psiconv_character_layout base_char,
                                     const psiconv_paragraph_layout base_para);



/* *********************
   * generate_texted.c *
   ********************* */

extern int psiconv_write_texted_section(psiconv_buffer buf,
                                    const psiconv_texted_section value,
                                    const psiconv_character_layout base_char,
                                    const psiconv_paragraph_layout base_para,
                                    psiconv_buffer *extra_buf);

/* *******************
   * generate_word.c *
   ******************* */

extern int psiconv_write_word_status_section(psiconv_buffer buf,
                                         psiconv_word_status_section value);
extern int psiconv_write_word_styles_section(psiconv_buffer buf,
                                      psiconv_word_styles_section value);


/* *********************
   * generate_driver.c *
   ********************* */

extern int psiconv_write_word_file(psiconv_buffer buf,psiconv_word_f value);
extern int psiconv_write_texted_file(psiconv_buffer buf,
                                     psiconv_texted_f value);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PSICONV_GENERATE_ROUTINES_H */
