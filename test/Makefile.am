
INCLUDES = \
	-I.. \
    $(YDATA_CFLAGS) \
    -pg

noinst_PROGRAMS = testdata test-tcp-server

testdata_SOURCES = testdata.c
testdata_LDADD = \
	../src/libydata-0.0.la \
	-lm
	
testdata_LDFLAGS = -static

test_tcp_server_SOURCES = test-tcp-server.c
test_tcp_server_LDADD = \
	../src/libydata-0.0.la \
	-lm
	
test_tcp_server_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.h5
