#! gmake

## AbiWord
## Copyright (C) 1998,1999 AbiSource, Inc.
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

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		fv_View.cpp			\
			fv_View_cmd.cpp	\
			fv_View_protected.cpp	\
			fv_View_TestRoutines.cpp	\
			fv_InlineImage.cpp	\
			fb_LineBreaker.cpp		\
			fb_Alignment.cpp		\
			fl_ContainerLayout.cpp          \
			fl_BlockLayout.cpp		\
			fl_Squiggles.cpp		\
			fl_DocLayout.cpp		\
			fl_DocListener.cpp		\
			fl_Layout.cpp			\
			fl_SectionLayout.cpp		\
			fl_FootnoteLayout.cpp           \
			fl_TOCLayout.cpp           \
			fl_FrameLayout.cpp           \
			fl_AutoNum.cpp			\
			fl_AutoLists.cpp		\
			fl_SelectionPreserver.cpp	\
			fl_TableLayout.cpp              \
			fl_TestRoutines.cpp		\
			fp_Column.cpp			\
			fp_ContainerObject.cpp		\
			fp_Line.cpp			\
			fp_Page.cpp			\
			fp_PageSize.cpp			\
			fp_EmbedRun.cpp			\
			fp_Run.cpp			\
			fp_MathRun.cpp			\
			fp_FmtMarkRun.cpp		\
			fp_TextRun.cpp			\
			fp_TestRoutines.cpp		\
	                fp_TableContainer.cpp           \
			fp_FootnoteContainer.cpp        \
			fp_TOCContainer.cpp        \
			fp_FrameContainer.cpp        \
			fg_Graphic.cpp			\
			fg_GraphicRaster.cpp		\
			fg_GraphicVector.cpp		\
			fp_FieldListLabelRun.cpp	\
			fp_FieldTableSumRun.cpp	\
			fp_FieldTOCNum.cpp	\
			fp_DirectionMarkerRun.cpp   \
			fb_ColumnBreaker.cpp \
			fv_FrameEdit.cpp \
			fv_VisualDragText.cpp \
			fv_Selection.cpp 

LIBRARY_NAME=		AbiFmt
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)







