## Process this file with automake to produce Makefile.in

SUBDIRS = \
	IccProfLib \
	Tools \
	Contrib

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sampleicc.pc

EXTRA_DIST = bootstrap \
	README.OSX \
	Build/XCode/ICC_utils-Info.plist \
	Build/XCode/SampleICC-Info.plist \
	Build/XCode/SampleICC.xcodeproj/project.pbxproj

dist-hook:
	rm -fr `find $(distdir) -name .DS_Store -or -name .svn`
