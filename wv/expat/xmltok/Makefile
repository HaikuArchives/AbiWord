# Generated automatically from Makefile.in by configure.
EXEEXT = 
OBJEXT = o

LDFLAGS = 

CC = gcc

SHELL=/bin/sh

RANLIB = ranlib

AR = ar

srcdir=.

# If you know what your system's byte order is, define XML_BYTE_ORDER:
# use -DXML_BYTE_ORDER=12 for little-endian byte order;
# use -DXML_BYTE_ORDER=21 for big-endian (network) byte order.
# -DXML_NS adds support for checking of lexical aspects of XML namespaces spec
# -DXML_MIN_SIZE makes a smaller but slower parser

CFLAGS = -I$(srcdir)  -g -O2

# Use one of the next two lines; unixfilemap is better if it works.
FILEMAP_OBJ=xmlwf/unixfilemap.o
#FILEMAP_OBJ=xmlwf/readfilemap.o
SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)

TOKENOBJECTS =	\
	xmltok.$(OBJEXT) \
	xmlrole.$(OBJEXT)

all: libxmltok.a

libxmltok.a:	$(TOKENOBJECTS)
	$(AR) rc $@ $(TOKENOBJECTS) 
	$(RANLIB) $@

%.$(OBJEXT): $(srcdir)/%.c
	$(CC) $(CFLAGS) -c $(srcdir)/$*.c

clean: 
	rm -rf $(TOKENOBJECTS)

distclean: clean
	rm -rf Makefile *.a 

realclean: distclean

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
