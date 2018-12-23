//
// "$Id: Fluid_Image.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $"
//
// Pixmap label support for the Fast Light Tool Kit (FLTK).
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

#include <fltk/run.h>
#include <fltk/Widget.h>
#include "FluidType.h"
#include "Fluid_Image.h"
#include <string.h>
#ifdef _WIN32
# define strcasecmp(a,b) stricmp(a,b)
# define strncasecmp(a,b,c) strnicmp(a,b,c)
#endif
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <fltk/filename.h>

////////////////////////////////////////////////////////////////

static uchar* store_datas_from_file(const char *filename, size_t &size)
{
  uchar *d=0;
  FILE *fd = fopen(filename, "rb");
  if(!fd) return 0;
  size_t c=512, cc=0, r;
  size = 0;
  do {
    c*=2;
    cc+=c;
    if(d)
      d=(uchar *) realloc(d, cc);
    else
      d=(uchar *) malloc(cc);
    r=fread(d+cc-c, 1, c, fd);
    size+=r;
  } while(r==c);
  if(size!=cc) d=(uchar *) realloc(d, size?size:1);
  return d;
}

////////////////////////////////////////////////////////////////

class generic_image : public Fluid_Image {
protected:
  fltk::SharedImage *p;
  int *linelength;
  fltk::ImageType* filetype;
public:
  generic_image(const char *name);
  ~generic_image();
  virtual const fltk::Symbol* symbol() {return p;}
  virtual void write_static();
  virtual void write_code();
  static int test_file(char *buffer);
};

int generic_image::test_file(char *buffer) {
  fltk::ImageType* ft = fltk::guess_image("", (uchar*)buffer);
  return ft->name != 0;
}

static int image_file_header_written;

#define MAX_CLINESIZE 256
void generic_image::write_static() {
  uchar* d=0;
  if (!p) return;
  if(image_file_header_written != write_number)
  {
    write_c("\n#include <fltk/SharedImage.h>\n");
    image_file_header_written = write_number;
  }
  if (inlined) {
    size_t l=0;
    if (filetype->name && !strcasecmp(filetype->name, "xpm")) {
      write_c("static const char *%s[] = {\n",
	      unique_id(this, "datas", fltk::filename_name(name()), 0));
      FILE* fp = fopen(name(), "rb");
      if(fp) {
	indentation += 2;
	char s[MAX_CLINESIZE+1];
	do {
	  if(fgets(s, MAX_CLINESIZE+1, fp)); //ignore the return value
	} while (!feof(fp) && !strchr(s, '{'));
	while (!feof(fp) && fgets(s, MAX_CLINESIZE+1, fp)) {
		write_c(indent());
	    write_craw(s);
	  //	  write_c(s);
	}
	indentation -= 2;
	fclose(fp);
      }
    } else {
      d = store_datas_from_file(name(), l);
      if(d) {
#if 1
	write_c("static const unsigned char %s[%d] = {\n", unique_id(this, "datas", fltk::filename_name(name()), 0), l);
	write_carray((const char*)d, l);
	write_c("};\n");
#else
	write_c("static const unsigned char %s[] =\n",
		unique_id(this, "datas", filename_name(name()), 0));
	write_cstring((const char*)d, l);
	write_c(";\n");
#endif
	free(d);
      }
    }
  }
}

void generic_image::write_code() {
  if (!p) return;
  if (0 && inlined) {
    write_c("%so->image(%s%s", indent(),
	(filetype->name && !strcasecmp(filetype->name, "xpm")) ? 
	"(const char*const *)" : "", unique_id(this, "datas", fltk::filename_name(name()), 0) );
  } else {
    write_c("%so->image(fltk::SharedImage::get(\"%s\"", indent(), name());
  }

  write_c("));\n");
    
}

generic_image::generic_image(const char *name ) : Fluid_Image(name) {
  filetype = fltk::guess_image(fltk::SharedImage::get_filename(name));
  p = filetype->get((char*) name, 0);
  inlined = 1;
}

generic_image::~generic_image() {
}

////////////////////////////////////////////////////////////////
#include <fltk/xbmImage.h>

class bitmap_image : public Fluid_Image {
  fltk::xbmImage *p;
public:
  ~bitmap_image();
  bitmap_image(const char *name, FILE *);
  virtual const fltk::Symbol* symbol() {return p;}
  virtual void write_static();
  virtual void write_code();
  static int test_file(char *buffer);
};

// bad test, always do this last!
int bitmap_image::test_file(char *buffer) {
  return (strstr(buffer,"#define ") != 0);
}

static int bitmap_header_written;

void bitmap_image::write_static() {
  if (!p) return;
  write_c("\n");
  if (bitmap_header_written != write_number) {
    write_c("#include <fltk/xbmImage.h>\n");
    bitmap_header_written = write_number;
  }
  int w = p->w();
  int h = p->h();
  int n = ((w+7)/8)*h;
#if 1 // older one
  write_c("static const unsigned char %s[%d] = {\n",
	  unique_id(this, "bits", fltk::filename_name(name()), 0), n);
  write_carray((const char*)(p->array), n);
  write_c("};\n");
#else // this seems to produce slightly shorter c++ files
  write_c("static const unsigned char %s[] =\n",
	  unique_id(this, "bits", fltk::filename_name(name()), 0));
  write_cstring((const char*)(p->array), n);
  write_c(";\n");
#endif
  write_c("static fltk::xbmImage %s(%s, %d, %d);\n",
	  unique_id(this, "xbmImage", fltk::filename_name(name()), 0),
	  unique_id(this, "bits", fltk::filename_name(name()), 0),
	  w, h);
}

void bitmap_image::write_code() {
  if (!p) return;
  write_c("%so->image(%s);\n", indent(),
	  unique_id(this, "xbmImage", fltk::filename_name(name()), 0));
}

#define ns_width 16
#define ns_height 16
static unsigned char ns_bits[] = {
   0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00};
static fltk::xbmImage nosuch_bitmap(ns_bits, ns_width, ns_height);

bitmap_image::bitmap_image(const char *name, FILE *f) : Fluid_Image(name) {
  p = &nosuch_bitmap; // if any problems with parse we exit with this
  if (!f) return;
  char buffer[1024];
  char junk[1024];
  int wh[2]; // width and height
  int i;
  for (i = 0; i<2; i++) {
    for (;;) {
      if (!fgets(buffer,1024,f)) return;
      int r = sscanf(buffer,"#define %s %d",junk,&wh[i]);
      if (r >= 2) break;
    }
  }
  // skip to data array:
  for (;;) {
    if (!fgets(buffer,1024,f)) return;
    if (!strncmp(buffer,"static ",7)) break;
  }
  int n = ((wh[0]+7)/8)*wh[1];
  uchar *data = new uchar[n];
  // read the data:
  i = 0;
  for (;i<n;) {
    if (!fgets(buffer,1024,f)) return;
    const char *a = buffer;
    while (*a && i<n) {
      int t;
      if (sscanf(a," 0x%x",&t)>0) data[i++] = t;
      while (*a && *a++ != ',');
    }
  }
  p = new fltk::xbmImage(data,wh[0],wh[1]);
}

bitmap_image::~bitmap_image() {
  if (p && p != &nosuch_bitmap) {
    delete p;
  }
}

////////////////////////////////////////////////////////////////

static Fluid_Image** images; // sorted list
static int numimages;
static int tablesize;

Fluid_Image* Fluid_Image::find(const char *name) {
  if (!name || !*name) return 0;

  // first search to see if it exists already:
  int a = 0;
  int b = numimages;
  while (a < b) {
    int c = (a+b)/2;
    int i = strcmp(name,images[c]->name_);
    if (i < 0) b = c;
    else if (i > 0) a = c+1;
    else return images[c];
  }

  // no, so now see if the file exists:

  Fluid_Image *ret = 0;

  const char* realname = fltk::SharedImage::get_filename(name);
  FILE *f = fopen(realname,"rb");

  if (!f) {
    read_error("%s : %s", realname, strerror(errno));
  } else {
    // now see if we can identify the type, by reading in some data
    // and asking all the types we know about:
    char buffer[1025];
    if(fread(buffer, 1, 1024, f)); // Ignore the return value
    rewind(f);
    buffer[1024] = 0; // null-terminate so strstr() works
    if (generic_image::test_file(buffer)) {
      ret = new generic_image(name);
    } else if (bitmap_image::test_file(buffer)) {
      ret = new bitmap_image(name,f);
    } else {
      ret = 0;
      read_error("%s : unrecognized image format", name);
    }
    fclose(f);
  }
  if (!ret) ret = new bitmap_image(name, 0);

  // make a new entry in the table:
  numimages++;
  if (numimages > tablesize) {
    tablesize = tablesize ? 2*tablesize : 16;
    images = (Fluid_Image**)realloc(images, tablesize*sizeof(Fluid_Image*));
  }
  for (b = numimages-1; b > a; b--) images[b] = images[b-1];
  images[a] = ret;

  return ret;
}

Fluid_Image::Fluid_Image(const char *name) {
  name_ = strdup(name);
  written = 0;
  refcount = 0;
  inlined=0;
}

void Fluid_Image::increment() {
  ++refcount;
}

void Fluid_Image::decrement() {
  --refcount;
  if (refcount > 0) return;
  delete this;
}

Fluid_Image::~Fluid_Image() {
  int a;
  for (a = 0; a<numimages; a++) if (images[a] == this) break;
  if(a<numimages) {
    numimages--;
    for (; a < numimages; a++) images[a] = images[a+1];
  }
  free((void*)name_);
}

////////////////////////////////////////////////////////////////

#include <fltk/file_chooser.h>

Fluid_Image *ui_find_image(Fluid_Image *old) {
  const char *name = fltk::file_chooser("Image", "*.{bm|xbm|xpm|gif|png|bmp|jpg|jpeg}",
				     old ? old->name() : 0, 0, 0);
  Fluid_Image *ret = (name && *name) ? Fluid_Image::find(name) : 0;
  return ret;
}

////////////////////////////////////////////////////////////////

static int cancel, modal;
void browse_dir_cb();

#include "image_file_panel.h"
#include "image_file_panel.cxx"

const char *images_dir = 0;
extern void fix_images_dir();

void browse_dir_cb()
{
  const char *f = fltk::file_chooser("Images directory","",
				     images_dir_input->value(), 0, 0);
  if (f) images_dir_input->value(f);
}

void set_images_dir_cb(fltk::Widget *, void *) {
  if(!images_dir_window) make_images_dir_window();
  images_dir_input->value(images_dir);
  images_dir_window->show();
  cancel=0; modal=1;
  while(modal) fltk::wait();
  if (!cancel) {
    images_dir = images_dir_input->value();
    if (!*images_dir) images_dir = 0;
    fix_images_dir();
  }
}
 
//
// End of "$Id: Fluid_Image.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $".
//
