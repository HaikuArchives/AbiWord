AC_INIT([libpng],[1.2.5],[http://bugzilla.abisource.com/])
AC_CONFIG_SRCDIR([pngconf.h])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_RANLIB

AM_INIT_AUTOMAKE(libpng,1.2.5)

FR_ZLIB_OPT(no)
ZLIB_CPPFLAGS="$fr_zlib_cppflags"
AC_SUBST(ZLIB_CPPFLAGS)

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT
