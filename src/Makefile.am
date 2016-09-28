-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path="${INTROSPECTION_GIRDIR}" \
	-v --warn-all --accept-unprefixed \
	--namespace Y --nsversion="@YDATA_API_VER@" \
	--identifier-prefix="Y" \
	--symbol-prefix="y" --symbol-prefix="libydata"

INTROSPECTION_COMPILER_ARGS = \
	--includedir=$(srcdir) \
	--includedir=. \
	--includedir="${INTROSPECTION_GIRDIR}"

#####################################
# graph directory files

INCLUDES = -std=c99

ydata_SOURCES =				\
	y-data.c \
	y-data-simple.c \
	y-hdf.c \
	y-data-tcp.c \
	y-data-vector-slice.c \
	y-operation.c \
	y-data-derived.c \
	y-linear-range.c \
	y-scalar-property.c \
	y-vector-ring.c

ydatadir = $(y_include_dir)

ydata_HEADERS =					\
	y-data.h \
	y-data-class.h \
	y-data-simple.h \
	y-hdf.h \
	y-data-tcp.h \
	y-data-vector-slice.h \
	y-operation.h \
	y-data-derived.h \
	y-linear-range.h \
	y-scalar-property.h \
	y-vector-ring.h

lib_LTLIBRARIES = libydata-@YDATA_API_VER@.la

libydata_@YDATA_API_VER@_la_LIBADD = 			\
	$(YDATA_LIBS)

libydata_@YDATA_API_VER@_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -lhdf5 -lhdf5_hl

libydata_@YDATA_API_VER@_la_SOURCES = \
        $(ydata_SOURCES)

AM_CPPFLAGS = \
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/src \
	$(YDATA_CFLAGS)

y_include_dir = $(includedir)/libydata-@YDATA_API_VER@

YDATA_HDRS = $(ydata_HEADERS)

if HAVE_INTROSPECTION

INTROSPECTION_GIRS += YData-@YDATA_API_VER@.gir

YData-@YDATA_API_VER@.gir: \
	$(INTROSPECTION_SCANNER) \
	libydata-@YDATA_API_VER@.la

YData_@YDATA_API_VER_@_gir_INCLUDES = \
	GObject-2.0 \
	Gio-2.0
YData_@YDATA_API_VER_@_gir_CFLAGS = $(AM_CPPFLAGS)
YData_@YDATA_API_VER_@_gir_LIBS = libydata-@YDATA_API_VER@.la
YData_@YDATA_API_VER_@_gir_FILES = \
	$(libydata_@YDATA_API_VER@_la_SOURCES) \
	$(YDATA_HDRS)

girdir = $(GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(TYPELIBDIR)
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelibs_DATA)

endif # HAVE_INTROSPECTION
