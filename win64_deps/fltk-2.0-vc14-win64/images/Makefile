#
# "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $"
#
# Library makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2003 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to the following page:
#
#    http://www.fltk.org/str.php
#

#
# Include common definitions...
#

include ../makeinclude

# If ZLIBINC is set, we are using local ZLIB
ifneq ( ,$(ZLIBINC))
	CFLAGS += -I./zlib
	CXXFLAGS += -I./zlib
endif

#
# Files for this directory...
#

CPPFILES = \
	FileIcon2.cxx \
	Fl_Guess_Image.cxx \
	fl_jpeg.cxx\
	fl_png.cxx \
	HelpDialog.cxx \
	images_core.cxx \
	pnmImage.cxx \
	xpmFileImage.cxx
CFILES	=

LIBNAME	=	$(LIBPREFIX)fltk2_images$(LIBSUFFIX)
DSONAME	=	$(DSOPREFIX)fltk2_images$(DSOSUFFIX)
DSOLINK = 	$(DSOPREFIX)fltk2_images$(DSOSYMLINK)
OBJECTS =	$(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

#
# Make everything...
#

all:		$(LIBTARGET) $(DSOTARGET)

static:		../lib/$(LIBNAME)

shared:		../lib/$(DSONAME)

#
# Clean old files...
#

clean:
	$(RM) *.bck
	$(RM) *.o
	$(RM) core*
	$(RM) ../lib/$(LIBNAME)
	$(RM) ../lib/$(DSONAME)
        ifeq ($(DSONAME), fltk2_images.dll)
	$(RM) lib$(DSONAME).a $(DSONAME)
        endif


#
# Make dependencies, excluding standard include directories...
#

depend:
	$(MAKEDEPEND) -I.. $(CPPFILES) $(CFILES) > makedepend


#
# Include automatically generated dependencies...
#

makedepend:
	touch makedepend
include makedepend


#
# Make static libraries...
#

../lib/$(LIBNAME): $(OBJECTS)
	$(RM) $@
	echo $(LIBCOMMAND) $@ ...
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

../lib/$(DSONAME): $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS)
	$(RM) ../lib/$(DSOLINK)
	$(LN) $(DSONAME) ../lib/$(DSOLINK)

../lib/fltk2_images.dll: $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS) $(IMAGELIBS) $(LOCAL_IMAGELIBS) $(LINKFLTK) $(LDLIBS)
	cp ../lib/fltk2_images.dll ./

#
# Install everything...
#

install:
        ifneq (,$(wildcard ../lib/$(LIBNAME)))
	echo "Installing static images library in $(libdir)"
	$(MKDIR) $(DESTDIR)$(libdir)
	$(RM) $(DESTDIR)$(libdir)/$(LIBNAME)
	$(CP) ../lib/$(LIBNAME) $(DESTDIR)$(libdir)
	$(RANLIB) $(DESTDIR)$(libdir)/$(LIBNAME)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBPREFIX)fltk2_z$(LIBSUFFIX)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBPREFIX)fltk2_png$(LIBSUFFIX)
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBPREFIX)fltk2_jpeg$(LIBSUFFIX)
  ifneq (,$(LOCAL_IMAGELIBS))
	  $(CP) $(LOCAL_IMAGELIBS) $(DESTDIR)$(libdir)
	  $(RANLIB) $(DESTDIR)$(libdir)/$(LOCAL_IMAGELIBS)
  endif
        endif
        ifneq (,$(wildcard ../lib/$(DSONAME)))
	echo "Installing shared images library in $(libdir)"
	$(MKDIR) $(DESTDIR)$(libdir)
	$(RM) $(DESTDIR)$(libdir)/$(DSONAME)
	$(RM) $(DESTDIR)$(libdir)/$(DSOLINK)
	$(CP) ../lib/$(DSONAME) $(DESTDIR)$(libdir)
	$(LN) -s $(DSONAME) $(DESTDIR)$(libdir)/$(DSOLINK)
        endif
        ifeq ($(DSONAME), fltk2_images.dll)
	$(CP) lib$(DSONAME).a $(DESTDIR)$(libdir)
        endif
#
# Uninstall the libraries...
#

uninstall:
	echo "Removing static images library from $(libdir)"
	$(RM) $(libdir)/$(LIBNAME)
	echo "Removing shared images library from $(libdir)"
	$(RM) $(libdir)/$(DSONAME)
	$(RM) $(libdir)/$(DSOLINK)
        ifeq ($(DSONAME), fltk2_images.dll)
	$(RM) $(libdir)/lib$(DSONAME).a
        endif

#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
