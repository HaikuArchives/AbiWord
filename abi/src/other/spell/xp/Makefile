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

# TODO we should move this down into src/other/spell/xp

ABI_ROOT:=$(shell pwd)/../../../..
ABI_PEER:=/spell

include $(ABI_ROOT)/src/config/abi_defs.mk

#dummy initialization
CSRCS=
CPPSRCS= spell_manager.cpp barbarisms.cpp

ifeq ($(ABI_OPT_ENCHANT),1)
	CPPSRCS+= enchant_checker.cpp
else
	CPPSRCS+= ispell_checker.cpp good.cpp tgood.cpp lookup.cpp makedent.cpp hash.cpp \
correct.cpp 
endif

LIBRARY_NAME=		AbiSpell
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
