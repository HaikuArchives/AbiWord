INCLUDES=-I.. -I../compat

lib_LTLIBRARIES = libpsiconv.la
libpsiconv_la_SOURCES = error.c misc.c checkuid.c list.c buffer.c data.c \
                        parse_common.c parse_driver.c parse_formula.c \
                        parse_layout.c parse_image.c parse_page.c  \
                        parse_simple.c parse_texted.c parse_word.c \
                        parse_sheet.c \
                        generate_simple.c generate_layout.c generate_driver.c \
                        generate_common.c generate_texted.c generate_page.c \
                        generate_word.c generate_image.c
libpsiconv_la_LDFLAGS = -version-info 8:0:3
libpsiconv_la_LIBADD = ../compat/libcompat.la

psiconvincludedir = $(includedir)/psiconv
psiconvinclude_HEADERS = data.h parse.h list.h parse_routines.h error.h \
                         generate_routines.h generate.h common.h buffer.h

# There must be some automake-way to do this nicer, but for now it works.
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(psiconvincludedir)
	p=general.h; \
	if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(psiconvincludedir)/$$p"; \
	$(INSTALL_DATA) $$d$$p $(DESTDIR)$(psiconvincludedir)/$$p; 
