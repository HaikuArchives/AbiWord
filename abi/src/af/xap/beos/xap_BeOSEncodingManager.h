#ifndef XAP_BEOSENCMGR_H
#define XAP_BEOSENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_BeOSEncodingManager: public XAP_EncodingManager
{
protected:
	XAP_BeOSEncodingManager();
virtual ~XAP_BeOSEncodingManager();    

	void 	initialize();    

public:
	const char* getNativeEncodingName() 	const;
	const char* getNativeSystemEncodingName() 	const;
	const char* getNative8BitEncodingName()	const;
	const char* getNativeNonUnicodeEncodingName() 	const;
	const char* getNativeUnicodeEncodingName() 	const;
	const char* getLanguageISOName() 		const;
	const char* getLanguageISOTerritory() 	const;

	virtual const char* getUCS2BEName() const;
	virtual const char* getUCS2LEName() const;
	virtual const char* getUCS4BEName() const;
	virtual const char* getUCS4LEName() const;

friend class XAP_EncodingManager;
};

#endif /* XAP_BEOSENCMGR_H */
