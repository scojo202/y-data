# Makefile.am for goffice

SUBDIRS = src docs test

EXTRA_DIST = README NEWS BUGS MAINTAINERS AUTHORS \
	COPYING-gpl2 COPYING-gpl3 \
	y-data.mk \
	ydata.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

DISTCLEANFILES = intltool-extract intltool-merge intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc \
	--enable-introspection=auto \
	--with-gir-dir=\$${datadir}/gir-1.0 \
	--with-typelib-dir=\$${libdir}/girepository-1.0

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = y-data-@YDATA_API_VER@.pc

%-@YDATA_API_VER@.pc: %.pc
	cp $< $@

CLEANFILES = $(pkgconfig_DATA)

include $(top_srcdir)/y-data.mk
