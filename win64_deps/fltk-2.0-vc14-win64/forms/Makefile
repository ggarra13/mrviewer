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
# Please report all bugs and problems to the following website:
#
#    http://www.fltk.org/str.php
#

CPPFILES = \
	Fl_Chart.cxx \
	Fl_Positioner.cxx \
	forms_bitmap.cxx \
	forms_compatability.cxx \
	forms_free.cxx \
	forms_fselect.cxx \
	forms_pixmap.cxx \
	forms_timer.cxx \
	fl_load_browser.cxx

DEMOS = \
	forms colbrowser

################################################################

include ../makeinclude
DSOCFLAGS += -DFL_FORMS_LIBRARY
DSOCXXFLAGS += -DFL_FORMS_LIBRARY
LDLIBS := $(FORMSLIBS) $(LDLIBS)

LIBRARY = ../lib/$(LIBNAME)_forms
DSO = ../lib/$(DSOLIBformsNAME)
DSONAME = $(DSOLIBformsNAME)$(DSOEXT)
LIBRARIES = $(LIBRARY)$(LIBEXT) $(DSO)$(DSOEXT)

OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

all:	$(BUILDTARGETS)

static:	$(LIBRARY)$(LIBEXT)

shared:	$(DSO)$(DSOEXT)

demos: $(DEMOS)

$(DEMOS): $(FLFORMSLIB) $(FLLIB)

programs: demos

$(LIBRARY).a: $(OBJECTS)
	@rm -f $@
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

$(DSO).so.2 $(DSO).sl.2: $(OBJECTS)
	@rm -f $@
	$(DSOCOMMAND) $(DSONAME) $(OBJECTS) $(DSOLIBS)
	mv $(DSONAME) $@

DLLIMPORTLIBS = ../lib/$(LIBNAME).2.dll$(LIBEXT)
DLLIMPORTNAME = $(LIBNAME)_forms.2.dll$(LIBEXT)
$(DSO).2.dll: $(OBJECTS)
	@rm -f $@
	$(DSOCOMMAND) $@ $(OBJECTS) $(DSOLIBS) $(DLLIMPORTLIBS)
	mv $@.import ../lib/$(DLLIMPORTNAME)

#.fl.cxx .fl.h:
#	echo Generating $@...
#	../fluid/fluid -c $<

clean :
	@rm -f *.o $(DEMOS) *.exe $(LIBRARY)$(LIBEXT) $(DSO)$(DSOEXT) \
	       ../lib/$(DLLIMPORTNAME) core *~

depend:	$(CPPFILES)
	$(MAKEDEPEND) -I.. $(CPPFILES) > makedepend

# Automatically generated dependencies...
makedepend:
	touch makedepend
include makedepend

################################################################

install: $(INSTALLTARGETS)

install_static: static
	@echo "Installing static libraries..."
	@mkdir -p $(libdir)
	@rm -f $(libdir)/$(LIBNAME)_forms$(LIBEXT)
	@cp $(LIBRARY)$(LIBEXT) $(libdir)
	@-chmod 644 $(libdir)/$(LIBNAME)_forms$(LIBEXT) 

install_shared: shared
	@echo "Installing shared libraries..."
	@rm -f $(libdir)/$(DLLIMPORTNAME)
	@rm -f $(libdir)/$(DSOLIBformsNAME)$(DSOEXT)
	@if [ "$(DSOSHORTEXT)" ]; then rm -f $(libdir)/$(DSOLIBformsNAME)$(DSOSHORTEXT); fi
	@cp $(DSO)$(DSOEXT) $(libdir)
	@chmod 755 $(libdir)/$(DSOLIBformsNAME)$(DSOEXT)
	@if [ "$(DSOSHORTEXT)" ]; then \
		ln -s $(DSOLIBformsNAME)$(DSOEXT) $(libdir)/$(DSOLIBformsNAME)$(DSOSHORTEXT); fi
	@if [ -f "../lib/$(DLLIMPORTNAME)" ]; then cp ../lib/$(DLLIMPORTNAME) $(libdir); fi

install_programs:

#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
