## Process this file with automake to produce Makefile.in

bin_PROGRAMS = iccApplyProfiles

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib $(TIFF_CPPFLAGS)

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC $(TIFF_LDFLAGS) $(TIFF_LIBS)

iccApplyProfiles_SOURCES = iccApplyProfiles.cpp TiffImg.cpp TiffImg.h
