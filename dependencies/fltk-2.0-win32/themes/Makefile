#
# "$Id: Makefile 8636 2011-05-06 08:01:12Z bgbnbigben $"
#
# FLTK-plugin makefile for the Fast Light Tool Kit (FLTK).
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

# * essai.theme removed from default list because it requires -ljpeg
#   and breaks the build on some platforms

# * These themes wont compile either
# 	Alternative.theme \
#	classic.theme \
#	motif.theme \
#	scheme.theme \

THEMES = \
	KDE.theme \
	none.theme \
	windows.theme

CPPFILES = \
	windows.cxx \
	none.cxx \
	scheme.cxx

################################################################

include ../makeinclude

LIBNAME = 	$(LIBPREFIX)fltk2_themes$(LIBSUFFIX)
DSONAME = 	$(DSOPREFIX)fltk2_themes$(DSOSUFFIX)
DSOLINK = 	$(DSOPREFIX)fltk2_themes$(DSOSUFFIX)
OBJECTS =	$(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

all: 		$(LIBTARGET) $(DSOTARGET)

static: 	../lib/$(LIBNAME)

shared:		../lib/$(DSONAME)

clean:
	$(RM) *.bck
	$(RM) *.o
	$(RM) core*
	$(RM) ../lib/$(LIBNAME)
	$(RM) ../lib/$(DSONAME)
        ifeq ($(DSONAME), fltk_themes.dll)
	$(RM) lib$(DSONAME).a $(DSONAME)
        endif

$(LIBRARY).a: $(OBJECTS)
	@rm -f $@
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

ESSAI_OBJ = essai.o
essai.theme : $(ESSAI_OBJ)
	$(THEMECOMMAND) $@ $(ESSAI_OBJ) ../lib/libfltk2_images.so.2 -ljpeg -lpng $(DSOLIBS)

MOTIF_OBJ = motif.o
motif.theme : $(MOTIF_OBJ)
	$(THEMECOMMAND) $@ $(MOTIF_OBJ) $(DSOLIBS) $(THEMELIBS)

CLASSIC_OBJ = classic.o
classic.theme : $(CLASSIC_OBJ)
	$(THEMECOMMAND) $@ $(CLASSIC_OBJ) $(DSOLIBS) $(THEMELIBS)

WINDOWS_OBJ = windows.o
windows.theme : $(WINDOWS_OBJ)
	$(THEMECOMMAND) $@ $(WINDOWS_OBJ) $(DSOLIBS) $(THEMELIBS)

ALT_OBJ = Alternative.o
Alternative.theme : $(ALT_OBJ)
	$(THEMECOMMAND) $@ $(ALT_OBJ) $(DSOLIBS) $(THEMELIBS)

KDE_OBJ = KDE.o conf.o conf_get.o conf_util.o conf_list_free.o conf_keys.o
KDE.theme : $(KDE_OBJ)
	$(THEMECOMMAND) $@ $(KDE_OBJ) $(DSOLIBS) $(THEMELIBS)

SCHEME_OBJ = scheme.o conf.o conf_get.o conf_util.o conf_list_free.o conf_keys.o conf_sections.o
scheme.theme : $(SCHEME_OBJ)
	$(THEMECOMMAND) $@ $(SCHEME_OBJ) $(DSOLIBS) $(THEMELIBS)

NONE_OBJ = none.o
none.theme : $(NONE_OBJ)
	$(THEMECOMMAND) $@ $(NONE_OBJ) $(DSOLIBS) $(THEMELIBS)

../lib/$(LIBNAME): $(OBJECTS)
	$(RM) $@
	echo $(LIBCOMMAND) $@ ...
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@


install : $(INSTALLTARGETS)

install_static:
# it would be nice if this built .o versions of the themes

install_shared:
	@echo "Installing themes..."
	@-mkdir -p $(libdir)/fltk
	@cp -f $(THEMES) $(libdir)/fltk

install_programs:

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) *.cxx *.c > makedepend
	sed -e "s'\.o'\\.o'" makedepend > foo
	cat foo >> makedepend
	rm foo

makedepend:
	touch makedepend
include makedepend

#
# End of "$Id: Makefile 8636 2011-05-06 08:01:12Z bgbnbigben $".
#
