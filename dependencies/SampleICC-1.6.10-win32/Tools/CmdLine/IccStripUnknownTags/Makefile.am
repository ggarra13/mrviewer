## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	iccStripUnknownTags

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC

iccStripUnknownTags_SOURCES = \
	iccStripUnknownTags.cpp
