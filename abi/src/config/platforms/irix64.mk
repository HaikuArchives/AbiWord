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

##############################################################################
## SGI IRIX 64 platform defines, courtesy of Philippe Defert 
## (Philippe.Defert@cern.ch)
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:= $(shell uname -m)
OS_ENDIAN	= BigEndian32

# Define tools
CC		= gcc
#CC		= cc
CCC		= g++
#CCC		= CC
#RANLIB		= ranlib
RANLIB		= true

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

# Compiler flags
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g -Wall # -ansi -pedantic
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
OPTIMIZER	= -O2 -Wall # -ansi -pedantic
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
OS_INCLUDES		=
G++INCLUDES		= -I/usr/include/g++

# Compiler flags
PLATFORM_FLAGS		= 
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -DPATH_MAX=1024 -DSCANDIR_MISSING -DSETENV_MISSING
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+=
PORT_FLAGS		+= 

GLIB_CONFIG		= glib-config
GTK_CONFIG		= gtk-config
GNOME_CONFIG    	= gnome-config

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Which links can this platform create.  Define one or
# both of these options. (what can IRIX do?  someone
# with an SGI set the right one and mail a patch)
UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=0

# Compiler options for static and dynamic linkage
DL_LIBS			= dl
STATIC_FLAGS		= -static

ABI_NATIVE	= unix
ABI_FE		= Unix

##################################################################
## Here you can choice if you want to use the gnome stuff.
## Set ABI_OPT_GNOME to 1 (when invoking 'make') or set as
## an environment variable.
##
## ABI_OPT_GNOME=1

##################################################################              
## ABIPKGDIR defines the directory containing the Makefile to use to            
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)           
## This is relative to $(ABI_ROOT)/src/pkg                                      

ABIPKGDIR       = irix
PSICONV_PLATFORM_DEFS= CFLAGS='-O2'

EXTRA_LIBS += -liconv

# End of IRIX 64 defs
