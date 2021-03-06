#! gmake

## AbiWord
## Copyright (C) 1998 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
## 02111-1307, USA.

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

ifeq ($(ABI_OPT_PEER_EXPAT),1)
REQUIRE=	_EXPAT_ 
endif
#REQUIRE+=	_PSICONV_
TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

#################################################################

_EXPAT_:
#use pregenerated Makefile on Windows, else configure generated one
ifeq ($(ABI_FE),Win32)
	@if [ -d $(ABI_ROOT)/../expat ]; then				\
		echo "Found expat in peer directory";			\
		cd $(ABI_ROOT)/../expat; 			\
		$(ABICOPY) expat_config.h.msvc expat_config.h;	\
		$(ABICOPY) lib/winconfig.h lib/config.h;	\
		$(MAKE) -fMakefile.abi ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../expat;	\
	else										\
		echo "ERROR -- peer expat not found";			\
		exit 1;								\
	fi
else
	@if [ -d $(ABI_ROOT)/../expat ]; then				\
		echo "Found expat in peer directory";			\
		if [ ! -r $(ABI_ROOT)/../expat/Makefile ]; then 	\
			cd $(ABI_ROOT)/../expat; 			\
			$(EXPAT_PLATFORM_DEFS) ./configure --disable-shared --enable-static; 	\
			$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../expat;	\
		fi; 							\
	elif [ -f /usr/include/expat.h -o -f /usr/local/include/expat.h ]; then 	\
		echo "*** Found a shared library version of expat.  I'm going to use "; \
		echo "*** that.  If it doesn't work, you should use the version "; 	\
		echo "*** distributed with AbiWord."; 					\
		echo ; 									\
	else										\
		echo "ERROR -- expat not found";					\
		echo "You need to get expat from the AbiSource downloads site";		\
		echo "It is identical to expat from expat.sourceforge.net, except for";	\
		echo "minor modifications to integrate with the AbiSource build system.";	\
		exit 1;									\
	fi
endif

_PSICONV_:
	@if [ -r $(ABI_ROOT)/../psiconv ]; then 			\
		echo "Found psiconv in peer directory"; 		\
		if [ ! -r $(ABI_ROOT)/../psiconv/Makefile ]; then 	\
			cd $(ABI_ROOT)/../psiconv; 			\
			$(PSICONV_PLATFORM_DEFS) ./configure --disable-shared --enable-static; 	\
		fi; 							\
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../psiconv; 		\
	elif [ -d /usr/include/psiconv -o -d /usr/local/include/psiconv ]; then 			\
		echo "*** Found an install of psiconv. I'm going to use it; if it is too old,"; 	\
		echo "*** you will have to upgrade it or get the version distributed with AbiWord."; 	\
		echo "*** You can find psiconv at http://huizen.dds.nl/~frodol/psiconv/"; 		\
		echo ; 											\
	else 												\
		echo "Psiconv not found. Get the version distributed with AbiWord and unpack it" ;	\
		echo "in a peer directory; or get it from http://huizen.dds.nl/~frodol/psiconv and" ;	\
		echo "install it separately." ;								\
		exit 1;											\
	fi
