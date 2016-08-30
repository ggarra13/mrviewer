## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	create_CLUT_profile

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC \
	-L$(top_builddir)/Contrib/ICC_utils -lICC_utils

create_CLUT_profile_SOURCES = \
	Black_scaler.h Black_scaler.cpp \
	create_CLUT_profile.cpp
