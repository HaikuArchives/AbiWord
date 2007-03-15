# start: libeXtra/FraMiNG/ac-helpers/fr-zlib.m4
# 
# Copyright (C) 2002 Francis James Franklin
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The above license applies to THIS FILE ONLY.
#
# Check for optional zlib

dnl Usage: FR_ZLIB_OPT(yes|no)

AC_DEFUN([FR_ZLIB_OPT],[

fr_zlib_cppflags=""
fr_zlib_ldflags=""
fr_zlib_prefix=""

if [ test "x$1" = "xyes" ]; then
	fr_zlib_opt=check
else
	fr_zlib_opt=required
fi

# At this point fr_zlib_opt can be "check" or "required"

AC_ARG_WITH(zlib,[  --with-zlib=DIR         use libz in DIR],[
	if test "x$withval" = "xyes"; then
		fr_zlib_opt=specified
	elif test "x$withval" != "xno"; then
		fr_zlib_prefix="$withval"
		fr_zlib_opt=specified
	else
		if test $fr_zlib_opt = required; then
			AC_MSG_ERROR([* * * zlib is required * * *])
		fi
		fr_zlib_opt=no
	fi
])

# At this point fr_zlib_opt can be "check", "required", "specified", or "no"

if test $fr_zlib_opt != no; then
	if test "x$fr_zlib_prefix" != "x"; then
		_fr_cppflags="$CPPFLAGS"
		_fr_ldflags="$LDFLAGS"
		CPPFLAGS="$CPPFLAGS -I$fr_zlib_prefix/include"
		LDFLAGS="$LDFLAGS -L$fr_zlib_prefix/lib"
	fi
	AC_CHECK_HEADER(zlib.h,[
		AC_CHECK_LIB(z,gzread,[
			fr_zlib_opt=yes
			fr_zlib_ldflags="-lz"
		],[	AC_CHECK_LIB(gz,gzread,[
				fr_zlib_opt=yes
				fr_zlib_ldflags="-lgz"
			],[	if test $fr_zlib_opt != check; then
					AC_MSG_ERROR([* * * failed to find zlib! * * *])
				fi
				fr_zlib_opt=no
			],-lm)
		],-lm)
	],[	if test $fr_zlib_opt != check; then
			AC_MSG_ERROR([* * * failed to find zlib! * * *])
		fi
		fr_zlib_opt=no
	])
	if test "x$fr_zlib_prefix" != "x"; then
		if test $fr_zlib_opt = yes; then
			fr_zlib_cppflags="-I$fr_zlib_prefix/include"
			fr_zlib_ldflags="-L$fr_zlib_prefix/lib $fr_zlib_ldflags"
		fi
		CPPFLAGS="$_fr_cppflags"
		LDFLAGS="$_fr_ldflags"
	fi
fi

# At this point fr_zlib_opt can be "yes" or "no"

])
# 
# end: libeXtra/FraMiNG/ac-helpers/fr-zlib.m4
# 
