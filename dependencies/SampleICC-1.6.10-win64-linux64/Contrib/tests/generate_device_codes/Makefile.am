## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	generate_device_codes

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/Contrib/ICC_utils -lICC_utils \
	-L$(top_builddir)/IccProfLib -lSampleICC

generate_device_codes_SOURCES = \
	generate_device_codes.cpp
