AM_CPPFLAGS = \
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/src \
	$(YDATA_CFLAGS)

ydata_include_dir = $(includedir)/libydata-@YDATA_API_VER@/ydata
