dnl AbiWord
dnl Copyright (C) 2001 Sam Tobin-Hochstadt
dnl MacOS X hackery by Hubert Figuiere <hfiguiere@teaser.fr>
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License
dnl as published by the Free Software Foundation; either version 2
dnl of the License, or (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
dnl 02111-1307, USA.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT([AbiSuite],[2.2],[http://www.abisource.com/])
AC_CONFIG_SRCDIR([src/af/ev/xp/ev_Menu.h])
AC_CONFIG_AUX_DIR(ac-helpers)
dnl AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE(AbiSuite-2.2,2.2.8)

if test "$srcdir" = "."; then
	abi_rootdir=".."
else
	abi_rootdir="`dirname $srcdir`"
fi
abs_srcdir="`cd $srcdir && pwd`"

dnl Checks for programs.

abi_save_CFLAGS="$CFLAGS"
abi_save_CXXFLAGS="$CXXFLAGS"

AC_PROG_CXX
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CPP

CFLAGS="$abi_save_CFLAGS"
CXXFLAGS="$abi_save_CXXFLAGS"

AC_MSG_CHECKING(CFLAGS)
AC_MSG_RESULT($CFLAGS)

AC_MSG_CHECKING(CXXFLAGS)
AC_MSG_RESULT($CXXFLAGS)

ABI_FEATURE_DEFS=""

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
dnl AC_PROG_LIBTOOL
AM_ACLOCAL_INCLUDE(ac-helpers)

dnl Detect our platform (unix, win32, etc)
dnl Do this before the GNOME stuff
ABI_DETECT_PLATFORM
ABI_TYPES

dnl Checks for libraries.
dnl Define here ABI_FE, ABI_BE, PLATFORM_CFLAGS, PLATFORM_LIBS
with_unix=false
with_macosx=false
with_cocoa=false
with_qnx=false
if test "$PLATFORM" = "unix"; then
	ABI_FE="Unix"
	ABI_BE="$ABI_FE"
	with_unix=true
	dnl locate glib and gtk
	ABI_GTK2
	AM_BINRELOC
elif test "$PLATFORM" = "cocoa" ; then
	ABI_GLIB2
	ABI_FE="Cocoa"
	ABI_BE="Unix"
	with_macosx=true
	with_cocoa=true

	dnl Don't change these (fjf)
	dnl 
	OBJCXX="c++"
	OBJCXXFLAGS=""
	OBJCXXDEPMODE="depmode=gcc3"
elif test "$PLATFORM" = "qnx" ; then
	ABI_FE="QNX"
	with_qnx=true
	ABI_THREAD

else
	ABI_THREAD
fi
AM_CONDITIONAL(WITH_UNIX,$with_unix)
AM_CONDITIONAL(WITH_MACOSX,$with_macosx)
AM_CONDITIONAL(WITH_COCOA,$with_cocoa)
AM_CONDITIONAL(WITH_QNX,$with_qnx)
AM_CONDITIONAL(WITH_BINRELOC, test "x$br_cv_binreloc" = "xyes")
AC_SUBST(OBJCXX)
AC_SUBST(OBJCXXFLAGS)
AC_SUBST(OBJCXXDEPMODE)

dnl These are the flags required and defined above
AC_SUBST(ABI_FE)
AC_SUBST(ABI_BE)

ABI_DETECT_ICONV

ABIWORD_APP_NAME="AbiWord"
AC_SUBST(ABIWORD_APP_NAME)
ABIWORD_APP_LIBDIR="AbiWord"
AC_SUBST(ABIWORD_APP_LIBDIR)

dnl ./configure command line arguments
dnl currently, we have:
dnl   --enable-gnome
dnl   --enable-debug
dnl   --enable-extra-optimization
dnl   --enable-profile
dnl   --enable-bidi
dnl   --enable-peer-config
dnl   --with-pspell
dnl   --with-libxml2
dnl The defined values are used mostly in includes.mk, but also in
dnl some other places

ABI_GNOME_QUICK
ABI_GUCHARMAP

ABI_XFT_QUICK

abi_build_shared=yes
abi_build_static=no

AC_ARG_ENABLE(shared,[  --disable-shared   don't build shared AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" = "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnome" != "true"; then
			abi_build_shared=no
			abi_build_static=yes
		fi
	fi
])
AC_ARG_ENABLE(static,[  --enable-static          build static AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" != "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnome" != "true"; then
			abi_build_static=yes
		fi
	fi
])

AM_CONDITIONAL(ABI_BUILD_SHARED,[test $abi_build_shared = yes])
AM_CONDITIONAL(ABI_BUILD_STATIC,[test $abi_build_static = yes])

dnl spell-checker detection precedes curl setting
dnl 
AC_LANG_CPLUSPLUS
ABI_SPELL
AC_LANG_C

ABI_FRIBIDI

ABI_PLUGINS

dnl The resource manager is a class (XAP_ResourceManager) designed to handle
dnl images/objects/hyperlinks included by the document. Disabled by default.
dnl 
abi_resource_manager=no
dnl AC_ARG_ENABLE(resource-manager,[  --enable-resource-manager  Use the resource manager (experimental)],[
dnl 	if test "x$enableval" = "xyes"; then
dnl 		abi_resource_manager=yes
dnl 		ABI_FEATURE_DEFS="$ABI_FEATURE_DEFS -DENABLE_RESOURCE_MANAGER=1"
dnl 	fi
dnl ])
AM_CONDITIONAL(WITH_RESOURCE_MANAGER,[test "x$abi_resource_manager" = "xyes"])

AC_ARG_ENABLE(profile,
    [  --enable-profile    Turn on profile ],
     [case "${enableval}" in
       yes) PROFILE_CFLAGS="-pg -fprofile-arcs -ftest-coverage" ;;
       no)  PROFILE_CFLAGS="" ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-profile) ;;
     esac],[PROFILE_CFLAGS=""])

AC_ARG_ENABLE(extra-optimization,
    [  --enable-extra-optimization    Turn on extra optimizations ],
     [case "${enableval}" in
       yes) OPTIMIZE_CFLAGS="-O3" ;;
       no)  OPTIMIZE_CFLAGS="-O2" ;; 
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-extra-optimization) ;;
     esac],[OPTIMIZE_CFLAGS=""])


AC_ARG_ENABLE(debug,[  --enable-debug    Turn on debugging],[
	case "${enableval}" in
	   yes)	debug=true ;
		DEBUG_CFLAGS="-DDEBUG -DUT_DEBUG -DFMT_TEST -DUT_TEST -DPT_TEST -g" ;
		OPTIMIZE_CFLAGS="" ;;
	    no)	debug=false ;
		DEBUG_CFLAGS="-DNDEBUG" ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	esac
],[	debug=false
	DEBUG_CFLAGS="-DNDEBUG"
])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

if test $debug = false; then
AC_ARG_ENABLE(symbols,[  --enable-symbols   Debugging symbols, but not debug-specific code],[
	case "${enableval}" in
	   yes)	symbols=true ;
		DEBUG_CFLAGS="-g" ;
		OPTIMIZE_CFLAGS="" ;;
	    no)	symbols=false ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-symbols) ;;
	esac
],[	symbols=false])
AM_CONDITIONAL(DEBUG, test x$symbols = xtrue)
fi

# by default, configure in the abi directory runs the same configure command
# in the peer directories (wv, expat, etc...).  this provides a way to
# disable that behavior.
AC_ARG_ENABLE(peer-config,
    [  --enable-peer-config  Enable automatic configuration of peer directories (on by default) ],
     [case "${enableval}" in
       yes) peerconfig=true ;;
       no)  peerconfig=false ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-peer-config) ;;
      esac],[peerconfig=true])

AC_ARG_ENABLE(scripting,
    [  --enable-scripting   Enable the perl scripting interface ],
     [case "${enableval}" in
       yes) script=true ;;
       no)  script=false ;; 
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-scripting) ;;
     esac],[script=false])

dnl if test "x$script" = "xtrue"; then
dnl 	AC_MSG_WARN([scripting not currently supported - please contact your priest for details])
dnl 	script=false
dnl fi

dnl this isn't actually used anywhere, but we provide it anyway
AM_CONDITIONAL(SCRIPT, test x$script = xtrue)

case "$script" in
     "true" ) 
	    SCRIPT_LIBS="`perl -MExtUtils::Embed -e ldopts`"
   AC_MSG_CHECKING(for perl)
   ABI_WHICH_PERL="`which perl 2>&1 | grep -v no\ perl`"
   dnl I had an actual check, but the error-if-absent part didn't work, and I was/am too drunk to figure out why
   AC_MSG_RESULT(${ABI_WHICH_PERL})
   AC_MSG_CHECKING(perl version)
   ABI_PERL_VERSION=`perl -V:version | tr -d "version=" | tr -d "\'" | tr -d "\;" | tr -d . | awk '{printf "%d", [$]1 * 10}'`
			if test "$ABI_PERL_VERSION" -le 5699; then
			    SCRIPT_CFLAGS="-DABI_OPT_PERL `perl -MExtUtils::Embed -e ccopts` -Ubool -DNOT_PERL_5_8"
			else
			    SCRIPT_CFLAGS="-DABI_OPT_PERL `perl -MExtUtils::Embed -e ccopts` -Ubool"
			fi
   AC_MSG_RESULT($ABI_PERL_VERSION)
		AC_MSG_CHECKING(for perl xsubpp precompiler)
		XSUBPPDIR="`(cd $abs_srcdir/src/bindings/perl && perl -MExtUtils::MakeMaker -e 'print ExtUtils::MakeMaker->new({NAME => qw(abi)})->tool_xsubpp') | grep ^XSUBPPDIR | sed -e 's/XSUBPPDIR = //g;'`"
		if test -n "${XSUBPPDIR}"; then
			AC_MSG_RESULT(${XSUBPPDIR})
		else
			AC_MSG_ERROR(not found)
		fi
	    ;;
     * )     
    	    SCRIPT_CFLAGS=""
	    SCRIPT_LIBS=""
	    ;;
esac

if test "$PLATFORM" = "unix"; then
	if test "$gnome" = "true"; then
		PLATFORM_CFLAGS="$GNOME_CFLAGS $GMODULE_CFLAGS $GUCHARMAP_CFLAGS -DSUPPORTS_UT_IDLE=1"
		PLATFORM_LIBS="$GNOME_LIBS $GMODULE_LIBS $GUCHARMAP_LIBS"
	else
		PLATFORM_CFLAGS="$GTK_CFLAGS $GMODULE_CFLAGS $GUCHARMAP_CFLAGS -DSUPPORTS_UT_IDLE=1"
		PLATFORM_LIBS="$GTK_LIBS $GMODULE_LIBS $GUCHARMAP_LIBS"
	fi
fi
dnl AUTOCONF SUCKS
if test "$PLATFORM" = "cocoa"; then
	if "$debug" = "true"; then
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	else
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	fi
	PLATFORM_CFLAGS="$GLIB_CFLAGS -fpascal-strings -DNO_SYS_ERRLIST"
fi

if test "$PLATFORM" = "qnx"; then
	PLATFORM_LIBS="-fexceptions"
fi
dnl SUBST all the variables we just defined
AC_SUBST(ABI_FEATURE_DEFS)
AC_SUBST(OPTIMIZE_CFLAGS)
AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(SCRIPT_CFLAGS)
AC_SUBST(SCRIPT_LIBS)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)
AC_SUBST(XSUBPPDIR)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h strings.h sys/time.h unistd.h malloc.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
ABI_C_INLINE
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AM_CONDITIONAL(BIGENDIAN,[ test "x$ac_cv_c_bigendian" = "xyes" ])

dnl Currently, we need all of the following, but in the future we
dnl might not.  
dnl
dnl check for an xml parser

ABI_XML_PARSER

dnl check for wv

ABI_NEED_WV="yes"
ABI_WV($abi_rootdir/wv)

dnl check for zlib.
ABI_ZLIB

dnl check for libpng.
ABI_LIBPNG

dnl check for popt.
ABI_POPT

dnl check for libjpeg
ABI_LIBJPEG

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(re_comp regcomp strdup strstr strcasecmp stricmp)

AC_CHECK_FUNC(putenv,abi_putenv=maybe,abi_putenv=no)
AC_CHECK_FUNC(setenv,abi_setenv=maybe,abi_setenv=no)

if test $abi_putenv = no -a $abi_setenv = no; then
	AC_MSG_ERROR([* * * neither setenv nor putenv found! * * *])
fi
if test $abi_setenv = no; then
	AC_DEFINE(SETENV_MISSING,1,[use putenv, not setenv])
elif test $abi_putenv = maybe; then
	_abi_cppflags=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $WARNING_CFLAGS"

	AC_MSG_CHECKING([whether setenv declared in stdlib.h (for us)])
	AC_TRY_COMPILE([
#include <stdlib.h>
float setenv (int);
	],[
/* if this program fails then setenv isn't declared in stdlib.h 
 * or at least not for the flags we're using...
 */
	],[	AC_MSG_RESULT(no)
		dnl well, okay, setenv may not be missing but I'm going to
		dnl mark it as such:
		AC_DEFINE(SETENV_MISSING,1,[use putenv, not setenv])
	],[	AC_MSG_RESULT(yes)
	])

	CPPFLAGS=$_abi_cppflags
fi

if test $peerconfig = "true"; then
	for i in . $PEERDIRS; do
		if test $i != . ; then
			j=`basename $i`
			ABI_PEER_CONFIG($j,$i)
		fi
	done
fi

AC_SUBST(PEERDIRS)
AC_SUBST(PEERS)

AC_CONFIG_FILES([GNUmakefile
 src/GNUmakefile
 src/af/GNUmakefile
 src/af/ev/GNUmakefile
 src/af/ev/xp/GNUmakefile
 src/af/ev/cocoa/GNUmakefile
 src/af/ev/qnx/GNUmakefile
 src/af/ev/unix/GNUmakefile
 src/af/gr/GNUmakefile
 src/af/gr/xp/GNUmakefile
 src/af/gr/cocoa/GNUmakefile
 src/af/gr/unix/GNUmakefile
 src/af/gr/qnx/GNUmakefile
 src/af/util/GNUmakefile
 src/af/util/xp/GNUmakefile
 src/af/util/unix/GNUmakefile
 src/af/util/qnx/GNUmakefile
 src/af/xap/GNUmakefile
 src/af/xap/xp/GNUmakefile
 src/af/xap/cocoa/GNUmakefile
 src/af/xap/qnx/GNUmakefile
 src/af/xap/unix/GNUmakefile
 src/af/xap/unix/gnome/GNUmakefile
 src/other/GNUmakefile
 src/other/spell/GNUmakefile
 src/other/spell/xp/GNUmakefile
 src/other/ttftool/GNUmakefile
 src/other/ttftool/unix/GNUmakefile
 src/pkg/GNUmakefile
 src/pkg/linux/GNUmakefile
 src/pkg/linux/apkg/GNUmakefile
 src/text/GNUmakefile
 src/text/fmt/GNUmakefile
 src/text/fmt/xp/GNUmakefile
 src/text/ptbl/GNUmakefile
 src/text/ptbl/xp/GNUmakefile
 src/wp/GNUmakefile
 src/wp/ap/GNUmakefile
 src/wp/ap/xp/GNUmakefile
 src/wp/ap/cocoa/GNUmakefile
 src/wp/ap/qnx/GNUmakefile
 src/wp/ap/unix/GNUmakefile
 src/wp/ap/unix/gnome/GNUmakefile
 src/wp/impexp/GNUmakefile
 src/wp/impexp/cocoa/GNUmakefile
 src/wp/impexp/unix/GNUmakefile
 src/wp/impexp/qnx/GNUmakefile
 src/wp/impexp/xp/GNUmakefile
 src/wp/main/GNUmakefile
 src/wp/main/xp/GNUmakefile
 src/wp/main/cocoa/GNUmakefile
 src/wp/main/cocoa/bundle/GNUmakefile
 src/wp/main/cocoa/bundle/Info.plist
 src/wp/main/cocoa/AbiWord.app/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/MacOS/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Frameworks/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Resources/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Resources/English.lproj/GNUmakefile
 src/wp/main/qnx/GNUmakefile
 src/wp/main/unix/GNUmakefile
 src/tools/GNUmakefile
 src/tools/cdump/GNUmakefile
 src/tools/cdump/xp/GNUmakefile
 src/tools/cdump/qnx/GNUmakefile
 src/tools/pfa2afm/GNUmakefile
 src/tools/pfa2afm/unix/GNUmakefile
 src/tools/ttftool/GNUmakefile
 src/tools/ttftool/unix/GNUmakefile
 src/tools/scripts/GNUmakefile
 src/tools/scripts/unix/GNUmakefile
 src/bindings/GNUmakefile
 src/bindings/perl/GNUmakefile
 autopackage/abiword-gnome.apspec
 autopackage/abiword-gtk.apspec
])
AC_OUTPUT

echo " "

case "$PLATFORM" in
     "unix" )
       AC_MSG_RESULT([Configured to build on a Unix/Linux platform.]) ;;
     "win" )
       AC_MSG_RESULT([Configured to build on a Windows platform.]) ;;
     "beos" )
       AC_MSG_RESULT([Configured to build on a BeOS platform.]) ;;
     "qnx" )
       AC_MSG_RESULT([Configured to build on a QNX RTOS platform.]) ;;
     "cocoa" )
       AC_MSG_RESULT([Configured to build on a Mac OS X platform using Cocoa.]) ;;
esac

case "$gnome" in
     "true" )
        AC_MSG_RESULT([Configured with GNOME support.]) ;;
esac

case "$debug" in
     "true" )
        AC_MSG_RESULT([Configured with debugging symbols.]) ;;
esac

case "$script" in
     "true" )
        AC_MSG_RESULT([Configured with scripting support.]) ;;
esac

case "$bidi" in
     "true" )
        AC_MSG_RESULT([Configured with BiDi support.]) ;;
esac

case "$xft" in
     "true" )
	AC_MSG_RESULT([Configured with XFT support.]) ;;
esac

echo " "

cat <<EOF
  XML Parser:      ${abi_xml_parser_message}
  zlib:            ${abi_zlib_message}
  PNG Library:     ${abi_libpng_message}
  popt:		   ${abi_libpopt_message}
  WV Library:      ${abi_wv_message}
  iconv:           ${abi_iconv_message}
  spelling:        ${abi_spell_message}

EOF

if test "x$_abi_wv_warning" = "xyes"; then
	ABI_WV_WARNING
fi

AC_MSG_RESULT([Type make to build AbiWord.])
