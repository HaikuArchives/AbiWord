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

#include <Debug.h>
#include <Application.h>
#include <Cursor.h>
#include <String.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gr_BeOSGraphics.h"
#include "gr_BeOSImage.h"
#include "gr_CharWidthsCache.h"
#include "gr_CharWidths.h"
#include "be_BackView.h"

#include "xap_BeOSFrameImpl.h"	//For be_DocView 
#include <limits.h>
#include <Font.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"


/*
  OPTIMIZATIONS:
  -Make BView callbacks for the scroll code that would
  replay a previously recorded BPicture.
  -Get rid of all the sync calls 
*/

//#define DPRINTF(x) x
#define DPRINTF(x) 

inline uint32 
utf8_char_len(uchar byte) 
{ 
	return (((0xE5000000 >> ((byte >> 3) & 0x1E)) & 3) + 1); 
}

/*const char* GR_BeOSGraphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
	return pszFontFamily;
}*/

GR_BeOSGraphics::GR_BeOSGraphics(BBackView *docview, XAP_App * app) 
{

	dontflush = false;
	m_pApp = app;
	m_pBeOSFont = NULL;
	m_pFontGUI = NULL;
	m_pPrintSettings = NULL;
	m_pPrintJob = NULL;
 	m_bPrint = FALSE;  
	if (!docview)
		docview = new BBackView(NULL, BRect(0,0,1,1), "", B_FOLLOW_ALL, 0);

	m_pFrontView = docview;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	
	/*
	  white for _highlight & _bevelup
	  black for foreground
	  lite gray(192*) for background
	  dark gray(128*) for beveldown          
	*/
	
	/* 21-02-2007*/
	
	if (m_pFrontView->LockLooper())
	{
		m_pFrontView->SetViewCursor(B_CURSOR_SYSTEM_DEFAULT,TRUE);
		m_pFrontView->UnlockLooper();
	}
	
	rgb_color c;
	c.alpha = 255;
	c.red = c.blue = c.green = 0;		//Black
	m_3dColors[CLR3D_Foreground] = c;
	c.red = c.blue = c.green = 219;		//Light Grey
	m_3dColors[CLR3D_Background] = c;
	c.red = c.blue = c.green = 150;		//Dark Grey
	m_3dColors[CLR3D_BevelDown] = c;
	c.red = c.blue = c.green = 255;		//White
	m_3dColors[CLR3D_Highlight] = c;
	c.red = c.blue = c.green = 255;		//White
	m_3dColors[CLR3D_BevelUp] = c;
}		

GR_BeOSGraphics::~GR_BeOSGraphics() 
{
}

void GR_BeOSGraphics::ResizeBitmap(BRect r) 
{
	BView * m_pShadowView = m_pFrontView->BackView();
	if(m_pShadowView->LockLooper())
	{
		m_pShadowView->ResizeTo(r.Width(), r.Height());
		m_pShadowView->UnlockLooper();
	}
}


bool GR_BeOSGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_OPAQUEOVERLAY:
		case DGP_SCREEN:
			return true;
		case DGP_PAPER:			//Not sure what this does
			return false;
		default:
			UT_ASSERT(0);
			return false;
	}
}

 bool BeOSFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
 {
		 return true;
 }
//
void GR_BeOSGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT(UT_TODO);
}

// Draw this string of characters on the screen in current font
void GR_BeOSGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
	int iLength, UT_sint32 xoff, UT_sint32 yoff, int * pCharWidths)
{
	int i;
	BString buffer("");
	UT_sint32 idy = _tduY(yoff+getFontHeight()*0.75);
	UT_sint32 idx = _tduX(xoff);
	
	for(i=0;i<iLength;i++)
	{
		char * utf8char;
		utf8char =  UT_encodeUTF8char(pChars[i]);
		buffer << *utf8char;				
	}

	
	UT_DEBUGMSG(("drawing string %s\n", buffer.String()));

	BView * m_pShadowView = m_pFrontView->BackView();	
	if (m_pShadowView->LockLooper()) 
	{	
		// If we use B_OP_OVER, our text will anti-alias correctly against
		// e.g. the ruler and the status bar.
		// scale to Zoom div 100
		
		drawing_mode oldMode = m_pShadowView->DrawingMode();
		m_pShadowView->SetDrawingMode(B_OP_OVER);
//		m_pShadowView->SetScale(getZoomPercentage()/100.);
		/*DH:
		 * I just had a brainstorm on how to fix the jitter problem.
		 * Let's measure the string, and draw using the charwidths Abi thinks
		 * we are using. This involves moving the pen and using DrawChar, which sucks,
		 * but let's see if it works.
		 * And it does.
		 * Perfectly.
		 */
		// TODO: need remapGlyph() before the following function call?
		
		if (!pCharWidths)
		{
			m_pShadowView->MovePenTo(idx, idy);
			m_pShadowView->DrawString(buffer.String(), iLength);
		}
		else
		{
			m_pShadowView->DrawChar(buffer[iCharOffset], BPoint(idx, idy));		
			for(i=iCharOffset+1; i<iLength; i++)
			{
				idx+=_tduX(pCharWidths[i-iCharOffset-1]);
				m_pShadowView->DrawChar(buffer.ByteAt(i-iCharOffset), BPoint(idx, idy));
			}
		}						  
		// Restore the old drawing mode & scale
//		m_pShadowView->SetScale(1.0);
		m_pShadowView->SetDrawingMode(oldMode);
		m_pShadowView->UnlockLooper();
	}
}

BFont *findClosestFont(const char* pszFontFamily, 
		 	const char* pszFontStyle, 
			const char* pszFontWeight) {

	BFont 	*aFont = new BFont();
	font_family family;
	font_style style; 
	uint32 flags; 
	int32 i;

	//Try and get a good family ... or one that matches
	int32 numFamilies = count_font_families(); 
   	for (i = 0; i < numFamilies; i++ ) { 
       uint32 flags; 
       if ((get_font_family(i, &family, &flags) == B_OK) &&  
           (strcmp(family, pszFontFamily) == 0)) {
           DPRINTF(printf("Font Match %s and %s \n", family, pszFontFamily));
           break;
       }
	}
	if (i >= numFamilies) {
		strcpy(family, "Dutch801 Rm BT");
		DPRINTF(printf("Didn't find font to match %s \n", pszFontFamily));
	}

#define REGULAR_BIT	0x1
#define ROMAN_BIT	0x2
#define BOLD_BIT	0x4
#define ITALIC_BIT	0x8
#define BOLD_ITALIC_BIT	0x10
	
	//Then try and match the styles 
	//(sub normal for Regular if possible, Roman if required)
	int32 numStyles = count_font_styles(family); 
	int32 stylemask = 0;
	for ( int32 j = 0; j < numStyles; j++ ) { 
		if ( get_font_style(family, j, &style, &flags) == B_OK ) {
			if (strcmp(style, "Regular") == 0) 
				stylemask |= REGULAR_BIT;
			else if (strcmp(style, "Roman") == 0)
				stylemask |= ROMAN_BIT;
			else if (strcmp(style, "Italic") == 0)
				stylemask |= ITALIC_BIT;
			else if (strcmp(style, "Bold") == 0)
				stylemask |= BOLD_BIT;
			else if (strcmp(style, "Bold Italic") == 0)
				stylemask |= BOLD_ITALIC_BIT;			
		} 
	}
	
	int32 targetstyle = 0;								 
	if ((strcmp(pszFontStyle, "italic") == 0) &&
	    (strcmp(pszFontWeight, "bold") == 0)) {
	    targetstyle |= BOLD_BIT | ITALIC_BIT | BOLD_ITALIC_BIT;
	}
	else if (strcmp(pszFontStyle, "italic") == 0) 
		targetstyle |= ITALIC_BIT;
	else if (strcmp(pszFontWeight, "bold") == 0) 
		targetstyle |= BOLD_BIT;
	else 
		targetstyle |= ROMAN_BIT | REGULAR_BIT;
		
	//Search order preference
	//Bold Italic --> Bold Italic; 
	//Bold    --> Bold
	//Italic  --> Italic
	//Regular --> Normal/Roman
	if (targetstyle & stylemask & BOLD_ITALIC_BIT) 
		strcpy(style, "Bold Italic");
	else if (targetstyle & stylemask & ITALIC_BIT) 
		strcpy(style, "Italic");
	else if (targetstyle & stylemask & BOLD_BIT) 
		strcpy(style, "Bold");
	else if (targetstyle & stylemask & ROMAN_BIT) 
		strcpy(style, "Roman");
	else if (targetstyle & stylemask & REGULAR_BIT) 
		strcpy(style, "Regular");

	DPRINTF(printf("Setting Style %s \n", style));
	
	aFont->SetFamilyAndStyle((strlen(family) == 0) ? NULL : family, 
	                         (strlen(style) == 0) ? NULL : style);
	return(aFont);
}

GR_Font* GR_BeOSGraphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		BFont *aFont = new BFont();
		m_pFontGUI = new BeOSFont(aFont);
	}
	return m_pFontGUI;
}

GR_Font* GR_BeOSGraphics::_findFont(const char* pszFontFamily, 
								  const char* pszFontStyle, 
								  const char* pszFontVariant, 
								  const char* pszFontWeight, 
								  const char* pszFontStretch, 
								  const char* pszFontSize)
{
	BFont 	*aFont;
	//int 	size = atoi(pszFontSize);
	//UT_sint32 iHeight = convertDimension(pszFontSize);
	UT_sint32 size = (UT_sint32)UT_convertToPoints(pszFontSize);	
	//int		size = convertDimension(pszFontSize);

	DPRINTF(printf("GR Find Font:\n\tFamily: %s ", pszFontFamily));
	DPRINTF(printf("\n\tStyle: %s ", pszFontStyle));
	DPRINTF(printf("\n\tWeight: %s ", pszFontWeight));
	DPRINTF(printf("\n\tSize: %s (%d) ", pszFontSize, size));
	
	aFont = findClosestFont(pszFontFamily, pszFontStyle, pszFontWeight);
	aFont->SetSize(size);
	DPRINTF(printf("GR: -- Located Font: \n"));
	DPRINTF(aFont->PrintToStream());
	m_pBeOSFont = new BeOSFont(aFont);
	return(m_pBeOSFont);
}
//
//void GR_BeOSGraphics::getCoverage(UT_NumberVector& coverage)
//{
//	m_pFont->get_font()->getCoverage(coverage);
//}

//Set the font to something (I guess we set pFont to be like BFont somewhere)
void GR_BeOSGraphics::setFont(GR_Font* pFont)
{
	BeOSFont *tmpFont;
	
	tmpFont = static_cast<BeOSFont*> (pFont);
	UT_ASSERT(tmpFont);
	
	m_pBeOSFont = tmpFont;
	m_pBeOSFont->get_font()->SetSpacing(B_BITMAP_SPACING);

	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		if (m_pBeOSFont)
			m_pShadowView->SetFont(m_pBeOSFont->get_font());
		else
			UT_DEBUGMSG(("HEY! NO FONT INFORMATION AVAILABLE!\n"));
		m_pShadowView->UnlockLooper();
	}
}

//Get the height of the font
UT_uint32 GR_BeOSGraphics::getFontHeight()
{
	return getFontAscent()+getFontDescent();
}

UT_uint32 GR_BeOSGraphics::getFontAscent()
{
	font_height fh;

	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->GetFontHeight(&fh);
		m_pShadowView->UnlockLooper();
	}
	//Gives ascent, descent, leading
	//printf("GR: Font Ascent %d\n",(int)(fh.ascent + 0.5));
	return((UT_uint32)((fh.ascent)* getResolution() / s_getDeviceResolution()));
}

UT_uint32 GR_BeOSGraphics::getFontDescent()
{
	font_height fh;
	BView * m_pShadowView = m_pFrontView->BackView();	
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->GetFontHeight(&fh);
		m_pShadowView->UnlockLooper();
	}
	//Gives ascent, descent, leading
	//printf("GR: Font Descent %d\n",(int)(fh.descent + 0.5));
	return((UT_uint32)((fh.descent)* getResolution() / s_getDeviceResolution()));
}


UT_uint32 GR_BeOSGraphics::getFontAscent(GR_Font *font)
{
	font_height fh;
	BeOSFont* bFont = static_cast<BeOSFont *>(font);

	bFont->get_font()->GetHeight(&fh);
	
	return((UT_uint32)((fh.ascent)* getResolution() / s_getDeviceResolution()));
}

UT_uint32 GR_BeOSGraphics::getFontDescent(GR_Font *font)
{
	font_height fh;
	BeOSFont* bFont = static_cast<BeOSFont *>(font);

	bFont->get_font()->GetHeight(&fh);
	
	return((UT_uint32)((fh.descent)* getResolution() / s_getDeviceResolution()));
}

UT_uint32 GR_BeOSGraphics::getFontHeight(GR_Font *font)
{
	return getFontAscent(font)+getFontDescent(font);
}

UT_sint32 GR_BeOSGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	//Just return value from widthcahce converted to logical units
	//if no width, it calls BeOSFont's measure itself
	BFont aFont;
	m_pFrontView->BackView()->GetFont(&aFont);
	float fontsize = aFont.Size();
	return tlu(fontsize*(getGUIFont()->getCharWidthFromCache(c)/100.));
}

void GR_BeOSGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_BeOSGraphics::setColor(const UT_RGBColor& clr)
{
	DPRINTF(printf("GR: setColor\n"));
	m_curColor = clr;
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->SetHighColor(clr.m_red, clr.m_grn, clr.m_blu);
		if(m_pShadowView->Window()->IsLocked())
			m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
							UT_sint32 y2)
{
	DPRINTF(printf("GR: Draw Line\n"));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{		
		UT_sint32 idx1 = _tduX(x1);
		UT_sint32 idx2 = _tduX(x2);

		UT_sint32 idy1 = _tduY(y1);
		UT_sint32 idy2 = _tduY(y2);
		m_pShadowView->StrokeLine(BPoint(idx1, idy1),// BPoint(idx2, idy2));
		beosiseLineEnding(idx1, idy1, idx2, idy2));
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	DPRINTF(printf("GR: Set Line Width %d \n", iLineWidth));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->SetPenSize(iLineWidth);
		m_pShadowView->UnlockLooper();
	}
}

/*Poly line is only used during drawing squiggles*/
void GR_BeOSGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	DPRINTF(printf("GR: Poly Line \n"));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		for (UT_uint32 k=1; k<nPoints; k++)
			drawLine(_tduX(pts[k-1].x),_tduY(pts[k-1].y), _tduX(pts[k].x),_tduY(pts[k].y)); 
		m_pShadowView->UnlockLooper();
	}
}


void GR_BeOSGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	DPRINTF(printf("GR: XOR Line\n"));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		drawing_mode oldmode = m_pShadowView->DrawingMode();
		m_pShadowView->SetDrawingMode(B_OP_INVERT);//or B_OP_BLEND
		UT_sint32 idx1 = _tduX(x1);
		UT_sint32 idy1 = _tduY(y1);
		UT_sint32 idx2 = _tduX(x2);
		UT_sint32 idy2 = _tduY(y2);
		m_pShadowView->StrokeLine(BPoint(idx1, idy1), BPoint(idx2, idy2));
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::invertRect(const UT_Rect* pRect)
{
	DPRINTF(printf("GR: Invert Rect\n"));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
	/*Dan thinks StrokeRect is only getting the outer edges, when we don't really want that. we'll see in a sec*/
		drawing_mode oldmode = m_pShadowView->DrawingMode();
		//printf("Inverting rect\n");
		m_pShadowView->SetDrawingMode(B_OP_INVERT);	//or B_OP_BLEND
		UT_sint32 idx = _tduX(pRect->left);
		UT_sint32 idy = _tduY(pRect->top);
		UT_sint32 idw = _tduX(pRect->width);
		UT_sint32 idh = _tduY(pRect->height);
		m_pShadowView->/*Stroke*/FillRect(BRect(idx, idy,
									idx + idw -1,
									idy + idh -1));
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->UnlockLooper();
	}
}


void GR_BeOSGraphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
						UT_sint32 w, UT_sint32 h)
{
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		rgb_color old_colour = m_pShadowView->HighColor();
		m_pShadowView->SetHighColor(c.m_red, c.m_grn, c.m_blu);
		UT_sint32 idx = _tduX(x);
		UT_sint32 idy = _tduY(y);
		UT_sint32 idw = _tduX(w);
		UT_sint32 idh = _tduY(h);
//		UT_DEBUGMSG(("GR: Flll Rect!! color (r:%i,g:%i,b:%i)\n", c.m_red, c.m_grn, c.m_blu));
//		UT_DEBUGMSG(("GR: Flll Rect!! rect (%i,%i,%i,%i,%i)\n", idx, idy, idx+idw-1, idy+idh-1));	
		BRect r(idx, idy, idx+idw-1, idy+idh-1);
		m_pShadowView->FillRect(r);
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::setClipRect(const UT_Rect* pRect)
{
	
	m_pRect = pRect;
	BRegion region;
	BRegion *r = NULL;
	if (pRect) {
		DPRINTF(printf("GR: Set Clip Rect: %d-%d -> %d-%d\n", 
				pRect->left, pRect->top, 
				pRect->left+pRect->width,
				pRect->top+pRect->height));
//knorr
		UT_sint32 idx = _tduX(pRect->left);
		UT_sint32 idy = _tduY(pRect->top);
		UT_sint32 idw = _tduX(pRect->width);
		UT_sint32 idh = _tduY(pRect->height);
		region.Set(BRect(idx, idy, 
				 idx+idw-1,	idy+idh-1));
//knorr
		r = &region;
	}
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->ConstrainClippingRegion(r);
		m_pShadowView->UnlockLooper();
	}
}
//дабы не мешалось
//#if defined(METHOD_PRE_R4)
//	//This is slow and crappy method, but it works
//	//when you don't have a CopyBits function
//	BRegion region;
//	if (m_pShadowView->LockLooper())
//	{
//
//	//If we are moving down, right offset positive
//		BRect r = m_pShadowView->Bounds();
//		(dy < 0) ? (r.top -= dy) : (r.bottom -= dy);
//		(dx < 0) ? (r.left -= dx) : (r.right -= dx);
//		printf("Invalidating "); r.PrintToStream();
//		region.Set(BRect(pRect->left, pRect->top, 
//				 pRect->left+pRect->width,
//				pRect->top+pRect->height));
//		m_pShadowView->ConstrainClippingRegion(&region);
//		m_pShadowView->Invalidate(r);
//		m_pShadowView->UnlockLooper();
//	}
//#endif

void GR_BeOSGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{

	DPRINTF(printf("GR: Scroll dx %d dy %d\n", dx, dy));
	//This method lets the app server draw for us
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		BRect src, dest;
		dest = src = m_pShadowView->Bounds();
		UT_sint32 ddx = _tduX(dx);
		UT_sint32 ddy = _tduY(dy);
		dest.OffsetBy(-1*ddx, -1*ddy);
		//printf("Scroll SRC "); src.PrintToStream();
		//printf("Scroll DST "); dest.PrintToStream();
		m_pShadowView->CopyBits(src, dest);
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
			  UT_sint32 x_src, UT_sint32 y_src,
			  UT_sint32 width, UT_sint32 height)
{
	UT_sint32 dx = x_src-x_dest;
	UT_sint32 dy = y_src-y_dest;
	scroll(dx, dy);
//	DEBUGGER("GR: MoveArea\n");
//	printf("GR: Move Area\n");
//	UT_ASSERT(0);
}

void GR_BeOSGraphics::clearArea(UT_sint32 x, UT_sint32 y,
			     UT_sint32 w, UT_sint32 h)
{
	UT_DEBUGMSG(("GR: Clear Area %d-%d -> %d-%d\n", 
					x, y, x+w, y+h));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		rgb_color old_colour = m_pShadowView->HighColor();
		m_pShadowView->SetHighColor(m_pShadowView->ViewColor());
		UT_sint32 idx = _tduX(x);
		UT_sint32 idy = _tduY(y);
		UT_sint32 idw = _tduX(w);
		UT_sint32 idh = _tduY(h);
		m_pShadowView->FillRect(BRect(idx, idy, idx+idw-1, idy+idh-1));
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->UnlockLooper();
	}
}

bool GR_BeOSGraphics::startPrint(void)
{
	if (!m_pPrintJob) {
		printf("Creating a new print job \n");
		m_pPrintJob = new BPrintJob("ADD A NAME HERE");
	}
	if (!m_pPrintJob) {
		printf("No print job ... exiting \n");
		return(false);
	}

	if (!m_pPrintSettings) {
		printf("I SHOULD NEVER BE HERE! \n");
		return(false);
		if (m_pPrintJob->ConfigPage() != B_OK) {
			return(false);
		}

		if (m_pPrintJob->ConfigJob() != B_OK) {
       		         return(false);
       		}
		m_pPrintSettings = m_pPrintJob->Settings();
	}
	m_pPrintJob->SetSettings(m_pPrintSettings);

	printf("Paper Rect: "); m_pPrintJob->PaperRect().PrintToStream();
	printf("Print Rect: "); m_pPrintJob->PrintableRect().PrintToStream();

	m_pPrintJob->BeginJob();

	//Make sure that we start spooling at the right time
	m_bPrint = FALSE;
	return(true);
}

bool GR_BeOSGraphics::startPage(const char * /*szPageLabel*/, 
				   UT_uint32 /*pageNumber*/,
				   bool /*bPortrait*/, 
				   UT_uint32 /*iWidth*/, 
				   UT_uint32 /*iHeight*/) 
{
	BView * m_pShadowView = m_pFrontView->BackView();

	if (!m_pPrintJob || !m_pPrintJob->CanContinue() || !m_pShadowView) {
		printf("GR: Start page something amiss \n");
		return(false);
	}

	if (m_bPrint) {
		BPicture *tmppic;
		BRect     r;

		if(m_pShadowView->LockLooper())
		{	
			r = m_pShadowView->Bounds();
			tmppic = m_pShadowView->EndPicture();
			m_pShadowView->UnlockLooper();
		}
		((be_DocView *)m_pShadowView)->SetPrintPicture(tmppic);
		m_pPrintJob->DrawView(m_pShadowView, 
				      //BRect(0, 0, 600, 600), 
				      BRect(0, 0, SHRT_MAX-1, SHRT_MAX-1), 
				      BPoint(0,0));
		
		//Commit this page and move to the next one
		m_pPrintJob->SpoolPage();
		delete(tmppic);
	}

	m_bPrint = TRUE;
	m_pShadowView->BeginPicture(new BPicture());

	return(true);
}

bool GR_BeOSGraphics::endPrint(void) 
{
	BView * m_pShadowView = m_pFrontView->BackView();
	if (!m_pPrintJob || !m_pPrintJob->CanContinue()) {
		return(false);
	}

	if (m_bPrint) {
		BPicture *tmppic;

		if (m_pShadowView->LockLooper())
		{
			tmppic = m_pShadowView->EndPicture();
			m_pShadowView->UnlockLooper();
		}
		else return(false);
		((be_DocView *)m_pShadowView)->SetPrintPicture(tmppic);
		m_pPrintJob->DrawView(m_pShadowView, 
				      //BRect(0, 0, FLT_MAX, FLT_MAX), 
				      BRect(0, 0, SHRT_MAX-1, SHRT_MAX-1), 
                                      BPoint(0,0));
		
		//Commit this page and move to the next one
		m_pPrintJob->SpoolPage();
		delete(tmppic);
	}

	((be_DocView *)m_pShadowView)->SetPrintPicture(NULL);
	m_pPrintJob->CommitJob();
	delete(m_pPrintJob);
	m_pPrintJob = NULL;
	return(true);
}

GR_Image* GR_BeOSGraphics::createNewImage(const char* pszName, 
					  const UT_ByteBuf* pBB,
					  UT_sint32 iDisplayWidth, 
					  UT_sint32 iDisplayHeight,
					  GR_Image::GRType iType)
{
	DPRINTF(printf("GR: Create new image %s \n", pszName));
	BView * m_pShadowView = m_pFrontView->BackView();
		
	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
	   	pImg = new GR_BeOSImage(pszName);
   	else
     	pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
	return pImg;
}

void GR_BeOSGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);
	
   	if (pImg->getType() != GR_Image::GRT_Raster) {
      		pImg->render(this, xDest, yDest);
      		return;
   	}
   
	GR_BeOSImage * pBeOSImage = static_cast<GR_BeOSImage *>(pImg);
	BBitmap* image = pBeOSImage->getData();
	if (!image)
		return;
	//UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
	//UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();

	BView * m_pShadowView = m_pFrontView->BackView();
	if(m_pShadowView->LockLooper())
	{
		m_pShadowView->DrawBitmapAsync(image, BPoint(xDest, yDest)); 
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::flush(void)
{	
	UT_DEBUGMSG(("flushing BackView to front\n"));
	BBitmap  * bmp = m_pFrontView->BackBmp();
	if(!bmp)
	{
		UT_DEBUGMSG(("no back bitmap!!\n"));
		m_pFrontView->Flush();
		return;
	}
		
	if(m_pFrontView->LockLooper())
	{
		m_pFrontView->BackView()->Flush();
		m_pFrontView->SetViewBitmap(bmp, B_FOLLOW_NONE, 0);
		m_pFrontView->Invalidate();
		m_pFrontView->UnlockLooper();
	}
}

void GR_BeOSGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	// TODO:  maybe? 
	m_cs = c;
}

GR_Graphics::ColorSpace GR_BeOSGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_BeOSGraphics::setCursor(GR_Graphics::Cursor c)
{

	if (m_cursor == c)
		return;
	
	m_cursor = c;
	UT_DEBUGMSG(("In set cursor\n"));
	const BCursor * tc = B_CURSOR_SYSTEM_DEFAULT;
	
	switch (c)
	{
	default:
		break;

	case GR_CURSOR_IBEAM:
		tc = B_CURSOR_I_BEAM;
		break;
	}
	UT_DEBUGMSG(("Cursor drawing\n"));
	be_app->SetCursor(tc);
	be_app->ShowCursor();
}

GR_Graphics::Cursor GR_BeOSGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_BeOSGraphics::setColor3D(GR_Color3D c)
{
	DPRINTF(printf("Set color 3D %d \n", c));
	BView * m_pShadowView = m_pFrontView->BackView();
		
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->SetHighColor(m_3dColors[c]);
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32
 w, UT_sint32 h)
{
	DPRINTF(printf("GR:FillRect 3D %d!\n", c));
	BView * m_pShadowView = m_pFrontView->BackView();
		
	if(m_pShadowView->LockLooper())
	{	
		rgb_color old_colour = m_pShadowView->HighColor();
		drawing_mode oldmode=m_pShadowView->DrawingMode();
		m_pShadowView->SetHighColor(m_3dColors[c]);
		m_pShadowView->SetDrawingMode(B_OP_COPY);
		UT_sint32 idx = _tduX(x);
		UT_sint32 idy = _tduY(y);
		UT_sint32 idw = _tduX(w);
		UT_sint32 idh = _tduY(h);
		m_pShadowView->FillRect(BRect(idx, idy, idx+idw-1, idy+idh-1));
		m_pShadowView->SetHighColor(old_colour);
		m_pShadowView->SetDrawingMode(oldmode);
		m_pShadowView->UnlockLooper();
	}
}

void GR_BeOSGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	DPRINTF(printf("Fill rectangle\n"));
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}                               

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////
void GR_Font::s_getGenericFontProperties(const char * szFontName,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = true;
}
//
UT_uint32 GR_BeOSGraphics::s_getDeviceResolution(void)
{
	// native BeOS resolution
	return 72;
}
//
UT_uint32 GR_BeOSGraphics::getDeviceResolution(void) const
{
	return s_getDeviceResolution();
}
//
void GR_BeOSGraphics::setLineProperties ( double inWidth, 
										  GR_Graphics::JoinStyle inJoinStyle,
										  GR_Graphics::CapStyle inCapStyle,
										  GR_Graphics::LineStyle inLineStyle )
{
	DPRINTF(printf("Set line properties\n"));
	BView * m_pShadowView = m_pFrontView->BackView();
	if (m_pShadowView->LockLooper())
	{
		m_pShadowView->SetPenSize(tduD(inWidth));
		//TODO: add styles here
		m_pShadowView->UnlockLooper();
	}
	
}
//
UT_sint32 BeOSFont::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
	char buffer[10];

	float escapementArray[1];
	
	char * utf8char;
	utf8char =  UT_encodeUTF8char(cChar);
	strcpy(buffer, utf8char);						

	escapement_delta tempdelta;
	tempdelta.space=0.0;
	tempdelta.nonspace=0.0;
	m_pBFont->SetSpacing(B_CHAR_SPACING);		

	m_pBFont->GetEscapements(buffer,1,&tempdelta,escapementArray);
	UT_sint32 retval = (UT_sint32)ceil(100.*escapementArray[0]);
	return retval;
}
//
void GR_BeOSGraphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx)
{
	UT_Rect* oldR = NULL;	

	m_vSaveRect.setNthItem(iIndx, new UT_Rect(r),&oldR);
	if(oldR) delete oldR;

	BBitmap * oldC = NULL;
	UT_sint32 idx = _tduX(r.left);
	UT_sint32 idy = _tduY(r.top);
	UT_sint32 idw = _tduX(r.width);
	UT_sint32 idh = _tduY(r.height);
	
	BRect rect(idx, idy, idx+idw-1, idy+idh-1);
	BBitmap * m_pShadowBitmap = ShadowBitmap();
	BBitmap * bmp = new BBitmap(rect, m_pShadowBitmap->ColorSpace(), true);
	BView * view = new BView(rect,"copyrect", 0, B_WILL_DRAW);
	bmp->AddChild(view);
	if(view->LockLooper())
	{
		view->DrawBitmap(m_pShadowBitmap, rect, rect);
		view->UnlockLooper();
	}
	m_vSaveRectBuf.setNthItem(iIndx, bmp, &oldC);

	DELETEP(oldC);	
	bmp->RemoveChild(view);
	DELETEP(view);
}
//
void GR_BeOSGraphics::restoreRectangle(UT_uint32 iIndx)
{
	UT_Rect *r = m_vSaveRect.getNthItem(iIndx);
	BBitmap *p = m_vSaveRectBuf.getNthItem(iIndx);
	UT_DEBUGMSG(("restore rectangle\n"));
	if (p && r)
	{
		UT_DEBUGMSG(("restore rectangle b4 lock\n"));
		if(m_pFrontView->LockLooper())
		{
			UT_DEBUGMSG(("restore rectangle after lock\n"));
			UT_sint32 idx = _tduX(r->left);
			UT_sint32 idy = _tduY(r->top);
			UT_sint32 idw = _tduX(r->width);
			UT_sint32 idh = _tduY(r->height);
			BRect rect(idx, idy, idx+idw-1, idy+idh-1);			
			m_pFrontView->DrawBitmap(p, rect);
			m_pFrontView->UnlockLooper();
			//little hack
			//no need to redraw full view
			dontflush = true;
		}
	}
}
//
GR_Graphics *   GR_BeOSGraphics::graphicsAllocator(GR_AllocInfo& allocInfo)
{
	GR_BeOSAllocInfo &allocator = (GR_BeOSAllocInfo&)allocInfo;
	return new GR_BeOSGraphics(allocator.m_win, allocator.m_app);
}
//
GR_Font* GR_BeOSGraphics::getDefaultFont(UT_String& fontFamily)
{
	font_family fam;
	font_style tmp;
	BFont *aFont = new BFont();
	BeOSFont * fontHandle = new BeOSFont(aFont);
	aFont->GetFamilyAndStyle(&fam, &tmp);
	fontFamily = fam;
	return fontHandle;
}
//
void GR_BeOSGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	GR_Graphics::setZoomPercentage (iZoom); // chain up
}
//
/*!
 * Take a screenshot of the graphics and convert it to an image.
 */
GR_Image * GR_BeOSGraphics::genImageFromRectangle(const UT_Rect &rec)
{
	UT_sint32 idx = _tduX(rec.left);
	UT_sint32 idy = _tduY(rec.top);
	UT_sint32 idw = _tduX(rec.width);
	UT_sint32 idh = _tduY(rec.height);

	BRect rect(idx, idy, idx+idw-1, idy+idh-1);
	BBitmap * m_pShadowBitmap = ShadowBitmap();
	BBitmap * bmp = new BBitmap(rect, m_pShadowBitmap->ColorSpace(), true);
	BView * view = new BView(rect,"ScreenShotView", 0, B_WILL_DRAW);
	bmp->AddChild(view);
	if(view->LockLooper())
	{
		view->DrawBitmap(m_pShadowBitmap, rect, rect);
		view->UnlockLooper();
	}
	GR_BeOSImage * pImg = new GR_BeOSImage("ScreenShot");
	pImg->setData(bmp);
	bmp->RemoveChild(view);
	delete view;
	return static_cast<GR_Image *>(pImg);
}
//
void GR_BeOSGraphics::_beginPaint ()
{
}
//
void GR_BeOSGraphics::_endPaint()
{	
	if(!dontflush)
		flush();
	else
		dontflush=false;
}
//
void GR_BeOSGraphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
//TODO: draw polygon
}
//
BBitmap * GR_BeOSGraphics::ShadowBitmap()
{
	return(m_pFrontView->BackBmp()); 
}
//
