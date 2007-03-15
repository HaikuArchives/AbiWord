# Generated automatically from Makefile.in by configure.
CC = gcc
OBJEXT = o
LEX = @LEX@

SHELL=/bin/sh



prefix=/usr/local
srcdir=.
datadir=${prefix}/share

HAVE_GLIB = 1
HAVE_GNOMEVFS= 

ifeq ($(EPATH_DYLIB),1)
WVPICFLAGS = -fno-common -DPIC
else
WVPICFLAGS = 
endif

CFLAGS = -I.. -I$(srcdir) -I$(srcdir)/..  -g -O2 -DHAVE_CONFIG_H -I/boot/home/config/include/glib-1.2 -I/boot/home/config/lib/glib/include $(WVPICFLAGS)
ifneq ($(HAVE_GLIB), 1)
CFLAGS += -I$(srcdir)/../glib-wv
endif

SHELL = /bin/sh

.SUFFIXES: .C .$(OBJEXT)

MYOBJECTS = \
	ms-ole.$(OBJEXT) \
	ms-ole-summary.$(OBJEXT)

ifeq ($(HAVE_GNOMEVFS),1)
MYOBJECTS += ms-ole-gnomevfs.$(OBJEXT)
endif

all: $(MYOBJECTS) 

ms-ole.$(OBJEXT): $(srcdir)/ms-ole.c 
	$(CC) $(CFLAGS) -c $(srcdir)/ms-ole.c 

ms-ole-summary.$(OBJEXT): $(srcdir)/ms-ole-summary.c
	$(CC) $(CFLAGS) -c $(srcdir)/ms-ole-summary.c

ms-ole-gnomevfs.$(OBJEXT): $(srcdir)/ms-ole-gnomevfs.c
	$(CC) $(CFLAGS) -c $(srcdir)/ms-ole-gnomevfs.c

clean:	
	rm -rf $(MYOBJECTS) core 

realclean:  
	rm -rf Makefile *.$(OBJEXT) core demo

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
