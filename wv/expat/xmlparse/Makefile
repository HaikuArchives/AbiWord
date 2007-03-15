# Generated automatically from Makefile.in by configure.
EXEEXT = 
OBJEXT = o

LDFLAGS = 

CC = gcc

SHELL=/bin/sh

RANLIB = ranlib

AR = ar

srcdir=.

CFLAGS = -I$(srcdir)/../xmltok -I$(srcdir)  -g -O2

SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)

PARSEOBJECTS =	\
	xmlparse.$(OBJEXT) \
	hashtable.$(OBJEXT)

all: libxmlparse.a

libxmlparse.a:	$(PARSEOBJECTS)
	$(AR) rc $@ $(PARSEOBJECTS) 
	$(RANLIB) $@

%.$(OBJEXT): $(srcdir)/%.c
	$(CC) $(CFLAGS) -c $(srcdir)/$*.c

clean: 
	rm -rf $(PARSEOBJECTS) 

distclean: clean
	rm -rf Makefile *.a

realclean: distclean

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
