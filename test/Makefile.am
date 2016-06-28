
INCLUDES = \
	-I.. \
    $(YDATA_CFLAGS) \
    -pg

noinst_PROGRAMS = testdata

testdata_SOURCES = testdata.c
testdata_LDADD = \
	../src/libydata-0.0.la \
	-lm
	
testdata_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.h5
