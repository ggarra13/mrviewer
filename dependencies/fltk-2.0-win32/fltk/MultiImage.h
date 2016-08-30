//
// "$Id: MultiImage.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Image type that draws a different image depending on the flags,
// for instace VALUE or SELECTED or HIGHLIGHT.
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

#ifndef fltk_MultiImage_h
#define fltk_MultiImage_h

#include "Symbol.h"
#include <stdarg.h>

namespace fltk {

class FL_API MultiImage : public Symbol
{
  const Symbol* image0;
  struct MultiImagePair {
    const Symbol* image;
    Flags flags;
  };
  MultiImagePair* pairs;
  unsigned n_images;

public:
  void _measure(int&, int&) const;
  void _draw(const Rectangle&) const;
  const Symbol* current_image() const;
  void inset(Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;

  ~MultiImage() { release();}

  //! for MultiImage arrays construction using set() for post initialization
  MultiImage() { pairs=0; n_images = 0; } 
  
  //! constructor for unlimited images state affectation
  MultiImage(unsigned count, const Symbol* img0, va_list ap) { set(count,img0, ap); }
    
  void set (unsigned count, const Symbol* img0, ...); // fabien: need to be accessible because of MultiImage arrays with post (set) affectations

  void add(Flags flags, const Symbol& image);

  //! Destroys everything except image0.
  void release() { delete[] pairs; pairs = 0; n_images = 0; }

  MultiImage(const Symbol& img0) : image0(&img0), pairs(0), n_images(0) {}

  /* compatibility convenient constructors */
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1) { set(2, &img0,f1,&img1); }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1,Flags f2, const Symbol& img2) {
    set(3, &img0, f1, &img1, f2, &img2); 
  }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1, Flags f2, const Symbol& img2, Flags f3, const Symbol& img3) {
    set(4, &img0, f1, &img1, f2, &img2, f3, &img3);
  }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1, Flags f2, const Symbol& img2, Flags f3, const Symbol& img3,
	     Flags f4, const Symbol& img4) {
    set(5, &img0, f1, &img1, f2, &img2, f3, &img3, f4, &img4); 
  }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1, Flags f2, const Symbol& img2, Flags f3, const Symbol& img3,
	     Flags f4, const Symbol& img4, Flags f5, const Symbol& img5) {
    set(6, &img0, f1, &img1, f2, &img2, f3, &img3, f4, &img4, f5, &img5); 
  }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1, Flags f2, const Symbol& img2, Flags f3, const Symbol& img3,
	     Flags f4, const Symbol& img4, Flags f5, const Symbol& img5, Flags f6, const Symbol& img6) {
    set(7, &img0, f1, &img1, f2, &img2, f3, &img3, f4, &img4, f5, &img5, f6, &img6); 
  }
  MultiImage(const Symbol& img0, Flags f1, const Symbol& img1, Flags f2, const Symbol& img2, Flags f3, const Symbol& img3,
	     Flags f4, const Symbol& img4, Flags f5, const Symbol& img5, Flags f6, const Symbol& img6,  Flags f7, const Symbol& img7 ) {
    set(8, &img0, f1, &img1, f2, &img2, f3, &img3, f4, &img4, f5, &img5, f6, &img6, f7, &img7); 
  }
};

}


#endif

//
// End of "$Id: MultiImage.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
