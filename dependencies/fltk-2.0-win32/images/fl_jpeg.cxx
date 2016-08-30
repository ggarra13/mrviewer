//
// "$Id: fl_jpeg.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// JPEG reading code for the Fast Light Tool Kit (FLTK).
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

// JPEG image decompression. From example code in libjpeg distribution.

#include <config.h>
#include <fltk/SharedImage.h>

#if HAVE_LIBJPEG

#include <stdio.h>
extern "C" {
# ifdef HAVE_LOCAL_JPEG_H
#  include "libjpeg/jpeglib.h"
# else
#  include <jpeglib.h>
# endif
}
#include <setjmp.h>
#include <string.h>

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  JOCTET * buffer;		/* start of buffer */

  JOCTET * start_datas, * datas;
} my_source_mgr;

static int INPUT_BUF_SIZE;

typedef my_source_mgr * my_src_ptr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->datas = src->start_datas;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  unsigned nbytes = INPUT_BUF_SIZE;

  memcpy(src->buffer, src->datas, INPUT_BUF_SIZE);
  src->datas += INPUT_BUF_SIZE;

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (unsigned) num_bytes;
    src->pub.bytes_in_buffer -= (unsigned) num_bytes;
  }
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr /*cinfo*/)
{
  /* no work necessary here */
}


/*
 * Prepare for input from memory.
 */

METHODDEF(void)
jpeg_rawdatas_src (j_decompress_ptr cinfo, JOCTET * datas)
{
  my_src_ptr src;

  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->start_datas = datas;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
output_message (j_common_ptr /*cinfo*/)
{
}

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

#endif

#if HAVE_LIBJPEG
// Dummy function to remove gcc's nasty warning about longjmp:
static void declare_now(void*) { }
#endif

using namespace fltk;

bool jpegImage::fetch()
{
#if HAVE_LIBJPEG
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  FILE* infile = 0;
  declare_now(&infile);

  INPUT_BUF_SIZE = 4096;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  jerr.pub.output_message = output_message;

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    if (infile) fclose(infile);
    return false;
  }

  jpeg_create_decompress(&cinfo);

  if (datas) {
    jpeg_rawdatas_src(&cinfo, (JOCTET*) datas);
  } else {
    if ((infile = fopen(get_filename(), "rb")) == NULL)
      return false;
    jpeg_stdio_src(&cinfo, infile);
  }

  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  setsize(cinfo.output_width, cinfo.output_height);
  setpixeltype((PixelType)cinfo.output_components);

  for (int y=0; y<height(); y++) {
    uchar* b = linebuffer(y);
    jpeg_read_scanlines(&cinfo, (JSAMPLE**)&b, 1);
    setpixels(b, y);
  }

  jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  if (infile) fclose(infile);
  return true;
#else
  return false;
#endif
}

bool jpegImage::test(const uchar* block, unsigned size)
{
// I don't know what the official test for jpeg is. There appears to
// be some complexity at the start, probably they assumme you will use
// a jpeg_decompress object. These 3 bytes are the only constant between
// all the example files I have seen:
  return block[0] == 0xff && block[1] == 0xd8 && block[2] == 0xff;
//   return (!strcmp((char*)block+6, "JFIF") ||
//	  !strcmp((char*)block+6, "Exif"));
}

//
// End of "$Id: fl_jpeg.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
