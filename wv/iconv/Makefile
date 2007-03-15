# Generated automatically from Makefile.in by configure.
CC = gcc
OBJEXT = o

SHELL=/bin/sh

INSTALL = /bin/install -c
INSTALL_DATA = ${INSTALL} -m 644



prefix=/usr/local
exec_prefix=${prefix}
srcdir=.
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
mandir=${prefix}/man
includedir = ${prefix}/include
PERL = @PERL@


CFLAGS = -g -O2 -DHAVE_CONFIG_H -I$(srcdir)/ -I$(srcdir)/../ -I$(srcdir)/../libole2/ -I$(srcdir)/../glib-wv/ 

SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)

ICONVOBJECTS =	\
	iconv.$(OBJEXT) \
	codepage-874.$(OBJEXT) \
	codepage-1250.$(OBJEXT) \
	codepage-1251.$(OBJEXT) \
	codepage-1252.$(OBJEXT) \
	codepage-1253.$(OBJEXT) \
	codepage-1254.$(OBJEXT) \
	codepage-1255.$(OBJEXT) \
	codepage-1256.$(OBJEXT) \
	codepage-1257.$(OBJEXT) \
	iso-8859-15.$(OBJEXT) \
	tis-620.$(OBJEXT) \
	utf-8.$(OBJEXT) \
	koi8-r.$(OBJEXT)

all: $(ICONVOBJECTS)

%.$(OBJEXT): $(srcdir)/%.c 
	$(CC) $(CFLAGS) -c $(srcdir)/$*.c 

clean:
	rm -rf *.$(OBJEXT) core a.$(OBJEXT)ut *.a

distclean: 
	rm -rf config.cache config.status config.log Makefile config.h *.a *.$(OBJEXT) core out* .pure

realclean: distclean

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
