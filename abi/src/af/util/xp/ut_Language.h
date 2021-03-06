/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
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

#ifndef UT_LANGUAGE_H
#define UT_LANGUAGE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

enum UT_LANGUAGE_DIR
{
	UTLANG_LTR,
	UTLANG_RTL,
	UTLANG_VERTICAL
};

struct UT_LangRecord
{
	XML_Char * m_szLangCode;
	XML_Char * m_szLangName;
	UT_uint32  m_nID;
    UT_LANGUAGE_DIR m_eDir;
};


class ABI_EXPORT UT_Language
{
public:
	UT_Language();

	UT_uint32	getCount();
	const XML_Char * 	getNthLangCode(UT_uint32 n);
	const XML_Char * 	getNthLangName(UT_uint32 n);
	const UT_uint32  	getNthId(UT_uint32 n);
	const XML_Char * 	getCodeFromName(const XML_Char * szName);
	const XML_Char * 	getCodeFromCode(const XML_Char * szCode); //
																  //see the cpp file for explanation
	const UT_LangRecord* getLangRecordFromCode(const XML_Char * szCode);
	UT_uint32 	        getIndxFromCode(const XML_Char * szCode);
	UT_uint32 	        getIdFromCode(const XML_Char * szCode);
	UT_LANGUAGE_DIR		getDirFromCode(const XML_Char * szCode);

private:
	static bool	s_Init;
};

ABI_EXPORT void UT_Language_updateLanguageNames(); 
#endif
