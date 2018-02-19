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
# Please report all bugs and problems on the following page
#
#    http://www.fltk.org/str.php
#

#
# Include common definitions...
#

include ../makeinclude


#
# Files for this directory...
#

CPPFILES = \
	Fl_Gl_Choice.cxx \
	Fl_Gl_Overlay.cxx \
	Fl_Gl_Window.cxx \
	gl_draw.cxx \
	gl_start.cxx

CFILES	=

LIBNAME	=	$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
DSONAME	=	$(DSOPREFIX)fltk2_gl$(DSOSUFFIX)
DSOLINK = 	$(DSOPREFIX)fltk2_gl$(DSOSYMLINK)
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
        ifeq ($(DSONAME), fltk2_gl.dll)
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

../lib/fltk2_gl.dll: $(OBJECTS)
	echo $(DSOCOMMAND) $@ ...
	$(DSOCOMMAND) $@ $(OBJECTS) $(IMAGELIBS) $(LOCAL_IMAGELIBS) $(LINKFLTK) $(GLDLIBS)
	cp ../lib/fltk2_gl.dll ./

#
# Install everything...
#

install:
        ifneq (,$(wildcard ../lib/$(LIBNAME)))
	echo "Installing static OpenGL library in $(libdir)"
	$(MKDIR) $(DESTDIR)$(libdir)
	$(RM) $(DESTDIR)$(libdir)/$(LIBNAME)
	$(CP) ../lib/$(LIBNAME) $(DESTDIR)$(libdir)
        endif
        ifneq (,$(wildcard ../lib/$(DSONAME)))
	echo "Installing shared OpenGL library in $(libdir)"
	$(MKDIR) $(DESTDIR)$(libdir)
	$(RM) $(DESTDIR)$(libdir)/$(DSONAME)
	$(RM) $(DESTDIR)$(libdir)/$(DSOLINK)
	$(CP) ../lib/$(DSONAME) $(DESTDIR)$(libdir)
	$(LN) -s $(DSONAME) $(DESTDIR)$(libdir)/$(DSOLINK)
        endif
        ifeq ($(DSONAME), fltk2_gl.dll)
	$(CP) lib$(DSONAME).a $(DESTDIR)$(libdir)
        endif

#
# Uninstall the libraries...
#

uninstall:
	echo "Removing static OpenGL library from $(libdir)"
	$(RM) $(libdir)/$(LIBNAME)
	echo "Removing shared OpenGL library from $(libdir)"
	$(RM) $(libdir)/$(DSONAME)
	$(RM) $(libdir)/$(DSOLINK)
        ifeq ($(DSONAME), fltk2_gl.dll)
	$(RM) $(libdir)/lib$(DSONAME).a
        endif


#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
