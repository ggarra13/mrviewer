## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	reconstruct_measurements

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC \
	-L$(top_builddir)/Contrib/ICC_utils -lICC_utils

reconstruct_measurements_SOURCES = \
	Measurement_extractor.h Measurement_extractor.cpp \
	reconstruct_measurements.cpp
