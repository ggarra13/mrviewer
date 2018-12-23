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

include ../../makeinclude


#
# Files for this directory...
#

CPPFILES =
CFILES	= adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c \
		gzread.c gzwrite.c infback.c inffast.c inflate.c \
		inftrees.c trees.c uncompr.c zutil.c

LIBNAME	=	$(LIBPREFIX)fltk2_z$(LIBSUFFIX)
OBJECTS =	$(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

#
# Make everything...
#

all:		$(LIBTARGET)

static:		../../lib/$(LIBNAME)


#
# Clean old files...
#

clean:
	$(RM) *.bck
	$(RM) *.o
	$(RM) core*
	$(RM) ../../lib/$(LIBNAME)


#
# Make dependencies, excluding standard include directories...
#

depend:
	$(MAKEDEPEND) -fmakedepend -I.. $(CPPFILES) $(CFILES)


#
# Include automatically generated dependencies...
#

include makedepend


#
# Make static libraries...
#

../../lib/$(LIBNAME): $(OBJECTS)
	$(RM) $@
	echo $(LIBCOMMAND) $@ ...
	$(LIBCOMMAND) $@ $(OBJECTS)
	$(RANLIB) $@

#
# Install everything...
#

install:

#
# Uninstall the libraries...
#

uninstall:

#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
