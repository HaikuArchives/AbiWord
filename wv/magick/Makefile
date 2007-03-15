# Generated automatically from Makefile.in by configure.
CC = gcc
OBJEXT = o

SHELL=/bin/sh



prefix=/usr/local
exec_prefix=${prefix}
srcdir=.
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
mandir=${prefix}/man
includedir = ${prefix}/include
PERL = @PERL@


ifeq ($(EPATH_DYLIB),1)
WVPICFLAGS = -fno-common -DPIC
else
WVPICFLAGS = 
endif

CFLAGS = -I.. -I$(srcdir) -I$(srcdir)/.. -DHAVE_CONFIG_H  -g -O2   $(WVPICFLAGS)

SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)

MAGICKOBJECTS =	\
	magick.$(OBJEXT) \
	image.$(OBJEXT) \
	blob.$(OBJEXT) \
	colors.$(OBJEXT) \
	error.$(OBJEXT) \
	transform.$(OBJEXT) \
	compress.$(OBJEXT) \
	memory.$(OBJEXT) \
	utility.$(OBJEXT) \
	png.$(OBJEXT) \
	bmp.$(OBJEXT) \
	signature.$(OBJEXT)


all: $(MAGICKOBJECTS)

%.$(OBJEXT): $(srcdir)/%.c 
	$(CC) $(CFLAGS) -c $(srcdir)/$*.c 

clean:
	rm -rf *.$(OBJEXT) core a.$(OBJEXT)ut *.a

distclean: 
	rm -rf config.cache config.status config.log Makefile config.h *.a *.$(OBJEXT) core out* .pure

realclean: distclean

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
