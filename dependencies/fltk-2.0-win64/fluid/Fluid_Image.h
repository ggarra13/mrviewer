//
// "$Id: Fluid_Image.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Pixmap image header file for the Fast Light Tool Kit (FLTK).
//
// This class stores the image labels for widgets in fluid.  This is
// not a class in fltk itself, and this will produce different types of
// code depending on what the image type is.  There are private subclasses
// in Fluid_Image.C for each type of image format.  Right now only xpm
// files are supported.
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#ifndef FLUID_IMAGE_H
#define FLUID_IMAGE_H

#include "Fluid_Plugins.h"
#include <fltk/SharedImage.h>

class Fluid_Image {
  const char *name_;
  int refcount;
protected:
  Fluid_Image(const char *name); // no public constructor
  virtual ~Fluid_Image(); // no public destructor
public:
  bool inlined;
  int written;
  static Fluid_Image* find(const char *);
  void decrement(); // reference counting & automatic free
  void increment();
  virtual const fltk::Symbol* symbol() = 0; // return the fltk Symbol object
  virtual void write_static() = 0;
  virtual void write_code() = 0;
  const char *name() const {return name_;}
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Fluid_Image *ui_find_image(Fluid_Image* old);

FLUID_API extern const char *images_dir;

#endif

//
// End of "$Id: Fluid_Image.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
