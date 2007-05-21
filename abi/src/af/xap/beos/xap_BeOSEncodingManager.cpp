#include "xap_BeOSEncodingManager.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"

static UT_UTF8String NativeEncodingName;
static UT_UTF8String NativeSystemEncodingName;
static UT_UTF8String Native8BitEncodingName;
static UT_UTF8String NativeNonUnicodeEncodingName;
static UT_UTF8String NativeUnicodeEncodingName;
static UT_UTF8String LanguageISOName;
static UT_UTF8String LanguageISOTerritory;

XAP_EncodingManager *XAP_EncodingManager::get_instance()
{
	if (_instance == 0) 
	{
		UT_DEBUGMSG(("Building XAP_EncodingManager\n"));	
		_instance = new XAP_BeOSEncodingManager();
		_instance->initialize();
		UT_DEBUGMSG(("XAP_EncodingManager built\n"));		
	}

	return _instance;
}

XAP_BeOSEncodingManager::XAP_BeOSEncodingManager()
{
}

XAP_BeOSEncodingManager::~XAP_BeOSEncodingManager() 
{
}

void XAP_BeOSEncodingManager::initialize()
{
	NativeEncodingName =  "UTF-8";
	NativeSystemEncodingName =
	Native8BitEncodingName =
	NativeNonUnicodeEncodingName = NativeEncodingName;
	NativeUnicodeEncodingName = "UTF-8";
	LanguageISOName = "en";
	LanguageISOTerritory = "US";   
}

const char* XAP_BeOSEncodingManager::getNativeEncodingName() const
{     
  return NativeEncodingName.utf8_str(); 
}

const char* XAP_BeOSEncodingManager::getNativeSystemEncodingName() const
{     
  return NativeSystemEncodingName.utf8_str(); 
}

const char* XAP_BeOSEncodingManager::getNative8BitEncodingName() const
{     
  return Native8BitEncodingName.utf8_str();
}

const char* XAP_BeOSEncodingManager::getNativeNonUnicodeEncodingName() const
{     
  return NativeNonUnicodeEncodingName.utf8_str();
}

const char* XAP_BeOSEncodingManager::getNativeUnicodeEncodingName() const
{     
  return NativeUnicodeEncodingName.utf8_str(); 
}

const char* XAP_BeOSEncodingManager::getLanguageISOName() const
{
 	return LanguageISOName.utf8_str(); 
}

const char* XAP_BeOSEncodingManager::getLanguageISOTerritory() const
{ 	
  return LanguageISOTerritory.utf8_str(); 
}

/*!
 * Returns the name this system uses for UCS-2, big endian
 *
 * UCS-2BE is standard but some older iconvs use UCS-2-BE
 */
const char* XAP_BeOSEncodingManager::getUCS2BEName() const
{
	return "UCS-2BE";
}

/*!
 * Returns the name this system uses for UCS-2, little endian
 *
 * UCS-2LE is standard but some older iconvs use UCS-2-LE
 */
const char* XAP_BeOSEncodingManager::getUCS2LEName() const
{
	return "UCS-2LE";
}

/*!
 * Returns the name this system uses for UCS-4, big endian
 *
 * UCS-4BE is standard
 */
const char* XAP_BeOSEncodingManager::getUCS4BEName() const
{
	return "UCS-4BE";
}

/*!
 * Returns the name this system uses for UCS-4, little endian
 *
 * UCS-4LE is standard
 */
const char* XAP_BeOSEncodingManager::getUCS4LEName() const
{
	return "UCS-4LE";
}
