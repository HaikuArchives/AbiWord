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

CPPSRCS=	gr_$(ABI_FE)Graphics.cpp	\
			gr_$(ABI_FE)Image.cpp		\
		be_BackView.cpp

#TF Change to
#CPPSRCS= $(wildcard *.cpp)

ABI_INCS += -I$(ABI_ROOT)/../libpng -I$(ABI_ROOT)/../zlib

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
