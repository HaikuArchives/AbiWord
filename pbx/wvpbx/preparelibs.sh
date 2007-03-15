#!/bin/sh
#/* AbiWord build scripts
# * Copyright (c) 2004 Hubert Figuiere
# * 
# * This program is free software; you can redistribute it and/or
# * modify it under the terms of the GNU General Public License
# * as published by the Free Software Foundation; either version 2
# * of the License, or (at your option) any later version.
# * 
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# * 
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
# * 02111-1307, USA.
# */


# Fink root setup dir. Not supposed to change as it is 
# hardcoded into the libraries
FINK_LIB_DIR=/sw/lib

# this is the DEPOT dir, the directory where to find 
# libs to copy from. By default it is the Fink lib dir
if [ x$FINK_DEPOT_DIR = x ] 
then
	FINK_DEPOT_DIR=$FINK_LIB_DIR
fi

EMBED_LIB_DIR=@executable_path/../Frameworks
# TODO: extract dependencies from, in that order
#  -glib
#  -intl
#  -iconv
#  -xml2
DYLIBS="libglib-2.0.dylib libiconv.2.dylib libintl.1.dylib libxml2.2.dylib"


for i in $DYLIBS ; do
    if [ ! -f $FINK_LIB_DIR/$i ] ; then
	echo "Can't find $FINK_LIB_DIR/$i in Fink. Build failed."
	exit 1
    fi
    if [ $FINK_LIB_DIR/$i -nt $SYMROOT/$i ]; then
	echo "Copying $FINK_DEPOT_DIR/$i to $SYMROOT"
	# copy following the link (ie copy the link target) 
	# and force override if needed (bug 6317)
	cp -Lf $FINK_DEPOT_DIR/$i $SYMROOT
	echo "Setting install name."
	install_name_tool \
		-id $EMBED_LIB_DIR/$i $SYMROOT/$i

    fi
done


