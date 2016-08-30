## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	create_pretransform_curve

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/Contrib/ICC_utils -lICC_utils \
	-L$(top_builddir)/IccProfLib -lSampleICC

create_pretransform_curve_SOURCES = \
	create_pretransform_curve.cpp

EXTRA_DIST = \
	01_make_probe_image.sh \
	ConversionProject.aep \
	pretransform.txt \
	02_make_input_profile_from_probe.sh \
	README

