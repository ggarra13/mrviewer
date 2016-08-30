## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libICC_utils.la

libICC_utils_la_SOURCES = Stubs.cpp \
	CAT.cpp \
	CLUT.cpp \
	CLUT_stuffer.cpp \
	ICC_tool_exception.cpp \
	Vetters.cpp

libICC_utils_la_LDFLAGS = -version-info @LIBTOOL_VERSION@

libICC_utils_includedir = $(includedir)/SampleICC

libICC_utils_include_HEADERS = Stubs.h \
	CAT.h \
	CLUT.h \
	CLUT_stuffer.h \
	ICC_tool_exception.h \
	Vetters.h

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC

