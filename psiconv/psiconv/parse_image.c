/*
    parse_image.c - Part of psiconv, a PSION 5 file formats converter
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

#include "parse_routines.h"
#include "error.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


int psiconv_parse_jumptable_section(const psiconv_buffer buf,int lev,
                                    psiconv_u32 off, int *length,
                                    psiconv_jumptable_section *result)
{
  int res = 0;
  int len = 0;
  psiconv_u32 listlen,temp;
  int i;

  psiconv_progress(lev+1,off+len,"Going to read the jumptable section");
  if (!((*result) = psiconv_list_new(sizeof(psiconv_u32))))
    goto ERROR1;
  
  psiconv_progress(lev+2,off+len,"Going to read the list length");
  listlen = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"List length: %08x",listlen);
  len += 4;

  psiconv_progress(lev+2,off+len,"Going to read the list");
  for (i = 0; i < listlen; i++) {
    temp = psiconv_read_u32(buf,lev+2,off+len,&res);
    if (res)
      goto ERROR2;
    if ((res = psiconv_list_add(*result,&temp)))
      goto ERROR2;
    psiconv_debug(lev+3,off+len,"Offset: %08x",temp);
    len += 4;
  }

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of jumptable section "
                   "(total length: %08x)", len);

  return 0;

ERROR2:
  psiconv_list_free(*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Jumptable Section failed");
  if (length)
    *length = 0;
   if (!res)
     return -PSICONV_E_NOMEM;
   else
    return res;
}

static int decode_byte(int lev, psiconv_u32 off,
                       psiconv_paint_data_section data, psiconv_u32 *pixelnr,
                       psiconv_u8 byte, int bits_per_pixel, int linelen,
                       int *linepos,int picsize)
{
  int mask = (bits_per_pixel << 1) -1;
  int i;
  if (*linepos < (data->xsize + (8/bits_per_pixel) - 1) / (8/bits_per_pixel)) 
    for (i = 0; i < 8/bits_per_pixel; i ++) {
      if ((i != 0) && ((*pixelnr % (data->xsize)) == 0)) {
        psiconv_debug(lev+1,off,"Skipping padding: %02x",byte);
        i = 8;
      } else if (*pixelnr >= picsize) {
        psiconv_warn(lev+1,off,"Corrupted picture data!");
        psiconv_debug(lev+1,off,"Trying to write a pixel too far");
        return -1;
      } else {
        data->red[*pixelnr] = data->green[*pixelnr] = data->blue[*pixelnr] =
          ((float) (byte & mask)) / ((1 << bits_per_pixel) -1);
        psiconv_debug(lev+1,off,"Pixel %04x: (%04x,%04x) value %02x, color %f",
                      *pixelnr,*pixelnr % data->xsize,
                      *pixelnr / data->xsize, byte&mask, data->red[*pixelnr]);
        byte = byte >> bits_per_pixel;
        (*pixelnr) ++;
      }
    }
  else 
    psiconv_debug(lev+1,off,"Skipping padding byte");
  (*linepos) ++;
  if (*linepos == linelen)
    *linepos = 0;
  return 0;
}
                   

int psiconv_parse_paint_data_section(const psiconv_buffer buf,int lev,
                                     psiconv_u32 off, int *length,int isclipart,
                                     psiconv_paint_data_section *result)
{
  int res = 0;
  int len = 0;
  psiconv_u32 size,offset,picsize,temp,datasize,pixelnr,datanr,linelen;
  psiconv_u8 marker;
  int i,leng;
  psiconv_u32 bits_per_pixel,compression;
  int linepos = 0;

  psiconv_progress(lev+1,off,"Going to read a paint data section");
  if (!((*result) = malloc(sizeof(**result))))
    goto ERROR1;

  psiconv_progress(lev+2,off+len,"Going to read section size");
  size = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Section size: %08x",size);
  len += 4;

  psiconv_progress(lev+2,off+len,"Going to read pixel data offset");
  offset = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (offset != 0x28) {
    psiconv_warn(lev+2,off+len,
                 "Paint data section data offset has unexpected value");
    psiconv_debug(lev+2,off+len,
                  "Data offset: read %08x, expected %08x",offset,0x28);
    res = -1;
  }
  len += 4;

  psiconv_progress(lev+2,off+len,"Going to read picture X size");
  (*result)->xsize = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Picture X size: %08x:",(*result)->xsize);
  len += 4;

  psiconv_progress(lev+2,off+len,"Going to read picture Y size");
  (*result)->ysize = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Picture Y size: %08x:",(*result)->ysize);
  len += 4;

  picsize = (*result)->ysize * (*result)->xsize;

  psiconv_progress(lev+2,off+len,"Going to read the real picture x size");
  (*result)->pic_xsize = psiconv_read_length(buf,lev+2,off+len,&leng,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Picture x size: %f",(*result)->pic_xsize);
  len += leng;

  psiconv_progress(lev+2,off+len,"Going to read the real picture y size");
  (*result)->pic_ysize = psiconv_read_length(buf,lev+2,off+len,&leng,&res);
  if (res)
    goto ERROR2;
  psiconv_debug(lev+2,off+len,"Picture y size: %f",(*result)->pic_ysize);
  len += leng;

  psiconv_progress(lev+2,off+len,"Going to read the number of bits per pixel");
  bits_per_pixel=psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (bits_per_pixel > 8) {
    psiconv_warn(lev+2,off+len,"Picture has too many colors");
    psiconv_debug(lev+2,off+len,"Read %d colorbits",bits_per_pixel);
    res = -PSICONV_E_PARSE;
    goto ERROR2;
  }
  psiconv_debug(lev+2,off+len,"Bits per pixel: %d",bits_per_pixel);
  len += 4;

  for (i = 0 ; i < 2; i++) {
    temp = psiconv_read_u32(buf,lev+2,off+len,&res);
    if (res)
      goto ERROR2;
    if (temp != 00) {
      psiconv_warn(lev+2,off+len,
                   "Paint data section prologue has unknown values (ignored)");
      psiconv_debug(lev+2,off+len,
                    "offset %02x: read %08x, expected %08x",i,temp, 0x00);
    }
    len += 4;
  }
  
  psiconv_progress(lev+2,off+len,
                   "Going to read whether RLE compression is used");
  compression=psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (compression > 1) {
    psiconv_warn(lev+2,off+len,"Paint data section has unknown "
                               "compression type, assuming RLE");
    psiconv_debug(lev+2,off+len,"Read compression type %d",compression);
    compression = 1;
  }
  psiconv_debug(lev+2,off+len,"Compression: %s",compression?"RLE":"none");
  len += 4;

  if (isclipart) {
    psiconv_progress(lev+2,off+len,"Going to read an unknown long");
    temp = psiconv_read_u32(buf,lev+2,off+len,&res);
    if (res)
      goto ERROR2;
    if (temp != 0xffffffff) {
      psiconv_warn(lev+2,off+len,
                   "Paint data section prologue has unknown values (ignoring)");
      psiconv_debug(lev+2,off+len,
                    "offset %02x: read %08x, expected %08x",i,temp, 0xffffffff);
    }
    len += 4;
    psiconv_progress(lev+2,off+len,"Going to read a second unknown long");
    temp = psiconv_read_u32(buf,lev+2,off+len,&res);
    if (res)
      goto ERROR2;
    if (temp != 0x44) {
      psiconv_warn(lev+2,off+len,
                   "Paint data section prologue has unknown values (ignoring)");
      psiconv_debug(lev+2,off+len,
                    "offset %02x: read %08x, expected %08x",i,temp, 0x44);
    }
    len += 4;
  }

  if (!((*result)->red = malloc(sizeof(float) * picsize)))
    goto ERROR2;
  if (!((*result)->green = malloc(sizeof(float) * picsize)))
    goto ERROR3;
  if (!((*result)->blue = malloc(sizeof(float) * picsize)))
    goto ERROR4;
  len = offset;
  datasize = size - len;
  if (isclipart)
    len += 8;

  psiconv_progress(lev+2,off+len,"Going to read the pixel data");
  pixelnr = 0;
  datanr = 0;
  if (!compression) {
    linelen = datasize / (*result)->ysize;
    psiconv_debug(lev+3,off+len,"Line length: %04x bytes",linelen);
    while((datanr < datasize)) {
      temp = psiconv_read_u8(buf,lev+2,off+len+datanr,&res);
      if (res)
        goto ERROR5;
      if (decode_byte(lev+3,off+len+datanr,*result,&pixelnr,temp,bits_per_pixel,
                  linelen,&linepos,picsize)) {
        res = -PSICONV_E_PARSE;
        break;
      }
      datanr++;
    }
  } else {
    psiconv_progress(lev+2,off+len,"First pass: determining line length");
    datanr = 0;
    i = 0;
    while (datanr < datasize) {
      marker = psiconv_read_u8(buf,lev+3,off+len+datanr,&res);
      if (res) 
        goto ERROR5;
      if (marker >= 0x80) {
        datanr += 0x100 - marker + 1;
        i += 0x100 - marker;
      } else {
        datanr += 2;
        i += marker + 1;
      }
    }
    linelen = i / (*result)->ysize;
    datanr=0;
    psiconv_debug(lev+2,off+len,"Linelen: %04x bytes",linelen);
    while((datanr < datasize)) {
      marker = psiconv_read_u8(buf,lev+3,off+len+datanr,&res);
      if (res)
        goto ERROR5;
      psiconv_debug(lev+3,off+len+datanr,
                    "Pixelnr %08x, Datanr %08x: Read marker %02x",
                    pixelnr,datanr,marker);
      datanr ++;
      if (marker >= 0x80) {
        /* 0x100 - marker bytes of data follow */
        for (i = 0; i < 0x100-marker; i++,datanr++) {
          if (datanr >= datasize) {
            psiconv_warn(lev+3,off+len+datanr,"Corrupted picture data");
            psiconv_debug(lev+3,off+len+datanr,
                          "Picsize: %08x, Datasize: %08x, Pixelnr: %08x,"
                          "Datanr: %08x, marker: %02x",picsize,datasize,pixelnr,
                          datanr,marker);
            res = -PSICONV_E_PARSE;
            break;
          }  
          temp = psiconv_read_u8(buf,lev+2,off+len+datanr,&res);
          if (res)
            goto ERROR5;
          if (decode_byte(lev+2,off+len+datanr,*result,&pixelnr,temp,
                      bits_per_pixel,linelen,&linepos,picsize)) {
            res = -PSICONV_E_PARSE;
            break;
          }
        } 
      } else {
        if (datanr >= datasize) {
          psiconv_warn(lev+3,off+len+datanr,"Corrupted picture data");
          psiconv_debug(lev+3,off+len+datanr,
                        "Picsize: %08x, Datasize: %08x, Pixelnr: %08x,"
                        "Datanr: %08x, marker: %02x",picsize,datasize,pixelnr,
                        datanr,marker);
          res = -PSICONV_E_PARSE;
        } else {
          temp = psiconv_read_u8(buf,lev+3,off+len+datanr,&res);
          if (res)
            goto ERROR5;
          for (i = 0; i <= marker; i++) {
            if (decode_byte(lev+2,off+len+datanr,*result,&pixelnr,temp,
                        bits_per_pixel,linelen,&linepos,picsize)) {
              res = -PSICONV_E_PARSE;
              break;
            }
          }  
          datanr ++;
        }    
      }
    }
  }

  if (linepos >= ((*result)->xsize + (8/bits_per_pixel) - 1) / 
                 (8/bits_per_pixel)) 
    datanr += (linelen - linepos);
    
  if (res || (datanr != datasize) || (pixelnr != picsize)) {
    psiconv_warn(lev+2,off+len,"Corrupted picture data!");
    psiconv_debug(lev+3,off+len+datanr,
                  "Picsize: %08x, Datasize: %08x, Pixelnr: %08x,"
                  "Datanr: %08x",picsize,datasize,pixelnr,datanr);
    goto ERROR5;
  }

  len += datanr;

  if (length)
    *length = len;

  psiconv_progress(lev+1,off+len-1,"End of paint data section "
                   "(total length: %08x)", len);

  return res;

ERROR5:
  free((*result)->blue);
ERROR4:
  free((*result)->green);
ERROR3:
  free((*result)->red);
ERROR2:
  free (*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Paint Data Section failed");
  if (length)
    *length = 0;
   if (!res)
     return -PSICONV_E_NOMEM;
   else
    return res;
}

int psiconv_parse_sketch_section(const psiconv_buffer buf, int lev,
                                 psiconv_u32 off, int *length, int is_object,
                                 psiconv_sketch_section *result)
{
  int res=0;
  int len=0;
  psiconv_u32 temp;
  int leng;
  int i;

  psiconv_progress(lev+1,off,"Going to read the sketch section");
  if (!(*result = malloc(sizeof(**result))))
    goto ERROR1;

  if (!is_object) {
    psiconv_progress(lev+2,off+len,"Going to read the form hor. size");
    (*result)->form_xsize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Form hor. size: %04x",
                  (*result)->form_xsize);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the form ver. size");
    (*result)->form_ysize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Form ver. size: %04x",
                  (*result)->form_ysize);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the picture hor. offset");
    (*result)->picture_x_offset = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Picture hor. offset: %04x",
                  (*result)->picture_x_offset);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the picture ver. offset");
    (*result)->picture_y_offset = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Picture ver. offset: %04x",
                  (*result)->picture_y_offset);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to skip 5 words of zeros");
    for (i = 0; i < 5; i++) {
      temp = psiconv_read_u16(buf,lev+2,off+len,&res);
      if (res)
        goto ERROR2;
      if (temp != 0) {
        psiconv_warn(lev+2,off+len,
                     "Unexpected value in sketch section preamble (ignored)");
        psiconv_debug(lev+2,off+len,"Word %d: Read %04x, expected %04x",i,
                      temp,0);
      }
      off += 0x02;
    }
  } else {
    psiconv_progress(lev+2,off+len,"Going to read the displayed hor. size");
    (*result)->picture_xsize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Displayed hor. size: %04x",
                  (*result)->picture_xsize);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the displayed ver. size");
    (*result)->picture_ysize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Displayed ver. size: %04x",
                  (*result)->picture_ysize);
    len += 0x02;

    psiconv_progress(lev+2,off+len,"Going to skip 2 words of zeros");
    for (i = 0; i < 2; i++) {
      temp = psiconv_read_u16(buf,lev+2,off+len,&res);
     if (res)
        goto ERROR2;
      if (temp != 0) {
        psiconv_warn(lev+2,off+len,
                     "Unexpected value in sketch section preamble (ignored)");
        psiconv_debug(lev+2,off+len,"Word %d: Read %04x, expected %04x",i,
                      temp,0);
      }
      off += 0x02;
    }
    psiconv_progress(lev+2,off+len,"Going to read the picture hor. offset");
    (*result)->picture_x_offset = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Picture hor. offset: %04x",
                  (*result)->picture_x_offset);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the picture ver. offset");
    (*result)->picture_y_offset = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Picture ver. offset: %04x",
                  (*result)->picture_y_offset);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the form hor. size");
    (*result)->form_xsize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Form hor. size: %04x",
                  (*result)->form_xsize);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to read the form ver. size");
    (*result)->form_ysize = psiconv_read_u16(buf,lev+2,off + len,&res);
    if (res)
      goto ERROR2;
    psiconv_debug(lev+2,off+len,"Form ver. size: %04x",
                  (*result)->form_ysize);
    len += 0x02;
    psiconv_progress(lev+2,off+len,"Going to skip 1 zero word");
    temp = psiconv_read_u16(buf,lev+2,off+len,&res);
    if (res)
      goto ERROR2;
    if (temp != 0) {
      psiconv_warn(lev+2,off+len,
                   "Unexpected value in sketch section preamble (ignored)");
      psiconv_debug(lev+2,off+len,"Read %04x, expected %04x",i, temp,0);
    }
    off += 0x02;
  }

  psiconv_progress(lev+2,off+len,"Going to read the picture data");
  if ((res = psiconv_parse_paint_data_section(buf,lev+2,off+len,&leng,0,
                                          &((*result)->picture))))
    goto ERROR2;
  off += leng;
  if (!is_object) {
    (*result)->picture_xsize = (*result)->picture->xsize;
    (*result)->picture_ysize = (*result)->picture->ysize;
  }
  
  psiconv_progress(lev+2,off+len,"Going to read the hor. magnification");
  (*result)->magnification_x = psiconv_read_u16(buf,lev+2,off+len,&res)/1000.0;
  if (res)
    goto ERROR3;
  psiconv_debug(lev+2,off+len,"Form hor. magnification: %f",
                (*result)->magnification_x);
  len += 0x02;
  psiconv_progress(lev+2,off+len,"Going to read the ver. magnification");
  (*result)->magnification_y = psiconv_read_u16(buf,lev+2,off+len,&res)/1000.0;
  if (res)
    goto ERROR3;
  psiconv_debug(lev+2,off+len,"Form ver. magnification: %f",
                (*result)->magnification_y);
  len += 0x02;

  psiconv_progress(lev+2,off+len,"Going to read the left cut");
  temp = psiconv_read_u32(buf,lev+2,off + len,&res);
  if (res)
    goto ERROR3;
  (*result)->cut_left = (temp * 6.0) / (*result)->picture_xsize;
  psiconv_debug(lev+2,off+len,"Left cut: raw %08x, real: %f",
                temp,(*result)->cut_left);
  len += 0x04;
  psiconv_progress(lev+2,off+len,"Going to read the right cut");
  temp = psiconv_read_u32(buf,lev+2,off + len,&res);
  if (res)
    goto ERROR3;
  (*result)->cut_right = (temp * 6.0) / (*result)->picture_xsize;
  psiconv_debug(lev+2,off+len,"Right cut: raw %08x, real: %f",
                temp,(*result)->cut_right);
  len += 0x04;
  psiconv_progress(lev+2,off+len,"Going to read the top cut");
  temp = psiconv_read_u32(buf,lev+2,off + len,&res);
  if (res)
    goto ERROR3;
  (*result)->cut_top = (temp * 6.0) / (*result)->picture_ysize;
  psiconv_debug(lev+2,off+len,"Top cut: raw %08x, real: %f",
                temp,(*result)->cut_top);
  len += 0x04;
  psiconv_progress(lev+2,off+len,"Going to read the bottom cut");
  temp = psiconv_read_u32(buf,lev+2,off + len,&res);
  if (res)
    goto ERROR3;
  (*result)->cut_bottom = (temp * 6.0) / (*result)->picture_ysize;
  psiconv_debug(lev+2,off+len,"Bottom cut: raw %08x, real: %f",
                temp,(*result)->cut_bottom);
  len += 0x04;
  
  if (length)
    *length = len;

  psiconv_progress(lev,off+len-1,
                   "End of sketch section (total length: %08x)", len);

  return res;
ERROR3:
  psiconv_free_paint_data_section((*result)->picture);
ERROR2:
  free (*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Sketch Section failed");
  if (length)
    *length = 0;
   if (!res)
     return -PSICONV_E_NOMEM;
   else
    return res;
}


int psiconv_parse_clipart_section(const psiconv_buffer buf,int lev,
                                  psiconv_u32 off, int *length,
                                  psiconv_clipart_section *result)
{
  int res=0;
  int len=0;
  int leng;
  psiconv_u32 temp;

  psiconv_progress(lev+1,off+len,"Going to read the clipart section");
  if (!(*result = malloc(sizeof(**result))))
    goto ERROR1;

  psiconv_progress(lev+2,off+len,"Going to read the section ID");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (temp != PSICONV_ID_CLIPART_ITEM) {
    psiconv_warn(lev+2,off+len,
                 "Unexpected value in clipart section preamble (ignored)");
    psiconv_debug(lev+2,off+len,"Read %08x, expected %08x",temp,
                  PSICONV_ID_CLIPART_ITEM);
  } else 
    psiconv_debug(lev+2,off+len,"Clipart ID: %08x", temp);
  off += 4;
  
  psiconv_progress(lev+2,off+len,"Going to read an unknown long");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (temp != 0x02) {
    psiconv_warn(lev+2,off+len,
                 "Unexpected value in clipart section preamble (ignored)");
    psiconv_debug(lev+2,off+len,"Read %08x, expected %08x",temp,
                  0x02);
  } else 
    psiconv_debug(lev+2,off+len,"First unknown long: %08x", temp);
  off += 4;

  psiconv_progress(lev+2,off+len,"Going to read a second unknown long");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (temp != 0) {
    psiconv_warn(lev+2,off+len,
                 "Unexpected value in clipart section preamble (ignored)");
    psiconv_debug(lev+2,off+len,"Read %08x, expected %08x",temp, 0);
  } else 
    psiconv_debug(lev+2,off+len,"Second unknown long: %08x", temp);
  off += 4;

  psiconv_progress(lev+2,off+len,"Going to read a third unknown long");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if (temp != 0) {
    psiconv_warn(lev+2,off+len,
                 "Unexpected value in clipart section preamble (ignored)");
    psiconv_debug(lev+2,off+len,"Read %08x, expected %08x",temp, 0);
  } else 
    psiconv_debug(lev+2,off+len,"Third unknown long: %08x", temp);
  off += 4;

  psiconv_progress(lev+2,off+len,"Going to read a fourth unknown long");
  temp = psiconv_read_u32(buf,lev+2,off+len,&res);
  if (res)
    goto ERROR2;
  if ((temp != 0x0c) && (temp != 0x08)) {
    psiconv_warn(lev+2,off+len,
                 "Unexpected value in clipart section preamble (ignored)");
    psiconv_debug(lev+2,off+len,"Read %08x, expected %08x or %08x",temp,
                   0x0c, 0x08);
  } else 
    psiconv_debug(lev+2,off+len,"Fourth unknown long: %08x", temp);
  off += 4;

  psiconv_progress(lev+2,off+len,"Going to read the Paint Data Section");
  if ((res = psiconv_parse_paint_data_section(buf,lev+2,off+len,&leng,1,
                                          &((*result)->picture))))
    goto ERROR2;
  len += leng;

  if (length)
    *length = len;

  psiconv_progress(lev,off+len-1,
                   "End of clipart section (total length: %08x)", len);
  return 0;

ERROR2:
  free (*result);
ERROR1:
  psiconv_warn(lev+1,off,"Reading of Font failed");
  if (length)
    *length = 0;
   if (!res)
     return -PSICONV_E_NOMEM;
   else
    return res;
}
