/* AbiSource Program Utilities
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2004 Hubert Figuiere
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


#include <string.h>

#include "ut_string.h"
#include "ut_PropVector.h"




void UT_PropVector::addOrReplaceProp(const XML_Char * pszProp, const XML_Char * pszVal)
{
	UT_ASSERT(pszVal);
	
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
/*	if(iCount <= 0)
	{
		char * prop = UT_strdup(pszProp);
		char * val = UT_strdup(pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
		return;
	}*/
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2) {
		pszV = reinterpret_cast<const XML_Char *>(getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0)) {
			break;
		}
	}
	if((iCount > 0) && (i < iCount)) {
	    XML_Char* pVal;
		XML_Char * val = UT_strdup(pszVal);
		setNthItem(i+1, val, &pVal);
		FREEP(pVal);
	}
	else {
		XML_Char * prop = UT_strdup(pszProp);
		XML_Char * val = UT_strdup(pszVal);
		addItem(prop);
		addItem(val);
	}
	return;
}


void UT_PropVector::getProp(const XML_Char * pszProp,
									   const XML_Char * &pszVal)
{
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		pszVal = getNthItem(i+1);
	}
	return;
}

/*!
 Removes the key,value pair  (pszProp,pszVal) given by pszProp
 from the Vector of all properties of the current format.
 If the Property does not exists nothing happens
 \param UT_Vector &vec the vector to remove the pair from
 \param const XML_Char * pszProp the property name
*/
void UT_PropVector::removeProp(const XML_Char * pszProp)
{
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0)) {
			break;
		}
	}
	if(i < iCount)
	{
		XML_Char * pSP = getNthItem(i);
		XML_Char * pSV = getNthItem(i+1);
		FREEP(pSP);
		FREEP(pSV);
		deleteNthItem(i+1);
		deleteNthItem(i);
	}
	return;
}

