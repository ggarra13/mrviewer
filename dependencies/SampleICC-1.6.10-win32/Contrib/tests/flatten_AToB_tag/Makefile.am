## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	flatten_AToB_tag

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC \
	-L$(top_builddir)/Contrib/ICC_utils -lICC_utils

flatten_AToB_tag_SOURCES = \
	flatten_AToB_tag.cpp
