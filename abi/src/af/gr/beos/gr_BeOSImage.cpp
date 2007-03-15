/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <png.h>

#include "gr_BeOSImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>
#include <DataIO.h>

GR_BeOSImage::GR_BeOSImage(const char* szName)
{
	m_image = NULL;
	
	if (szName) {
	  setName(szName);
	}
	else {
	  setName("BeOSImage");
	}
}

GR_BeOSImage::~GR_BeOSImage() {
	if (m_image) {
		delete m_image;
		m_image = NULL;
	}
}

UT_sint32 GR_BeOSImage::getDisplayWidth(void) const {
	return (m_image) ? ((UT_sint32)m_image->Bounds().Width()) : 0;
}

UT_sint32 GR_BeOSImage::getDisplayHeight(void) const {
	return (m_image) ? ((UT_sint32)m_image->Bounds().Height()) : 0;
}

bool	GR_BeOSImage::convertToBuffer(UT_ByteBuf** ppBB) const {
	/*
	  The purpose of this routine is to convert our internal bitmap
	  into a PNG image, storing it in a ByteBuf and returning it
	  to the caller.
	*/
	*ppBB = NULL;
	UT_ASSERT(m_image);

	BTranslatorRoster *roster = BTranslatorRoster::Default(); 
	BBitmapStream stream(m_image); 	// init with contents of bitmap 

      	//BFile file(filename, B_CREATE_FILE | B_WRITE_ONLY); 
	BMallocIO memory;
	if (roster->Translate(&stream, NULL, NULL, &memory, B_PNG_FORMAT) != B_NO_ERROR)
		return(false); 

	//Assuming that the translation went well we want to
	//stick it all into a byte buffer
	UT_ByteBuf *pBB = new UT_ByteBuf();
	if (!pBB || !memory.BufferLength() || 
	    !pBB->ins(0, (UT_Byte *)memory.Buffer(), memory.BufferLength()))
		return(false);

	return(true);

#if 0
	// Create our bytebuf
	UT_ByteBuf* pBB = new UT_ByteBuf();

		*ppBB = NULL;
		return false;

	// And pass the ByteBuf back to our caller
	*ppBB = pBB;

	return true;
#endif
}

bool	GR_BeOSImage::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	BBitmap 	*image;
	BMemoryIO	memio(pBB->getPointer(0), pBB->getLength());

	printf("IMAGE: Convert from PNG \n");

	//Use the translation library callouts
	if ((image = BTranslationUtils::GetBitmap(&memio)) == NULL)
		return(false);
	m_image = image;
	return(true);
}

GR_Image * GR_BeOSImage::createImageSegment(GR_Graphics * pG,const UT_Rect & rec)
{
	UT_sint32 x = pG->tdu(rec.left);
	UT_sint32 y = pG->tdu(rec.top);
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	UT_sint32 dH = getDisplayHeight();
	UT_sint32 dW = getDisplayWidth();
	if(height > dH)
	{
		height = dH;
	}
	if(width > dW)
	{
		width = dW;
	}
	if(x + width > dW)
	{
		width = dW - x;
	}
	if(y + height > dH)
	{
		height = dH - y;
	}
	if(width < 0)
	{
		x = dW -1;
		width = 1;
	}
	if(height < 0)
	{
		y = dH -1;
		height = 1;
	}
	UT_String sName("");
	getName(sName);
    UT_String sSub("");
	UT_String_sprintf(sSub,"_segemnt_%d_%d_%d_%d",x,y,width,height);
	sName += sSub;
	GR_BeOSImage * pImage = new GR_BeOSImage(sName.c_str());
	UT_ASSERT(m_image);
	pImage->m_image = new BBitmap(BRect(x, y, x+width, y+height), m_image->ColorSpace(), true);
	int pxsize = (int)(m_image->BytesPerRow() / m_image->Bounds().Width());
	char *src = (char *)m_image->Bits();
	src = src + pxsize * (x-1);
	for(int i=1;i<=height;i++)
	{
		pImage->m_image->SetBits(src, width, pImage->m_image->BytesPerRow()*(i-1), m_image->ColorSpace());
		if(i<height)
			src=src+m_image->BytesPerRow();
	}
	return static_cast<GR_Image *>(pImage);
}


 bool GR_BeOSImage::hasAlpha(void) const 
 {
		 return true;
 }


bool GR_BeOSImage::isTransparentAt(UT_sint32 x, UT_sint32 y) 
{
		return true;
}
