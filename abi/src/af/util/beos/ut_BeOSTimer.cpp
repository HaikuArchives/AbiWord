/* AbiSource Program Utilities
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
 

#include "ut_BeOSTimer.h"

#include <Application.h>
#include <MessageRunner.h>


static const int kTimerFired = 'fire';


UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData,
	GR_Graphics * /*pG*/)
{
	UT_ASSERT(pCallback);
	UT_BeOSTimer * p = new UT_BeOSTimer(pCallback, pData);
	return p;
}


UT_BeOSTimer::UT_BeOSTimer(UT_TimerCallback pCallback, void* pData)
	: fRunner(NULL)
{
	setCallback(pCallback);
	setInstanceData(pData);
	setIdentifier((int32)this);

	if (be_app->LockLooper()) {
		be_app->AddHandler(this);
		be_app->UnlockLooper();
	}
}


UT_BeOSTimer::~UT_BeOSTimer()
{
	stop();
	be_app->RemoveHandler(this);
	delete fRunner;
}


UT_sint32 UT_BeOSTimer::set(UT_uint32 iMilliseconds)
{
	/*
	  The goal here is to set this timer to go off after iMilliseconds
	  have passed.  This method should not block.  It should call some
	  OS routine which provides timing facilities.  It is assumed that this
	  routine requires a C callback.  That callback, when it is called,
	  must look up the UT_Timer object which corresponds to it, and
	  call its fire() method.  See ut_Win32Timer.cpp for an example
	  of how it's done on Windows.  We're hoping that something similar 
	  will work for other platforms.
	*/
	if (fRunner)
		delete fRunner;
	fRunner = new BMessageRunner(BMessenger(this), new BMessage(kTimerFired),
		iMilliseconds * 1000);
}


void UT_BeOSTimer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	fRunner->SetCount(0);
}


void UT_BeOSTimer::start(void)
{
	// resume the delivery of events.
	fRunner->SetCount(-1);
}


void UT_BeOSTimer::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case kTimerFired:
			fire();
			break;
		default:
			BHandler::MessageReceived(message);
	}
}
