#
# "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
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
# FLUID file rules...
#

.fl.cxx .fl.h:
	../fluid/fluid2$(EXEEXT) -c $<


#
# Files for this directory...
#

CPPFILES = \
	CubeMain.cxx \
	CubeView.cxx \
	ansiwidget.cxx \
	arc.cxx \
	ask.cxx \
	bitmap.cxx \
	boxtype.cxx \
	browser.cxx \
	button.cxx \
	buttons.cxx \
	cairo.cxx \
	callbacks.cxx \
	checkers.cxx \
	clock.cxx \
	color_chooser.cxx \
	cube.cxx \
	cursor.cxx \
	curve.cxx \
	demo.cxx \
	doublebuffer.cxx \
	drawing.cxx \
	drawtiming.cxx \
	editor.cxx \
	file_chooser.cxx \
	fonts.cxx \
	fullscreen.cxx \
	glpuzzle.cxx \
	gl_overlay.cxx \
	hello.cxx \
	helloask.cxx \
	iconize.cxx \
	image.cxx \
	image_transform.cxx \
	input.cxx \
	keyboard.cxx \
	label.cxx \
	list_visuals.cxx \
	mandelbrot.cxx \
	menu.cxx \
	message.cxx \
	monitors.cxx \
	navigation.cxx \
	output.cxx \
	overlay.cxx \
	pixmap.cxx \
	pixmap_browser.cxx \
	radio.cxx \
	resizable.cxx \
	resizealign.cxx \
	scroll.cxx \
	shape.cxx \
	subwindow.cxx \
	symbols.cxx \
	tabs.cxx \
	tile.cxx \
	timer.cxx \
	fast_slow.cxx \
	resize.cxx \
	pack.cxx \
	inactive.cxx \
	sizes.cxx \
	image_file.cxx \
	progress.cxx \
	layout.cxx \
	threads.cxx \
	menubar.cxx \
	line_style.cxx \
	utf.cxx \
	valuators.cxx \
	wizard.cxx

CFILES	=

# only some of the demos have been changed to use namespaces:
TARGETS = \
	arc$(EXEEXT) \
        ansiwidget$(EXEEXT) \
	bitmap$(EXEEXT) \
	boxtype$(EXEEXT) \
	browser$(EXEEXT) \
	cairo$(EXEEXT) \
	callbacks$(EXEEXT) \
	checkers$(EXEEXT) \
	clock$(EXEEXT) \
	button$(EXEEXT) \
	buttons$(EXEEXT) \
	color_chooser$(EXEEXT) \
	cube$(EXEEXT) \
	CubeView$(EXEEXT) \
	cursor$(EXEEXT) \
	curve$(EXEEXT) \
	demo$(EXEEXT) \
	doublebuffer$(EXEEXT) \
	drawing$(EXEEXT) \
	drawtiming$(EXEEXT) \
	editor$(EXEEXT) \
	exception$(EXEEXT) \
	file_chooser$(EXEEXT) \
	fonts$(EXEEXT) \
	fractals$(EXEEXT) \
	glpuzzle$(EXEEXT) \
	gl_overlay$(EXEEXT) \
	hello$(EXEEXT) \
	image$(EXEEXT) \
	image_transform$(EXEEXT) \
	inactive$(EXEEXT) \
	input$(EXEEXT) \
	keyboard$(EXEEXT) \
	label$(EXEEXT) \
	line_style$(EXEEXT) \
	list_visuals$(EXEEXT) \
	mandelbrot$(EXEEXT) \
	menu$(EXEEXT) \
	message$(EXEEXT) \
	monitors$(EXEEXT) \
	navigation$(EXEEXT) \
	output$(EXEEXT) \
	pack$(EXEEXT) \
	pixmap$(EXEEXT) \
	progress$(EXEEXT) \
	radio$(EXEEXT) \
	qubix$(EXEEXT) \
	resizable$(EXEEXT) \
	resizealign$(EXEEXT) \
	scroll$(EXEEXT) \
	shape$(EXEEXT) \
	sizes$(EXEEXT) \
	subwindow$(EXEEXT) \
	symbols$(EXEEXT) \
	threads$(EXEEXT) \
	tabs$(EXEEXT) \
	tile$(EXEEXT) \
	timer$(EXEEXT) \
	utf$(EXEEXT) \
	fullscreen$(EXEEXT) \
	valuators$(EXEEXT) \
	list$(EXEEXT) \
	wizard$(EXEEXT)

#
# Make everything...
#

all:	$(TARGETS)


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
# All demo programs depend on the static library.
#

$(TARGETS):	../lib/$(LIBPREFIX)fltk2$(LIBSUFFIX)


#
# Other programs needing special "help"...
#

button$(EXEEXT): button.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKIMG) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

cairo$(EXEEXT): cairo.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTK) $(LDLIBS) -o $@

CubeView$(EXEEXT): CubeMain.o CubeView.o CubeViewUI.o ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) CubeMain.o CubeView.o CubeViewUI.o \
		$(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

cube$(EXEEXT): cube.o ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

cursor$(EXEEXT): cursor.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKIMG) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

fullscreen$(EXEEXT): fullscreen.o ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

fractals$(EXEEXT): fractals.o fracviewer.o ../lib/$(LIBPREFIX)fltk2_glut$(LIBSUFFIX)  ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) fractals.o fracviewer.o ../lib/$(LIBPREFIX)fltk2_glut$(LIBSUFFIX) $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

glpuzzle$(EXEEXT): glpuzzle.o ../lib/$(LIBPREFIX)fltk2_glut$(LIBSUFFIX)  ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) glpuzzle.o ../lib/$(LIBPREFIX)fltk2_glut$(LIBSUFFIX) $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

gl_overlay$(EXEEXT): gl_overlay.o ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

mandelbrot$(EXEEXT): mandelbrot.o mandelbrot_ui.o
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) mandelbrot.o mandelbrot_ui.o $(LINKFLTK) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

shape$(EXEEXT): shape.o ../lib/$(LIBPREFIX)fltk2_gl$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKGL) $(GLDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

file_chooser$(EXEEXT): file_chooser.o ../lib/$(LIBPREFIX)fltk2_images$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKIMG) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r
image_file$(EXEEXT): image_file.o ../lib/$(LIBPREFIX)fltk2_images$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKIMG) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r

help$(EXEEXT): help.o ../lib/$(LIBPREFIX)fltk2_images$(LIBSUFFIX)
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTKIMG) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../fltk/mac.r


install:
uninstall:


#
# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
# DO NOT DELETE
