# Generated automatically from Makefile.in by configure.
CC = gcc
OBJEXT = o

SHELL=/bin/sh



srcdir=.

HAVE_GLIB = 1

CFLAGS = -I.. -I$(srcdir) -I$(srcdir)/..  -g -O2 -DHAVE_CONFIG_H -I/boot/home/config/include/glib-1.2 -I/boot/home/config/lib/glib/include
ifneq ($(HAVE_GLIB), 1)
CFLAGS += -I$(srcdir)/../glib-wv
endif

SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)
:g/\.0/s//\.$(OBJEXT)/g

MYOBJECTS = \
	garray.$(OBJEXT) \
	glist.$(OBJEXT) \
	gmem.$(OBJEXT) \
	gmessages.$(OBJEXT) \
	gslist.$(OBJEXT) \
	gstrfuncs.$(OBJEXT) \
	gtree.$(OBJEXT) 

all: $(MYOBJECTS) 

%.$(OBJEXT): $(srcdir)/%.c 
	$(CC) $(CFLAGS) -c $(srcdir)/$*.c 

clean:	
	rm -rf $(MYOBJECTS) core 

realclean:  
	rm -rf config.cache config.status config.log Makefile *.$(OBJEXT) core demo

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
