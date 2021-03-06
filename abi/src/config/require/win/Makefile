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

REQUIRE=	_ZLIB_ _LIBPNG_ _LIBICONV_ _LIBWV_ _NSIS_
REQUIREREALCLEAN=	realclean_LIBICONV_ realclean_LIBWV_
TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

realclean::		$(REQUIREREALCLEAN)


#################################################################

_ZLIB_:
	@if [ -d $(ABI_ROOT)/../zlib ]; then					\
		echo Found zlib in peer directory;				\
		$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../zlib;		\
	else									\
		echo zlib not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then					\
		echo Found libpng in peer directory;				\
		$(MAKE) -f Makefile.abi ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../libpng;	\
	else									\
		echo libpng not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		echo Found libiconv in peer directory;			\
		if [ ! -r $(ABI_ROOT)/../libiconv/lib/config.h ]; then	\
			cd $(ABI_ROOT)/../libiconv; 			\
			CC='cl.exe -nologo' CONFIG_SHELL=bash.exe bash ./configure; \
		fi;						\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libiconv;	\
	else								\
		echo libiconv not found;				\
		echo TODO provide a nicer error message;		\
		echo TODO with instructions for obtaining it;		\
		exit 1;							\
	fi

_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then			\
		echo Found libwv in peer directory;		\
		if [ ! -r $(ABI_ROOT)/../wv/config.h ]; then	\
			cd $(ABI_ROOT)/../wv; 			\
			CC='cl.exe -nologo' CONFIG_SHELL=bash.exe bash ./configure --with-peer-iconv=abi;	\
		fi;						\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../wv;  	\
	else							\
		echo wv not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_NSIS_:
	@if [ -d $(ABI_ROOT)/../nsis ]; then			\
		echo Found nsis in peer directory;		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../nsis;		\
	else									\
		echo nsis not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

#################################################################

realclean_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		$(MAKE) -C $(ABI_ROOT)/../libiconv distclean;	\
	fi

realclean_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then			\
		$(MAKE) -C $(ABI_ROOT)/../wv realclean;	\
	fi
