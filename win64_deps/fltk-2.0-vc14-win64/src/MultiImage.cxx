/*! \class fltk::MultiImage

  A Symbol containing pointers to a set of different Symbols.  The
  drawflags() are used to select which one to draw.  This is most
  useful for making an image for a button that is different depending
  on whether it is pushed in, the current value is on, or when the
  mouse is over it.

  The basic constructor takes the "image0" which is the image that
  is drawn if no other image is used. You can then use add() or replace()
  to add pairs of images and flags. These are searched and the last
  one where all it's flags are turned on is used. There are also
  inline constructors so that a fully-populated MultiImage with up
  to 8 images can be declared inline.

  Typical example:

\code
  MultiImage buttonImage(OffImage,
		STATE, OnImage,
		INACTIVE_R, DimOffImage,
		INACTIVE_R|STATE, DimOnImage,
                FOCUSED, FocusedOffImage,
                FOCUSED|STATE, FocusedOnImage,
		HIGHLIGHT, BrightOffImage,
		HIGHLIGHT|STATE, BrightOnImage,
                PUSHED, BrightPushedOffImage,
                PUSHED|STATE, BrightPushedOnImage);
\endcode

  In the above example, because the PUSHED is later than the
  FOCUSED, when the user pushes the button and it has the focus
  they will see the pushed image. If they were the other way around
  they would see the focused image and not see any feedback from
  pushing the button. In addition, although the hightlight or
  focus should not turn on for an inactive widget, this will show
  if it happens.

  Also note that the number of images is less than 2^n where n is the
  number of flags you care about. This seems to be true of most
  hand-painted image sets. The user probably does not care or is
  confused by showing the focus or highlight state of a button they
  are pushing.

  A fully-populated example like the above is not necessary,
  as the flags are passed to the sub-images. If they respond to
  them (for instance drawing differently depending on STATE) then
  fewer images are necessary.

  Useful flags are:
  - INACTIVE_R
  - STATE True for button that is turned on
  - HIGHLIGHT True when the mouse is over widget or pushing it (you
  must also set highlight_color() to non-zero or most widgets will
  not redraw because they don't think they changed appearance).
  - PUSHED True if user is pushing button
  - FOCUSED True if button has the keyboard focus

*/

#include <fltk/MultiImage.h>
#include <fltk/draw.h>
#include <fltk/string.h>
#include <stdarg.h>

using namespace fltk;

/*! \fn MultiImage::MultiImage(const Symbol& image0)
  Constructor where only the "image0" is provided. Unless you call add(),
  this will always just draw \a image0.
*/

/*!
  Makes it draw \a image if \e all of the \a flags are turned on in
  drawflags().

  If \a flags is zero, this replaces the image0 that was provided
  to the constructor. Otherwise, if any image with \a flags has
  already been specified, it is replaced with this image. Otherwise
  a new pair of flags and image is added to the end of the list.
*/
void MultiImage::add(Flags flags, const Symbol& image) {
  if (!flags) {
    image0 = &image;
    set_inset(image.get_inset());
    // release(); // this would be consistent with documentation?
    return;
  }
  // This replaces the last image that would become impossible to draw
  // because this new image hides it. It really should remove all such
  // images...
  for (unsigned i = n_images; i--; ) {
    if ((flags & pairs[i].flags)==flags) {
      pairs[i].flags = flags;
      pairs[i].image = &image;
      return;
    }
  }
  // this is not very efficient (it reallocates the array every time):
  // fabien: true, I initially thought of a simple linked list impl then I thought:
  // we have more memory to alloc and also alloc fragmentation and finally
  // it would also slow down non-default Multimage constructors
  // mainly because of the links pointers updates and multiple allocs 
  // so i think we can stay with that impl. if we safely suppose
  // that main use would be one shot alloc for MultiImage ...

  MultiImagePair* newpairs = new MultiImagePair[n_images+1];
  memcpy(newpairs, pairs, n_images*sizeof(*pairs));
  newpairs[n_images].flags = flags;
  newpairs[n_images].image = &image;
  delete[] pairs;
  pairs = newpairs;
  n_images++;
}

/*! \fn MultiImage::MultiImage(const Symbol& image0, Flags flags1, const Symbol& image1);
  If all of \a flags1 are on in drawimage(), then \a image1 will be
  drawn, otherwise \a image0 will be drawn.
*/

/*! \fn MultiImage::MultiImage(const Symbol& image0, Flags flags1, const Symbol& image1, Flags flags2, const Symbol& image2);
  If all of \a flags2 are on in drawimage(), then \a image2 will be
  drawn, else if all of \a flags1 are on then \a image1 will be
  drawn, otherwise \a image0 will be drawn.

  This pattern continues for the other inline constructors. The
  \e last image and flags are checked first, and then backwards.
*/

/*! Innards of the inline constructors. */
void MultiImage::set(unsigned count, const Symbol* img0, ...) {
  image0 = img0;
  set_inset(img0->get_inset());
  n_images = count>0 ? count-1 : 0; // don't authorize setting 2^32-1 images count on 32bit systems
  if (count > 1) {
    pairs =  new MultiImagePair[n_images];
    va_list ap;
    va_start(ap, img0);
    for (unsigned i=0; i<n_images; i++) {
      pairs[i].flags = va_arg(ap,int);
      pairs[i].image = (Symbol*) va_arg(ap, void*);
    }
    va_end(ap);
  }
}

/*! It probably is useless for the images to be different sizes.
  However if they are, Image0 (the first one passed to the
  constructor) is used to figure out the size. */
void MultiImage::_measure(int& w, int& h) const { image0->measure(w,h); }

/*! Returns the info from the first image given to the constructor. */
bool MultiImage::fills_rectangle() const {return image0->fills_rectangle();}

/*! Returns the info from the first image given to the constructor. */
bool MultiImage::is_frame() const {return image0->is_frame();}

/*! Return the image that will be drawn according to the current value
  of drawflags(). The \e last image with all the flags specified for
  it turned on will be drawn. If none of them match then image0
  (the first one passed to the constructor) is returned.
*/
const Symbol* MultiImage::current_image() const {
  const Flags f = drawflags();
  for (unsigned i = n_images; i--;)
    if ((f & pairs[i].flags)==pairs[i].flags)
      return pairs[i].image;
  return image0;
}

/*! Calls the same image that _draw() will call to get the inset. */
void MultiImage::inset(Rectangle& r) const {
  current_image()->inset(r);
}

/*! Select one of the images and draw it. The last image with all the
  flags specified for it turned on will be drawn. If none of them match
  then Image0 is drawn.
*/
void MultiImage::_draw(const Rectangle& r) const
{
  current_image()->draw(r);
}
