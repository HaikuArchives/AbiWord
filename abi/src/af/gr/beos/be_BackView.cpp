#include <String.h>
#include <Bitmap.h>
#include "be_BackView.h"
#include "ut_debugmsg.h"

BBackView::BBackView(AV_View * pView, BRect frame, const char* name, uint32 resizingMode, uint32 flags) :
	BView(frame, name, resizingMode, flags | B_WILL_DRAW),
	backBmp(NULL),
	backView(NULL),
	m_pView(pView)
{
	backView = new BView(Bounds(), BString("bv").Append(name).String(), B_FOLLOW_NONE, 0);
}

void BBackView::SetView(AV_View *pView) 
{
	m_pView = pView;
}

void BBackView::AttachedToWindow()
{
	UT_DEBUGMSG(("BackView \"%s\" attached to window\n", backView->Name()));
	FrameResized(Bounds().Width(), Bounds().Height());
}

void BBackView::FrameResized(float width, float height)
{
	if(backBmp)
	{
		backBmp->RemoveChild(backView);
		delete backBmp;
	}
	
	backView->ResizeTo(width, height);
	backBmp = new BBitmap(BRect(0, 0, width, height), B_RGB32, true);
	backBmp->AddChild(backView);
}
