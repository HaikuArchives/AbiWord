#ifndef _BACKVIEW_H_
#define _BACKVIEW_H_

#include <View.h>

class AV_View;

class BBackView : public BView 
{
public:
	BBackView(AV_View * pView, const char* name, uint32 flags);
	virtual void AttachedToWindow();
	virtual void FrameResized(float width, float height);
	virtual void SetView(AV_View *pView);	
	BView * BackView() { return backView; };	
	BBitmap * BackBmp() { return backBmp; };
	AV_View * m_pView;	
private:
	BBitmap * backBmp;
	BView * backView;
};

#endif
