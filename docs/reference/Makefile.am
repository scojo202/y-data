DOC_MODULE=y-@YDATA_API_VER@
DOC_MAIN_SGML_FILE=ydata-docs.xml
DOC_SOURCE_DIR=../../src

SCANGOBJ_OPTIONS= --nogtkinit

SCAN_OPTIONS= --deprecated-guards="Y_DISABLE_DEPRECATED" --rebuild-types

MKDB_OPTIONS=--xml-mode --output-format=xml

MKTMPL_OPTIONS=

FIXXREF_OPTIONS=

HFILE_GLOB=$(top_srcdir)/src/*.h
CFILE_GLOB=$(top_srcdir)/src/*.c

IGNORE_HFILES=	\
		y-config.h        		

HTML_IMAGES=

content_files = \
        $(top_srcdir)/docs/reference/version.xml

expand_content_files=

GTKDOC_CFLAGS = -I$(top_srcdir) -I$(top_builddir) $(YDATA_CFLAGS)
GTKDOC_LIBS = $(top_builddir)/src/libydata-@YDATA_API_VER@.la $(YDATA_LIBS)

include $(top_srcdir)/gtk-doc.make

# To work around bug #314105, we have to distribute the generated file, too.
EXTRA_DIST += version.xml.in version.xml

CLEANFILES += \
	y-@YDATA_API_VER@-decl-list.txt	\
	y-@YDATA_API_VER@-unused.txt	\
	y-@YDATA_API_VER@-decl.txt   	\
	y-@YDATA_API_VER@-undeclared.txt	\
	y-@YDATA_API_VER@-overrides.txt	\
	y-@YDATA_API_VER@-undocumented.txt	\
	y-@YDATA_API_VER@.types	\
	html/* tmpl/* xml/*

$(top_srcdir)/docs/reference/version.xml:
	cp $(top_builddir)/docs/reference/version.xml $@
