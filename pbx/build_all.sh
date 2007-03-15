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

CONFIGFILE=.config
BUILD_CMD=/usr/bin/pbxbuild
PBXSUFFIX=

if [ ! -x $BUILD_CMD ]
then
	echo "pbxbuild not found"
	if [ -x /usr/bin/xcodebuild ]
	then
		echo "xcodebuild found instead"
		BUILD_CMD=/usr/bin/xcodebuild
		PBXSUFFIX=2
	else
		echo "*************************"
		echo "* No devtools found !!! *"
		echo "*************************"
		exit 2
	fi
fi

# configuration
if [ ! -f $CONFIGFILE ]; then
    echo "Build directory [`pwd`/BUILD]:"
    read pbxbuilddir
    if [ -z "$pbxbuilddir" ]; then
	echo "Using default."
	pbxbuilddir=`pwd`/BUILD
    fi
    echo -n "SYMROOT=" >> $CONFIGFILE
    echo "$pbxbuilddir" >> $CONFIGFILE
    echo "export SYMROOT" >> $CONFIGFILE
    
fi

. $CONFIGFILE

abi_create_disc_image=yes
abi_build_dependencies=yes

for i in $@; do
    if test "x$i" = "x--no-disc-image"; then
	abi_create_disc_image=no
    fi
    if test "x$i" = "x--no-dependencies"; then
	abi_build_dependencies=no
    fi
done

if test "$abi_build_dependencies" = "yes"; then

    (cd poptpbx; $BUILD_CMD SYMROOT="$SYMROOT" -project poptpbx$PBXSUFFIX.pbproj -target "poptpbx" -buildstyle "Deployment - Embedded" build)
    if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
    fi

    (cd fribidipbx; $BUILD_CMD SYMROOT="$SYMROOT" -project fribidipbx$PBXSUFFIX.pbproj -target "fribidipbx" -buildstyle "Deployment" build)
    if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
    fi

    (cd pngpbx; $BUILD_CMD SYMROOT="$SYMROOT" -project pngpbx$PBXSUFFIX.pbproj -target "pngpbx" -buildstyle "Deployment" build)
    if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
    fi

    (cd wvpbx; $BUILD_CMD SYMROOT="$SYMROOT" -project wvpbx$PBXSUFFIX.pbproj -target "wv" -buildstyle "Deployment - Embedded" build)
    if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
    fi

    (cd enchantpbx \
     && $BUILD_CMD SYMROOT="$SYMROOT" -project enchantpbx.xcode -target "AppleSpell"        build \
     && $BUILD_CMD SYMROOT="$SYMROOT" -project enchantpbx.xcode -target "ISpell"            build \
     && $BUILD_CMD SYMROOT="$SYMROOT" -project enchantpbx.xcode -target "enchant framework" build)
    if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
    fi

fi

(cd abipbx; $BUILD_CMD SYMROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target Complete -buildstyle "Deployment" build)
if [ $? -ne 0 ] ; then
	echo "********************"
	echo "*** BUILD FAILED ***"
	echo "********************"
	exit 3
fi

for i in $@; do
    if test "$i" = "all" -o "$i" = "urldict"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiURLDict -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiURLDict -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "wikipedia"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiWikipedia -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiWikipedia -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "google"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiGoogle -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiGoogle -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "latex"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiLaTeX -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiLaTeX -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "xhtml"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiXHTML -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiXHTML -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "bz2abw"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiBZ2 -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiBZ2 -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "OpenWriter"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiOpenWriter -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiOpenWriter -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "sdw"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiSDW -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiSDW -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
    if test "$i" = "all" -o "$i" = "wordperfect"; then
	(cd abipbx \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiWordPerfect -buildstyle "Deployment" build \
	 && $BUILD_CMD SYMROOT="$SYMROOT" DSTROOT="$SYMROOT" -project abipbx$PBXSUFFIX.pbproj -target AbiWordPerfect -buildstyle "Deployment" install)
	if [ $? -ne 0 ] ; then
	    echo "********************"
	    echo "*** BUILD FAILED ***"
	    echo "********************"
	    exit 3
	fi
    fi
done

echo "Copying AbiWord system files to bundle resources directory:"
(cd abi/user/wp; tar cf $SYMROOT/tmp-appfiles.tar strings system.profile*; cd $SYMROOT/AbiWord.app/Contents/Resources; /bin/rm -rf AbiWord; mkdir AbiWord; cd AbiWord; tar xf $SYMROOT/tmp-appfiles.tar; rm $SYMROOT/tmp-appfiles.tar)

echo "Copying AbiWord barbarism files to bundle resources directory:"
(/bin/rm -rf $SYMROOT/AbiWord.app/Contents/Resources/dictionary; mkdir $SYMROOT/AbiWord.app/Contents/Resources/dictionary; cd abidistfiles/dictionary; cp *barbarism.xml $SYMROOT/AbiWord.app/Contents/Resources/dictionary)

echo "Copying AbiWord template files to bundle resources directory:"
(cd abidistfiles/templates; tar cf $SYMROOT/tmp-templates.tar *awt*; cd $SYMROOT/AbiWord.app/Contents/Resources; /bin/rm -rf templates; mkdir templates; cd templates; tar xf $SYMROOT/tmp-templates.tar; rm $SYMROOT/tmp-templates.tar)

rm -f $SYMROOT/AbiWord.dmg

if test "$abi_create_disc_image" = "no"; then
    exit
fi

hdiutil create -megabytes 30 -fs HFS+ -volname AbiWord $SYMROOT/AbiWord.dmg
device_name=`hdiutil attach $SYMROOT/AbiWord.dmg | grep "/Volumes/" | cut -f 1 -d " "`
if test -z $device_name; then
	echo "error: unable to mount $SYMROOT/AbiWord.dmg"
else
	echo "$SYMROOT/AbiWord.dmg mounted as $device_name"
	cp -RP $SYMROOT/AbiWord.app /Volumes/AbiWord/
	cp COPYING /Volumes/AbiWord/COPYING.txt
#	cp README-MacOSX.pdf /Volumes/AbiWord/
	mkdir /Volumes/AbiWord/InvisibleAnt
	mkdir /Volumes/AbiWord/InvisibleAnt/images
	/Developer/Tools/SetFile -a V /Volumes/AbiWord/InvisibleAnt
	cp abipbx/AbiWord-DMG-ReadMe/README.html /Volumes/AbiWord/
	cp abipbx/AbiWord-DMG-ReadMe/InvisibleAnt/*.* /Volumes/AbiWord/InvisibleAnt/
	cp abipbx/AbiWord-DMG-ReadMe/InvisibleAnt/images/*.* /Volumes/AbiWord/InvisibleAnt/images/
	cp abipbx/AbiWord-DMG-Background.png /Volumes/AbiWord/InvisibleAnt/
	cp abipbx/AbiWord-DMG-DS_Store /Volumes/AbiWord/.DS_Store
	hdiutil detach $device_name

	echo ""
	echo "(1) Use the Finder's View Options to set the image background."
	echo "(2) Then: hdiutil convert AbiWord-2.2.2.dmg -format UDZO -o AbiWord-2.2.2-UDZO9.dmg -imagekey zlib-level=9"
	echo ""
fi
