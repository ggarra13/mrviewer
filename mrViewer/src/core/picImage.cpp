
/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   picImage.cpp
 * @author gga
 * @date   Fri Sep 21 01:28:25 2007
 * 
 * @brief  Image reader for Softimage pic files.
 * 
 * 
 */

#include <iostream>
#include <limits>


#if defined(WIN32) || defined(WIN64)
#include <windows.h>  // for htonl, etc.
#undef max
#else
#include <netinet/in.h>
#endif

#include <fltk/run.h>  // for fltk_fopen

#include "picImage.h"
#include "byteSwap.h"

#include "gui/mrvIO.h"



// ===============================================
// = #DEFINES ===================================
// =============================================

// Data type
#define PIC_UNSIGNED_INTEGER	0x00
#define PIC_SIGNED_INTEGER		0x10	// XXX: Not implemented
#define PIC_SIGNED_FLOAT		0x20	// XXX: Not implemented


// Compression type
#define PIC_UNCOMPRESSED		0x00
#define PIC_PURE_RUN_LENGTH		0x01
#define PIC_MIXED_RUN_LENGTH	0x02

// Channel types (OR'd)
#define PIC_RED_CHANNEL			0x80
#define PIC_GREEN_CHANNEL		0x40
#define PIC_BLUE_CHANNEL		0x20
#define PIC_ALPHA_CHANNEL		0x10

#define PIC_SHADOW_CHANNEL		0x08	// XXX: Not implemented
#define PIC_DEPTH_CHANNEL		0x04	// XXX: Not implemented
#define PIC_AUXILIARY_1_CHANNEL	0x02	// XXX: Not implemented
#define PIC_AUXILIARY_2_CHANNEL	0x01	// XXX: Not implemented

// ===============================================
// = TYPEDEFS ===================================
// =============================================


namespace {

static const char* kModule = "pic";


// ===============================================
// = DECLARATIONS ===============================
// =============================================



uint32_t readInt(FILE *file)
{
    uint32_t	v;
    uint8_t	c;
	
    v = 0;
    c = fgetc(file);
    v |= (c << 24);
    c = fgetc(file);
    v |= (c << 16);
    c = fgetc(file);
    v |= (c << 8);
    c = fgetc(file);
    v |= c;
	
    return v;
}

uint32_t readShort(FILE *file)
{
    int32_t	v;
    uint8_t	c;

    v = 0;
    c = fgetc(file);
    v |= (c << 8);
    c = fgetc(file);
    v |= c;
	
    return v;
}

void writeInt(FILE *file, uint32_t v)
{
    fputc(((v >> 24) & 0xFF), file);
    fputc(((v >> 16) & 0xFF), file);
    fputc(((v >> 8) & 0xFF), file);
    fputc((v & 0xFF), file);
}

void writeShort(FILE *file, uint32_t v)
{
    fputc((v >> 8) & 0xFF, file);
    fputc(v & 0xFF, file);
}

char *readStr(FILE *file)
{
    char	*result;
    int		size, med, c;
	
    med = size = 0;
    result = (char*)malloc(16);
	
    c = fgetc(file);
	
    if(c == EOF)
        return NULL;
	
    while(!(c == EOF || c == '\n' || c == '\0')) {
        if(size == med) {
            med += 64;
            result = (char*)realloc(result, med);
        }
        result[size++] = (char)c;
        c = fgetc(file);
    }
	
    // Trim allocated buffer to contain only the string and terminating '\0'
    result = (char*)realloc(result, size + 1);

    result[size] = '\0';
	
    return result;
}

} // namespace


namespace mrv {


  using namespace std;

const char* picImage::kCompression[] = {
"None",
"RLE",
"Mixed"
};

  /** 
   * Constructor
   * 
   */
  picImage::picImage() :
    CMedia()
  {
  }



  /** 
   * Destructor
   * 
   */
  picImage::~picImage()
  {
  }



  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .map file. This returns true if the 
    data contains PIC's magic number (0x10) and a short of 1 in the
    as the number of components.
  */
  bool picImage::test(const boost::uint8_t *data, unsigned len)
  {
    if (!data || len < 7) return false;
    
    uint32_t magic = ntohl(((uint32_t*)data)[0]);
    if(magic != 0x5380F634) // 'S' + 845-1636 (SI's phone no in LE :-)
      return false;
    return true;
  }




  /** 
   * Fetch the shadow map image
   * 
   * 
   * @return true on success, false if not
   */
  bool picImage::fetch(const boost::int64_t frame) 
  {
      uint32_t		tmp, status;
      bool              has_rgb = false;
      bool		alpha = false;
      uint8_t		chained;
      Channel		*chan = NULL;
      uint32_t          dw, dh;

      FILE* file = fltk::fltk_fopen( sequence_filename(frame).c_str(), "rb" );

      tmp = readInt(file);
      if(tmp != 0x5380F634) {		// 'S' + 845-1636 (SI's phone no :-)
          IMG_ERROR( "has invalid magic number in the header." );
          return false;
      }
        
      tmp = readInt(file);
      
      char buf[81];
      buf[81] = 0;
      tmp = fread(buf, 80, 1, file );
      _iptc.insert( std::make_pair( "Creator", buf ) );
    
      tmp = readInt(file);		// File identifier 'PICT'
      /* pdb - if(tmp != 'PICT') { */
      if (tmp != 0x50494354) {
          IMG_ERROR( "is a Softimage file, but not a PIC file." );
          return false;
      }
   
      dw = readShort(file);
      dh = readShort(file);
    
      readInt(file);			// Aspect ratio (ignored)
      readShort(file);		// Interlace type (ignored)
      readShort(file);		// Read padding
      
      image_size( dw, dh );
    allocate_pixels(frame);

    // Read channels
    _layers.clear();
    
    do {
        Channel	*c;
        
        if(chan == NULL)
            chan = c = (Channel*) malloc(sizeof(Channel));
        else {
            c->next = (Channel*) malloc(sizeof(Channel));
            c = c->next;
        }
        c->next = NULL;
	
        chained = fgetc(file);
        c->size = fgetc(file);
        c->type = fgetc(file);
        c->channels = fgetc(file);
	
        // See if we have an alpha channel in there
        if(c->channels & PIC_ALPHA_CHANNEL)
        {
            alpha = true;
        }
        
        // See if we have a red, green or blue channel in there
        if( (c->channels & PIC_RED_CHANNEL) ||
            (c->channels & PIC_GREEN_CHANNEL) ||
            (c->channels & PIC_BLUE_CHANNEL) )
        {
            has_rgb = true;
        }
        
    } while(chained);

    image_type::PixelType pixel_type = image_type::kByte;
    
    if ( has_rgb )
        rgb_layers();
    
    if ( alpha )
        alpha_layers();

    _pixel_ratio = 1.0;
    _gamma = 1.0f;
    image_size( dw, dh );

    // Read pixel values
    allocate_pixels( frame, 4, image_type::kRGBA, pixel_type, dw, dh );
    
    if(!readScanlines(file, (uint32_t*)_hires->data().get(), dw, dh,
                      chan, alpha))
        return false;

noerror:
    
    while(chan) {
        Channel		*prev;
        prev = chan;
        chan = chan->next;
        free(prev);
    }

    fclose(file);
    return true;
  }



bool picImage::readScanlines(FILE *file, uint32_t *image,
                             int32_t width, int32_t height, 
                             Channel *channel, uint32_t alpha)
{
    int32_t		i;
    for(i = 0; i < height; ++i) {
        uint32_t	*scan = image + i * width;
        if(!readScanline(file, (uint8_t *)scan, width, channel, 4))
            return false;
    }
    return true;
}

bool picImage::readScanline(FILE *file, uint8_t *scan, int32_t width, Channel *chan,  int32_t bytes)
{
    bool status;
    int32_t		noCol;
    int32_t		off[4];
	
    while(chan) {
        noCol = 0;
        if(chan->channels & PIC_RED_CHANNEL) {
            off[noCol] = 0;
            noCol++;
        }
        if(chan->channels & PIC_GREEN_CHANNEL) {
            off[noCol] = 1;
            noCol++;
        }
        if(chan->channels & PIC_BLUE_CHANNEL) {
            off[noCol] = 2;
            noCol++;
        }
        if(chan->channels & PIC_ALPHA_CHANNEL) {
            off[noCol] = 3;
            noCol++;
        }

        switch(chan->type & 0x0F) {
            case PIC_UNCOMPRESSED:
                _compression = kNone;
                status = channelReadRaw(file, scan, width, noCol, off, bytes);
                break;
            case PIC_PURE_RUN_LENGTH:
                _compression = kRLE;
                status = channelReadPure(file, scan, width, noCol, off, bytes);
                break;
            case PIC_MIXED_RUN_LENGTH:
                _compression = kMixed;
                status = channelReadMixed(file, scan, width, noCol, off, bytes);
                break;
        }
        if(!status)
            break;
		
        chan = chan->next;
    }
    return status;
}

bool picImage::channelReadRaw(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes)
{
    int			i, j;
	
    for(i = 0; i < width; i++) {
        if(feof(file))
            return false;
        for(j = 0; j < noCol; j++)
            scan[off[j]] = (uint8_t)getc(file);
        scan += bytes;
    }
    return true;
}

bool picImage::channelReadPure(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes)
{
    uint8_t		col[4];
    int32_t		count;
    int			i, j, k;
	
    for(i = width; i > 0; ) {
        count = (unsigned char)getc(file);
        if(count > width)
            count = width;
        i -= count;
		
        if(feof(file))
            return false;
		
        for(j = 0; j < noCol; j++)
            col[j] = (uint8_t)getc(file);
		
        for(k = 0; k < count; k++, scan += bytes) {
            for(j = 0; j < noCol; j++)
                scan[off[j] + k] = col[j];
        }
    }
    return true;
}

bool picImage::channelReadMixed(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes)
{
    int32_t	count;
    int		i, j, k;
    uint8_t	col[4];
	
    for(i = 0; i < width; i += count) {
        if(feof(file))
            return false;
		
        count = (uint8_t)fgetc(file);
		
        if(count >= 128) {		// Repeated sequence
            if(count == 128)	// Long run
                count = readShort(file);
            else
                count -= 127;
			
            // We've run past...
            if((i + count) > width) {
                IMG_ERROR( "Overrun scanline (Repeat) [" << i
                           << " + " << count << " > "
                           << width << " (NC=" << noCol << ")" );
                return false;
            }
			
            for(j = 0; j < noCol; j++)
                col[j] = (uint8_t)fgetc(file);
			
            for(k = 0; k < count; k++, scan += bytes) {
                for(j = 0; j < noCol; j++)
                    scan[off[j]] = col[j];
            }
        } else {				// Raw sequence
            count++;
            if((i + count) > width) {
                IMG_ERROR( "Overrun scanline (Raw) [" << i
                           << " + " << count << " > " << width
                           << "] (NC=" << noCol << ")" );
                return false;
            }
                    
            for(k = count; k > 0; k--, scan += bytes) {
                for(j = 0; j < noCol; j++)
                    scan[off[j]] = (uint8_t)fgetc(file);
            }
        }
    }
    return true;
} 

} // namespace mrv
