#
# "$Id: Makefile 1186 2000-06-10 04:10:35Z carl $"
#
# Fluid plugin makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-1999 by Bill Spitzak and others.
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
# Please report all bugs and problems to "fltk-bugs@easysw.com".
#

PROGRAM = essai.flpl

CPPFILES = \
	essai_plugin.cxx

################################################################

OBJECTS = $(CPPFILES:.cxx=.o)

XTRAFLAGS = -I../../fluid -I../..

include ../../makeinclude

all: $(PROGRAM) Fl_NSlider.o

$(PROGRAM) : $(OBJECTS) ../../lib/$(LIBNAME).a
	$(DSOCOMMAND) $@ $(OBJECTS) $(DSOLIBS)

clean :
	-@ rm -f *.o $(PROGRAM) $(CLEAN) core *~ makedepend
	@touch makedepend

depend:
	$(MAKEDEPEND) $(XTRAFLAGS) $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend

install:

include makedepend

#
# End of "$Id: Makefile 1186 2000-06-10 04:10:35Z carl $".
#
