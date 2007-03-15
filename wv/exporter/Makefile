# Generated automatically from Makefile.in by configure.
EXEEXT=
OBJEXT=o

CC=gcc

LDFLAGS=

SHELL=/bin/sh

srcdir=.

HAVE_GLIB=1

INCLUDES = -I.. -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../libole2 -I$(srcdir)/../oledecod
ifneq ($(HAVE_GLIB), 1)
INCLUDES += -I$(srcdir)/../glib-wv
endif

DEFS = -DHAVE_CONFIG_H

CPPFLAGS =  -I/boot/home/config/include/glib-1.2 -I/boot/home/config/lib/glib/include

ifeq ($(EPATH_DYLIB),1)
WVPICFLAGS = -fno-common -DPIC
else
WVPICFLAGS = 
endif

CFLAGS = -g -O2 -ansi -pedantic $(WVPICFLAGS)

.SUFFIXES: .C .$(OBJEXT)
:g/\.0/s//\.$(OBJEXT)/g

MYOBJECTS = \
	anld.$(OBJEXT) \
	anlv.$(OBJEXT) \
	asumy.$(OBJEXT) \
	asumyi.$(OBJEXT) \
	atrd.$(OBJEXT) \
	bkd.$(OBJEXT) \
	bkf.$(OBJEXT) \
	bkl.$(OBJEXT) \
	brc.$(OBJEXT) \
	bte.$(OBJEXT) \
	bx.$(OBJEXT) \
	chp.$(OBJEXT) \
	dcs.$(OBJEXT) \
	dogrid.$(OBJEXT) \
	dop.$(OBJEXT) \
	doptypography.$(OBJEXT) \
	dttm.$(OBJEXT) \
	fdoa.$(OBJEXT) \
	ffn.$(OBJEXT) \
	fib.$(OBJEXT) \
	filetime.$(OBJEXT) \
	fld.$(OBJEXT) \
	font.$(OBJEXT) \
	frd.$(OBJEXT) \
	fspa.$(OBJEXT) \
	ftxbxs.$(OBJEXT) \
	generic.$(OBJEXT) \
	lfo.$(OBJEXT) \
	lspd.$(OBJEXT) \
	lst.$(OBJEXT) \
	lvl.$(OBJEXT) \
	numrm.$(OBJEXT) \
	olst.$(OBJEXT) \
	pcd.$(OBJEXT) \
	pgd.$(OBJEXT) \
	phe.$(OBJEXT) \
	prm.$(OBJEXT) \
	rr.$(OBJEXT) \
	rs.$(OBJEXT) \
	sed.$(OBJEXT) \
	sep.$(OBJEXT) \
	shd.$(OBJEXT) \
	stylesheet.$(OBJEXT) \
	summary.$(OBJEXT) \
	support.$(OBJEXT) \
	tbd.$(OBJEXT) \
	tc.$(OBJEXT) \
	tlp.$(OBJEXT) \
	wkb.$(OBJEXT) \
	wvexporter.$(OBJEXT) \
	exptest.$(OBJEXT)

all: $(MYOBJECTS)

exptest$(EXEEXT): exptest.$(OBJEXT) ../libwv.a
	$(CC) $(CFLAGS) -o $@ exptest.$(OBJEXT) -L. -L.. -lglib -lwv $(LIB) $(LDFLAGS)

%.$(OBJEXT): $(srcdir)/%.c 
	$(CC) $(INCLUDES) $(DEFS) $(CPPFLAGS) $(CFLAGS) -c $(srcdir)/$*.c 

clean:	
	rm -rf $(MYOBJECTS) core exptest

distclean: realclean

realclean:
	rm -rf Makefile *.$(OBJEXT) core 

#  DO  NOT  DELETE THIS LINE -- make depend depends on it.
