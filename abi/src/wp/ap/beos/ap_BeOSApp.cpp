/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_bytebuf.h"
#include "ut_string.h"
#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_BeOSFrame.h"
#include "ap_BeOSApp.h"

#include "fv_View.h"
#include "fp_Run.h"

// HACK: set this symbol to 0 to turn off command-line conversion
// TODO: if it works, remove it entirely
#ifndef CONVERT
#define CONVERT 1	
#endif

#if	CONVERT
#include "ap_Convert.h"
#endif
#include "sp_spell.h"
#include "ap_Strings.h"

#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_ActionSet.h"       

#include "ap_Clipboard.h"

#include "ie_impexp_Register.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"



// Signal handling
#include <signal.h>
void signalWrapper(int sig_num);

/*****************************************************************/
/*
 Splash Window Related Stuff
*/
#include "ut_Rehydrate.h"
#include <Bitmap.h>
#include <DataIO.h>
#include <Screen.h>
#include <TranslationUtils.h>
#include <View.h>
#include <Window.h>

//knorr!!
#define SPLASH_UP_TIME	5				// seconds
//knorr!!

extern unsigned char g_pngSplash[];             // see ap_wp_Splash.cpp
extern unsigned long g_pngSplash_sizeof;        // see ap_wp_Splash.cpp


class SplashWin:public BWindow 
{
public:
	SplashWin();
	virtual void DispatchMessage(BMessage *msg, BHandler* hnd);
private:
	int ignore;
};


SplashWin::SplashWin()
	: BWindow(BRect(0, 0, 500.0, 300.0), "AbiWord Splash",
		B_BORDERED_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_WILL_ACCEPT_FIRST_CLICK)
{
	BScreen* screen;
	screen = new BScreen(B_MAIN_SCREEN_ID);
	
	BView *view = new BView(BRect(0, 0, 500.0, 300.0 ), "splashView",
		B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED);
	AddChild(view);
	if (view) 
	{
		BMemoryIO memio(g_pngSplash, g_pngSplash_sizeof);
		BBitmap *bitmap = BTranslationUtils::GetBitmap(&memio);
		if (bitmap) {
			view->SetViewBitmap(bitmap, B_FOLLOW_ALL, 0);
			ResizeTo(bitmap->Bounds().Width(), bitmap->Bounds().Height());
		}
		else
			UT_DEBUGMSG(("Could not interpret splash image...\n"));
    }
	
	CenterOnScreen();
	Show();
	ignore = SPLASH_UP_TIME;			// keep on screen n seconds
	SetPulseRate(1000000);				// a 1 second pulse
}


void SplashWin::DispatchMessage(BMessage *msg, BHandler* hnd) {
	switch (msg->what) {
	case B_PULSE:
		if (ignore-- > 0)
			break;
	case B_MOUSE_DOWN:
		BWindow::DispatchMessage(msg, hnd);
		this->Close();
		break;
	default:
		BWindow::DispatchMessage(msg, hnd);
	}
} 


void _showSplash(XAP_Args * pArgs, const char * /*szAppName*/) {
	// Unix does put the program name in argv[0], 
	// unlike Win32, so [1] is the first argument
	int nFirstArg = 1;
	int k;
	bool bShowSplash = true;

	// scan args for splash-related stuff
#if CONVERT
	for (k=nFirstArg; (k<pArgs->m_argc); k++) {
		if (*pArgs->m_argv[k] == '-') {
			if (UT_stricmp(pArgs->m_argv[k],"-to") == 0) {
				bShowSplash = false;
			}
		}
	}	

	for (k=nFirstArg; (k<pArgs->m_argc); k++) {
		if (*pArgs->m_argv[k] == '-') {
			if (UT_stricmp(pArgs->m_argv[k],"-show") == 0) {
 				bShowSplash = true;
			}
		}
	}	
#endif

	for (k=nFirstArg; (k<pArgs->m_argc); k++) {
		if (*pArgs->m_argv[k] == '-') {
			if (UT_stricmp(pArgs->m_argv[k],"-nosplash") == 0)
				bShowSplash = false;
		}
	}

	if (!bShowSplash)
		return;

   SplashWin *nwin = new SplashWin();
}

/*****************************************************************/

AP_BeOSApp::AP_BeOSApp(XAP_Args * pArgs, const char * szAppName)
	: AP_App(pArgs,szAppName)
{
	m_pStringSet = NULL;
	m_pClipboard = NULL;
}

AP_BeOSApp::~AP_BeOSApp(void)
{

	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);
	
	IE_ImpExp_UnRegisterXP ();
}

static bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct stat statbuf;
	
	if (stat(szDir,&statbuf) == 0)								// if it exists
	{
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
	}
	
	if (mkdir(szDir,0700) == 0)
		return true;
	

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return false;
}	

bool AP_BeOSApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);

	// load the preferences.
	
	m_prefs = new AP_BeOSPrefs(this);
	m_prefs->fullInit();

		   
    //////////////////////////////////////////////////////////////////
    // load the dialog and message box strings
    //
    // (we want to do this as soon as possible so that any errors in
    // the initialization could be properly localized before being
    // reported to the user)
    //////////////////////////////////////////////////////////////////

    {
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
	
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,static_cast<const XML_Char*>(AP_PREF_DEFAULT_StringSet));
		UT_ASSERT(pBuiltinStringSet);
		m_pStringSet = pBuiltinStringSet;
		// see if we should load an alternative set from the disk
	
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;
	
		if (   (getPrefsValue(AP_PREF_KEY_StringSet,
							  static_cast<const XML_Char**>(&szStringSet)))
			   && (szStringSet)
			   && (*szStringSet)
			   && (strcmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(true,
					       static_cast<const XML_Char*>(AP_PREF_KEY_StringSetDirectory),
					       static_cast<const XML_Char**>(&szDirectory));
			UT_ASSERT((szDirectory) && (*szDirectory));

			UT_String szPathname = szDirectory;
			if (szDirectory[szPathname.size()-1]!='/')
				szPathname += "/";
			szPathname += szStringSet;
			szPathname += ".strings";
		
			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_ASSERT(pDiskStringSet);
		
			if (pDiskStringSet->loadStringsFromDisk(szPathname.c_str()))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname.c_str()));
			}
			else
			{
				DELETEP(pDiskStringSet);
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname.c_str()));
			}
		}
    }
	// now that preferences are established, let the xap init

	m_pClipboard = new AP_BeOSClipboard();
	UT_ASSERT(m_pClipboard);

	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);

	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);
		   
	if (!AP_App::initialize())
		return false;
printf("Initilize\n:");	
	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP ();

	printf("IE regsr xp\n");

//#if 0
	/* if'ed out like on other platforms... don't know exactly why */
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
//	{
		const char * szISpellDirectory = NULL;
		getPrefsValueDirectory(false,AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));

		const char * szSpellCheckWordList = NULL;
		//getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList);
		getPrefsValue(AP_PREF_KEY_SpellDirectory,&szSpellCheckWordList);
		UT_ASSERT((szSpellCheckWordList) && (*szSpellCheckWordList));
		
		char * szPathname = (char *)UT_calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
		UT_ASSERT(szPathname);
		
		sprintf(szPathname,"%s%s%s",
				szISpellDirectory,
				((szISpellDirectory[strlen(szISpellDirectory)-1]=='/') ? "" : "/"),
				szSpellCheckWordList);

		UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
		//SpellCheckInit(szPathname);
		free(szPathname);
		
		// we silently go on if we cannot load it....
//	}
//#endif
	
	// Now we have the strings loaded we can populate the field names correctly
	int i;
	
	for (i = 0; fp_FieldTypes[i].m_Type != FPFIELDTYPE_END; i++)
	{
	    (&fp_FieldTypes[i])->m_Desc = m_pStringSet->getValue(fp_FieldTypes[i].m_DescId);
	    UT_DEBUGMSG(("Setting field type desc for type %d, desc=%s\n", fp_FieldTypes[i].m_Type, fp_FieldTypes[i].m_Desc));
	}

	for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++)
	{
	    (&fp_FieldFmts[i])->m_Desc = m_pStringSet->getValue(fp_FieldFmts[i].m_DescId);
	    UT_DEBUGMSG(("Setting field desc for field %s, desc=%s\n", fp_FieldFmts[i].m_Tag, fp_FieldFmts[i].m_Desc));
	}

	//////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if (getPrefsValue( AP_PREF_KEY_StringSet, static_cast<const XML_Char**>(&szMenuLabelSetName))
		&& (szMenuLabelSetName) && (*szMenuLabelSetName))
	{
		;
	}
	else
		szMenuLabelSetName = AP_PREF_DEFAULT_StringSet;

	getMenuFactory()->buildMenuLabelSet(szMenuLabelSetName);

	//////////////////////////////////////////////////////////////////

	return true;
}

XAP_Frame * AP_BeOSApp::newFrame(void)
{
	AP_BeOSFrame * pBeOSFrame = new AP_BeOSFrame(this);

	if (pBeOSFrame)
		pBeOSFrame->initialize();

	//pBeOSFrame->loadDocument("Docs" , IEFT_Unknown);

	return pBeOSFrame;
}

bool AP_BeOSApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return true;
}

bool AP_BeOSApp::getPrefsValueDirectory(bool bAppSpecific,
										   const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return false;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

	if (*psz == '/')
	{
		*pszValue = psz;
		return true;
	}

	const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());
	UT_ASSERT((dir && *dir));
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",dir,psz);
	*pszValue = buf;
	return true;
}

const char * AP_BeOSApp::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

	sprintf(buf,"%s/%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
	return buf;
}

const XAP_StringSet * AP_BeOSApp::getStringSet(void) const
{
	return m_pStringSet;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_BeOSApp::copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard)
{
	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.

	m_pClipboard->clearClipboard();
	
	// also put RTF on the clipboard
	
	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	if (pExpRtf)
	{
		UT_ByteBuf buf;
		UT_Error status = pExpRtf->copyToBuffer(pDocRange,&buf);
		UT_Byte b = 0;
		buf.append(&b,1);			// null terminate string
		m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)buf.getPointer(0),buf.getLength());
		DELETEP(pExpRtf);
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.\n",buf.getLength()));
	}

	// put raw 8bit text on the clipboard
		
	IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
	if (pExpText)
	{
		UT_ByteBuf buf;
		UT_Error status = pExpText->copyToBuffer(pDocRange,&buf);
		UT_Byte b = 0;
		buf.append(&b,1);			// null terminate string
		m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)buf.getPointer(0),buf.getLength());
		DELETEP(pExpText);
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",buf.getLength()));
	}
}

void AP_BeOSApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool, bool bHonorFormatting)
{
	// paste from the system clipboard using the best-for-us format
	// that is present.
	// TODO handle bHonorFormatting
	
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
	{
		unsigned char * pData = NULL;
		UT_uint32 iLen = 0;
		bool bResult = m_pClipboard->getClipboardData(AP_CLIPBOARD_RTF,(void**)&pData,&iLen);
		UT_ASSERT(bResult);
		iLen = UT_MIN(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in RTF format.\n",iLen));
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
		goto MyEnd;
	}

	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
	{
		unsigned char * pData = NULL;
		UT_uint32 iLen = 0;
		bool bResult = m_pClipboard->getClipboardData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(void**)&pData,&iLen);
		UT_ASSERT(bResult);
		iLen = UT_MIN(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in TEXTPLAIN format.\n",iLen));
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
		goto MyEnd;
	}

	// TODO figure out what to do with an image....
	UT_DEBUGMSG(("PasteFromClipboard: TODO support this format..."));

MyEnd:
	return;
}

bool AP_BeOSApp::canPasteFromClipboard(void)
{
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		goto ReturnTrue;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		goto ReturnTrue;

	return false;

ReturnTrue:
	return true;
}

/*****************************************************************/

int AP_BeOSApp::main(const char * szAppName, int argc, const char ** argv) {
	// This is a static function.
#if CONVERT
	bool bShowApp = true;
#endif
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
	
	UT_DEBUGMSG((" initialize our application.\n"));
	XAP_Args XArgs = XAP_Args(argc,argv);
	AP_BeOSApp * pMyBeOSApp = new AP_BeOSApp(&XArgs, szAppName);	
	AP_Args Args = AP_Args(&XArgs, szAppName, pMyBeOSApp);

#if 0 //even more, no need for this in alpha-release
//#ifndef DEBUG
	UT_DEBUGMSG((" Setup signal handlers, primarily for segfault\n"));
	// If we segfaulted before here, we *really* blew it
	struct sigaction sa;

	sa.sa_handler = signalWrapper;

	sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
	sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that

	sa.sa_flags = 0; // Unsupported under BeOS -- SA_NODEFER | SA_RESETHAND; // Don't handle nested signals
	
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	UT_DEBUGMSG((" TODO: handle SIGABRT\n"));
#endif	
	Args.parsePoptOpts();
	UT_DEBUGMSG((" if the initialize fails, we don't have icons, fonts, etc.\n"));
	if (!pMyBeOSApp->initialize())
	{
		delete pMyBeOSApp;
		return -1;	// make this something standard?
	}

	UT_DEBUGMSG((" Show the splash screen perhaps\n"));
//knorr!! to save time :?)
  	_showSplash(&XArgs, szAppName);        
	UT_DEBUGMSG((" knorr!!\n"));
	if (pMyBeOSApp->openCmdLineFiles(&Args))
	{
		UT_DEBUGMSG((" Turn control over to the runtime (don't return until done)\n"));
		pMyBeOSApp->m_BApp.Run();	
	}

	UT_DEBUGMSG((" destroy the App.  It should take care of deleting all frames.\n"));
	pMyBeOSApp->shutdown();
	sleep(1);

	delete pMyBeOSApp;
	return 0;
}


void signalWrapper(int sig_num)
{
	AP_BeOSApp *pApp = (AP_BeOSApp *) XAP_App::getApp();
#if 0 //see above
//#ifndef DEBUG
	pApp->catchSignals(sig_num);
#endif
}




static int s_signal_count = 0;

void AP_BeOSApp::catchSignals(int sig_num)
{
	// Reset the signal handler (not that it matters - this is mostly for race conditions)
	signal(SIGSEGV, signalWrapper);

	s_signal_count = s_signal_count + 1;
	if(s_signal_count > 1)
	{
		UT_DEBUGMSG(("Segfault during filesave - no file saved  \n"));
		fflush(stdout);
		abort();
	}


	printf("Oh no - we just segfaulted!\n");
//	UT_DEBUGMSG(("Oh no - we just segfaulted!\n"));

	UT_uint32 i = 0;
	for(;i<m_vecFrames.getItemCount();i++)
	{
		AP_BeOSFrame * curFrame = (AP_BeOSFrame*) m_vecFrames[i];
		UT_ASSERT(curFrame);
		curFrame->backup();
	}
	
	fflush(stdout);
	
	// Abort and dump core
	abort();
}

