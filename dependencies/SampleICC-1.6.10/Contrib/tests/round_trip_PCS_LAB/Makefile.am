## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	round_trip_PCS_LAB

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC \
	-L$(top_builddir)/Contrib/ICC_utils -lICC_utils

round_trip_PCS_LAB_SOURCES = \
	round_trip_PCS_LAB.cpp
