#! gmake

## AbiSource Program Utilities
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

CPPSRCS=		abi_ver.cpp

LIBRARY_NAME=		AbiWordVer
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

ifdef ABI_BUILD_ID
BUILD_ID=		$(ABI_BUILD_ID)
else
BUILD_ID=		
endif

# This is always present now, so no more checks.
BUILD_VERSION=		$(ABI_BUILD_VERSION)

ifdef ABI_OPTIONS
ifeq ($(ABI_ESCAPE_QUOTES),YES)
BUILD_OPTIONS=		\"$(ABI_OPTIONS)\"
else
BUILD_OPTIONS=		"$(ABI_OPTIONS)"
endif
else
BUILD_OPTIONS=
endif

ABI_VERFLAGS=		-DABI_BUILD_ID=\"$(BUILD_ID)\"			\
			-DABI_BUILD_VERSION=\"$(BUILD_VERSION)\"	\
			-DABI_BUILD_OPTIONS=\"$(BUILD_OPTIONS)\"	\
			-DABI_BUILD_TARGET=\"$(OBJDIR)\"

INCLUDES=		$(ABI_VERFLAGS) -DABISUITE_HOME="\"$(pkgdatadir)\"" \

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
