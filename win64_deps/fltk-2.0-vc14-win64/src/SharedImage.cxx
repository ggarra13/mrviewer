// "$Id: SharedImage.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $"
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

// Draw an image that is stored compressed in a file or in memory. 
// Keep uncompressed images in memory for later use. 
// WAS: why? it's already in the Pixmap, ready for drawing!

#include <config.h>
#include <fltk/SharedImage.h>
#include <fltk/xbmImage.h>
#include <fltk/string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

using namespace fltk;

const char*	SharedImage::shared_image_root=0;
SharedImage*	SharedImage::first_image = 0;
int		SharedImage::image_used=0;
unsigned	SharedImage::mem_usage_limit=0;

////////////////////////////
// static module variables
////////////////////////////
SharedImage::Handler *SharedImage::handlers_ = 0;// Additional format handlers
int	SharedImage::num_handlers_ = 0;	// Number of format handlers
int	SharedImage::alloc_handlers_ = 0;	// Allocated format handlers

void SharedImage::set_cache_size(unsigned l)
{
  mem_usage_limit = l;
}

static SharedImage *limage; // used to find the less used image
void SharedImage::find_less_used() {
  if(l1) l1->find_less_used();
  if(l2) l2->find_less_used();
  if(mem_used() && (!limage->mem_used() || used<limage->used)) limage=this;
}
void SharedImage::check_mem_usage()
{
  if (mem_usage_limit==0 || first_image==NULL ||
      total_mem_used() <= mem_usage_limit)
    return;

  do {
    limage=first_image;
    first_image->find_less_used();
    if (!limage->mem_used()) break;
    limage->destroy();
  } while(total_mem_used() >= mem_usage_limit);
}

// WAS: this is probably a waste of time! No modern system requires these
// to be deleted.
class shared_image_destructor_class {
  int dummy;
public:
  shared_image_destructor_class() { dummy = 0; }
  ~shared_image_destructor_class() {
    if (SharedImage::first_image) SharedImage::first_image->clear_cache();
  }
};

shared_image_destructor_class shared_image_destructor;

/*! Calls destroy() on this and every less-recently-used SharedImage?. */
void SharedImage::clear_cache()
{
  destroy();
  if (l1 && l1!=this) l1->clear_cache();
  if (l2 && l2!=this) l2->clear_cache();
}

void SharedImage::set_root_directory(const char *d) {
  shared_image_root = d;
}

void SharedImage::insert(SharedImage*& p, SharedImage* image) {
  if(p == 0)
    p = image;
  else {
    int c = strcmp(image->name, p->name);
    if(c<0 &&p->l1!=p) insert(p->l1, image);
    else {
	if (p->l2!=p) insert(p->l2, image);
    }
  }
}

SharedImage* SharedImage::find(SharedImage* image, const char* name) {
  if(image == 0) return 0;
  int c = strcmp(name, image->name);
  if(c == 0) return image;
  else if(c<0) return image->l1!= image ? find(image->l1, name) : 0;
  else return image->l2!= image ? find(image->l2, name) : 0;
}


const char* SharedImage::get_filename() const {
  return get_filename(name);
}

/*! Prepends the shared_image_root to the name. This is what get_filename()
  would return for a SharedImage with this name. */
const char* SharedImage::get_filename(const char* name)
{
  if (name[0] == '/' || !shared_image_root || !*shared_image_root)
    return name;
  int m = strlen(shared_image_root);
  int n = strlen(name) + m + 2;
  static char *s;
  delete[] s;
  s = new char[n+1];
  strcpy(s, shared_image_root);
  if (s[m-1] != '/') s[m++] = '/';
  strcpy(s+m, name);
  return s;
}


SharedImage* SharedImage::get(SharedImage* (*create)(),
				      const char* name, const uchar *datas)
{
  SharedImage *image=SharedImage::find(first_image, name);
  if(!image)
  {
    image=create();
    image->refcount = 1;
    image->name = newstring(name);
    image->datas = datas;
    image->l1 = image->l2 = 0;
    SharedImage::insert(first_image, image);
  } else {
    if(image->datas==NULL) image->datas=datas;
    image->refcount++;
  }
  image->used = image_used++;
  return image;
}

//
// 'SharedImage::get()' - Get a shared image whatever the type is...
//
SharedImage * SharedImage::get(const char *n) {
  SharedImage *img=SharedImage::find(first_image, n);

  if (img!=NULL) return img;
    
    // Load image from disk...
  int		i;		// Looping var
  FILE		*fp;		// File pointer
  uchar		header[64];	// Buffer for auto-detecting files


  if (n  && (*n) && (fp = fopen(n, "rb")) != NULL) {
    if(fread(header, 1, sizeof(header), fp)); // ignore the return value
    fclose(fp);
  } else {
    return NULL;
  }

  // Load the image as appropriate...
  if (memcmp(header, "BM", 2) == 0)	// BMP file
      img = bmpImage::get(n);
  else if (memcmp(header, "GIF87a", 6) == 0 ||
      memcmp(header, "GIF89a", 6) == 0)	// GIF file
    img = gifImage::get(n);
  else {
    // Not a standard format; try an image handler...
    for (i = 0, img = 0; i < num_handlers_; i ++) {
      img = (handlers_[i])(n, header, sizeof(header));
      if (img) break;
    }
  }
  return img;
}

void SharedImage::reload(const uchar* pdatas)
{
  if (pdatas) datas = pdatas;
  refetch();
}
void SharedImage::reload(const char* name, const uchar* pdatas)
{
  SharedImage *image=SharedImage::find(first_image, name);
  if (image) image->reload(pdatas);
}

void SharedImage::remove_from_tree(SharedImage*& p, SharedImage* image) {
  if (p) {
    int c = strcmp(image->name, p->name);
    if (c == 0) {
      if (image->l1) {
	p = image->l1;
	if (image->l2) insert(first_image, image->l2);
      } else
	p = image->l2;
    } else if (c<0) remove_from_tree(p->l1, image);
    else remove_from_tree(p->l2, image);
  }
}

int SharedImage::remove()
{
  if (--refcount) return 0;
  remove_from_tree(first_image, this);
  return 1;
}

int SharedImage::remove(const char* name)
{
  SharedImage *image=SharedImage::find(first_image, name);
  if (image) return image->remove();
  else return 0;
}

void SharedImage::_draw(const Rectangle& r) const {
  const_cast<SharedImage*>(this)->used = image_used++; // do this before check_mem_usage
  Image::_draw(r);
  check_mem_usage();
}

//
// 'SharedImage::add_handler()' - Add a shared image handler.
//
void SharedImage::add_handler(SharedImage::Handler f) {
  int			i;		// Looping var...
  SharedImage::Handler 	*temp;		// New image handler array...

  // First see if we have already added the handler...
  for (i = 0; i < num_handlers_; i ++) {
    if (handlers_[i] == f) return;
  }

  if (num_handlers_ >= alloc_handlers_) {
    // Allocate more memory...
    temp = new SharedImage::Handler  [alloc_handlers_ + 32];

    if (alloc_handlers_) {
      memcpy(temp, handlers_, alloc_handlers_ * sizeof(SharedImage::Handler ));

      delete[] handlers_;
    }

    handlers_       = temp;
    alloc_handlers_ += 32;
  }

  handlers_[num_handlers_] = f;
  num_handlers_ ++;
}

//
// 'SharedImage::remove_handler()' - Remove a shared image handler.
//
void SharedImage::remove_handler(SharedImage::Handler  f) {
  int	i;				// Looping var...

  // First see if the handler has been added...
  for (i = 0; i < num_handlers_; i ++) {
    if (handlers_[i] == f) break;
  }

  if (i >= num_handlers_) return;

  // OK, remove the handler from the array...
  num_handlers_ --;

  if (i < num_handlers_) {
    // Shift later handlers down 1...
    memmove(handlers_ + i, handlers_ + i + 1,
           (num_handlers_ - i) * sizeof(SharedImage::Handler));
  }
}

//
// End of "$Id: SharedImage.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $"
//
