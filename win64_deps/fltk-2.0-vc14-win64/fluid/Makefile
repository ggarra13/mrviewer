#
# "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $"
#
# FLUID makefile for the Fast Light Tool Kit (FLTK).
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


#
# Files for this directory...
#

CPPFILES = \
	about_panel.cxx \
	align_widget.cxx \
	alignment_panel.cxx \
	code.cxx \
	CodeEditor.cxx \
	coding_style.cxx \
	coding_style_func.cxx \
	Enumeration.cxx \
	factory.cxx \
	file.cxx \
	fluid.cxx \
	Fluid_Image.cxx \
	fluid_img.cxx \
	fluid_menus.cxx \
	Fluid_Plugins.cxx \
	FluidType.cxx \
	function_panel.cxx \
	FunctionType.cxx \
	GroupType.cxx \
	MenuType.cxx \
	template_panel.cxx \
	undo.cxx \
	widget_panel.cxx \
	WidgetClassType.cxx \
	WidgetType.cxx \
	WindowType.cxx

CFILES	=

OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.cxx=.o)
TARGETS	= fluid2$(EXEEXT) $(FLUID)

#CXXFLAGS+=-g
#
# Make everything...
#

all:		$(TARGETS)


#
# Clean old files...
#

clean:
	$(RM) *.bck
	$(RM) *.o
	$(RM) core*
	$(RM) $(TARGETS)


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
# Build FLUID...
#

fluid2$(EXEEXT): $(OBJECTS) ../lib/$(LIBPREFIX)fltk2$(LIBSUFFIX) \
		../lib/$(LIBPREFIX)fltk2_images$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKFLTKIMG) $(LIBS) $(LDLIBS) $(LOCAL_IMAGELIBS)
	$(POSTBUILD) $@ ../fltk/mac.r


#
# Build shared FLUID...
#

fluid2-shared$(EXEEXT): $(OBJECTS) ../src/$(DSOPREFIX)fltk2$(DSOSUFFIX) \
		../images/$(DSOPREFIX)fltk2_images$(DSOSUFFIX)
	echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LINKSHARED) $(IMAGELIBS) $(LOCAL_IMAGELIBS) $(LIBS) $(LDLIBS)
	$(POSTBUILD) $@ ../fltk/mac.r


#
# Install everything...
#

install:
	echo "Installing FLUID2 in $(bindir)..."
	-$(MKDIR) $(DESTDIR)$(bindir)
	$(CP) $(FLUID) $(DESTDIR)$(bindir)/fluid2$(EXEEXT)
	$(STRIP) $(DESTDIR)$(bindir)/fluid2$(EXEEXT)
	$(DESTDIR)$(bindir)/fltk2-config --post $(DESTDIR)$(bindir)/fluid2$(EXEEXT)
	$(CHMOD) 755 $(DESTDIR)$(bindir)/fluid2$(EXEEXT)


uninstall:
	$(RM) $(bindir)/fluid2


#
# Note: The rebuild target can only be used if you have the original .fl
#       files.  This is normally only used by the FLTK maintainers...
#

rebuild:
	./fluid2$(EXEEXT) -c about_panel.fl
	./fluid2$(EXEEXT) -c alignment_panel.fl
	./fluid2$(EXEEXT) -c function_panel.fl
	./fluid2$(EXEEXT) -c widget_panel.fl
	./fluid2$(EXEEXT) -c coding_style.fl


#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
