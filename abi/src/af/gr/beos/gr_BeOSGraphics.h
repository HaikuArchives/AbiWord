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

#ifndef GR_BEOSGRAPHICS_H
#define GR_BEOSGRAPHICS_H

#include <Font.h>
#include <GraphicsDefs.h>
#include <Point.h>
#include <View.h>
#ifdef __HAIKU__
#include <GridLayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#endif
#include "gr_Graphics.h"
#include "ut_assert.h"

class BBackView;
class BPrintJob;

class BeOSFont : public GR_Font 
{
public:
	BeOSFont(BFont *aFont) 	{ m_pBFont = aFont; };
	BFont *get_font(void) 	{ return(m_pBFont); };
	virtual bool glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG) ;
	
private:
	virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const;
	
	BFont	*m_pBFont;
};

class GR_BeOSAllocInfo : public GR_AllocInfo
{
public:
	GR_BeOSAllocInfo(BBackView * win, XAP_App * app) {m_win = win; m_app = app;};

	virtual GR_GraphicsId getType() const {return GRID_BEOS;};
	virtual bool isPrinterGraphics() const {return false; };
	BBackView* m_win;	
	XAP_App * m_app;
};

class GR_BeOSGraphics : public GR_Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	~GR_BeOSGraphics();

	static UT_uint32 s_getClassId() {return GRID_BEOS;}
	virtual UT_uint32 getClassId() {return s_getClassId();}

	virtual GR_Capability getCapability(){UT_ASSERT(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
	
	static const char *    graphicsDescriptor(void){return "BeOS Default";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	
	virtual void setFont(GR_Font* pFont);
	virtual void clearFont(void) {m_pBeOSFont = NULL;} ;
	virtual UT_uint32 getFontHeight();
	//virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c);
	virtual void setColor(const UT_RGBColor& clr);
	virtual void getColor(UT_RGBColor& clr);

	virtual GR_Font* getGUIFont(void);

	virtual GR_Font*	getDefaultFont(UT_String& fontFamily);

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();

	virtual void getCoverage(UT_NumberVector& coverage) {};
	virtual void setLineWidth(UT_sint32);	

	virtual void setClipRect(const UT_Rect* pRect);
	virtual void scroll(UT_sint32, UT_sint32);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);

	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType = GR_Image::GRT_Raster);
		
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
						   bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	
	virtual void      setZoomPercentage(UT_uint32 iZoom);
	virtual bool		endPrint(void);	

	virtual void flush(void);
	
	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;

	virtual void setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void setColor3D(GR_Color3D c);
	
	virtual void setLineProperties ( double inWidth, 
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID ) ;

	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);
	virtual GR_Image * genImageFromRectangle(const UT_Rect & r);
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	  restoreRectangle(UT_uint32 iIndx);

	static UT_uint32  s_getDeviceResolution(void);
	virtual UT_uint32 getDeviceResolution(void) const;


	//Added for local updating of the View
	void			ResizeBitmap(BRect r);
	BBitmap 		*ShadowBitmap();
	BMessage		*GetPrintSettings() 	
					{ return(m_pPrintSettings); };
	void 			SetPrintSettings(BMessage *s) 
					{ m_pPrintSettings = s; };
	BPrintJob		*GetPrintJob()
					{ return(m_pPrintJob); };
	void			SetPrintJob(BPrintJob *j)
					{ m_pPrintJob = j; };
 
	//Added for obtain background Color
	rgb_color		Get3DColor(GR_Graphics::GR_Color3D c)
    				{ return m_3dColors[c];};
 
protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_BeOSGraphics(BBackView *front, XAP_App *app);

	virtual void _beginPaint ();
	virtual void _endPaint ();
	virtual void drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff);
	virtual void drawChars(const UT_UCSChar* pChars, int iCharOffset,
						   int iLength, UT_sint32 xoff, UT_sint32 yoff,
						   int * pCharWidths = NULL);
	virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void fillRect(GR_Color3D c, UT_Rect &r);   
// to avoid warnings
	virtual void fillRect(const UT_RGBColor& c, const UT_Rect &r) {};	
//
	virtual void polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);				
// to avoid warnings
	virtual void fillRect(GR_Image *pImg, const UT_Rect &src, const UT_Rect & dest) {};
//
	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);			
	virtual void invertRect(const UT_Rect* pRect);	
	virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual GR_Font*	_findFont(const char* pszFontFamily, 
								  const char* pszFontStyle, 
								  const char* pszFontVariant, 
								  const char* pszFontWeight, 
								  const char* pszFontStretch, 
								  const char* pszFontSize);
		
	BBackView				*m_pFrontView;
//	BBitmap				*m_pShadowBitmap;
	BMessage				*m_pPrintSettings;	
	BPrintJob				*m_pPrintJob;
	BeOSFont				*m_pBeOSFont, *m_pFontGUI;
	GR_Graphics::ColorSpace m_cs;
	GR_Graphics::Cursor		m_cursor;
	rgb_color				m_3dColors[COUNT_3D_COLORS];
 	bool					m_bPrint;  

 	
 	// Takes a line and modifies it to fit with the BeOS pixel coordinate system.
 	// Returns a BPoint containing the modified end-point (x2, y2).
 	inline BPoint beosiseLineEnding(UT_sint32 x1, UT_sint32 y1, 
 									UT_sint32 x2, UT_sint32 y2);
private:
	UT_RGBColor				m_curColor;
	UT_GenericVector<UT_Rect*>	m_vSaveRect;
	UT_GenericVector<BBitmap *>	m_vSaveRectBuf;
	bool dontflush;
};

BPoint GR_BeOSGraphics::beosiseLineEnding(UT_sint32 x1, UT_sint32 y1, 
												 UT_sint32 x2, UT_sint32 y2) {
	// Work out the line's orientation
	int tell = abs(x1-x2) - abs(y1-y2);

	// If it's exactly diagonal...
	if(tell == 0) 
	{
		// Drawn top-left to bottom-right
		if(x1<x2 && y1<y2)
		{
			x2--;
			y2--;
		}
		// Drawn bottom-right to top-left
		else if(x1>x2 && y1>y2)
		{
			x2++;
			y2++;
		}
		// Top-right to bottom-left
		else if(x1>x2 && y1<y2)
		{
			x2++;
			y2--;
		}
		// Bottom-left to top-right
		else if(x1<x2 && y1>y2)
		{
			x2--;
			y2++;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		
	} 
	// ...else if it's at an acute angle to the horizontal... 
	else if (tell > 0)
	{
		// If the line is being drawn left-to-right
		if(x1<x2)
			x2--;
		else
			x2++;
	}
	// ...else it's at an obtuse angle to the horizontal.
	else // if (tell < 0) 
	{
		// If the line is being drawn top-to-bottom
		if(y1<y2)
			y2--;
		else
			y2++;
	}
	
	return(BPoint(x2, y2));
}


#endif /* GR_BEOSGRAPHICS_H */
