#
# "$Id: Makefile 4923 2006-04-10 12:02:26Z fabien $"
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
CFLAGS += -I../..

#
# Files for this directory...
#

CPPFILES =
CFILES	= jaricom.c jmemnobs.c \
		jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c \
		jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c \
		jcparam.c jcprepct.c jcsample.c jctrans.c \
		jdapimin.c jdapistd.c jdatadst.c jdatasrc.c jdcoefct.c \
		jdcolor.c jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c \
		jdmaster.c jdmerge.c jdpostct.c jdsample.c \
		jdtrans.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c jmemmgr.c \
		jidctflt.c jidctfst.c jidctint.c jquant1.c jcarith.c jdarith.c \
		jquant2.c jutils.c # jmemmac.c jmemdos.c

LIBNAME	=	$(LIBPREFIX)fltk2_jpeg$(LIBSUFFIX)
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
	$(MAKEDEPEND) -fmakedepend -I.. -I../.. $(CPPFILES) $(CFILES)


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
# End of "$Id: Makefile 4923 2006-04-10 12:02:26Z fabien $".
#
