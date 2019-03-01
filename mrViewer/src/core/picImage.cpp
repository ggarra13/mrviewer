
/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#include <cstdio>
#define __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <cmath>
#ifdef _WIN32
#   include <float.h>
#   define isfinite(x) _finite(x)
#   undef max
#else
#  include <netinet/in.h>
#endif

#include <iostream>
#include <limits>



#include <FL/Fl.H>  // for fl_fopen

#include <ImfStringAttribute.h>

#include "core/mrvColorOps.h"
#include "core/mrvMath.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvIO.h"
#include "mrViewer.h"


#include "picImage.h"
#include "byteSwap.h"




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
bool picImage::fetch( mrv::image_type_ptr& canvas, const boost::int64_t frame)
{
    uint32_t		tmp;
    bool              has_rgb = false;
    bool		alpha = false;
    uint8_t		chained;
    Channel		*chan = NULL;
    uint32_t          dw, dh;

    FILE* file = fl_fopen( sequence_filename(frame).c_str(), "rb" );

    tmp = readInt(file);
    if(tmp != 0x5380F634) {		// 'S' + 845-1636 (SI's phone no :-)
        IMG_ERROR( _("has invalid magic number in the header.") );
        fclose(file);
        return false;
    }

    tmp = readInt(file);

    char buf[81];
    buf[80] = 0;
    size_t read = fread(buf, 80, 1, file );
    if ( buf[0] != 0 )
    {
        Imf::StringAttribute attr( buf );
        _attrs.insert( std::make_pair( "Creator", attr.copy() ) );
    }

    tmp = readInt(file);		// File identifier 'PICT'
    /* pdb - if(tmp != 'PICT') { */
    if (tmp != 0x50494354) {
        IMG_ERROR( _("is a Softimage file, but not a PIC file.") );
        fclose(file);
        return false;
    }


    dw = readShort(file);
    dh = readShort(file);

    readInt(file);			// Aspect ratio (ignored)
    readShort(file);		// Interlace type (ignored)
    readShort(file);		// Read padding


    // Read channels

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

    if ( _layers.empty() )
    {
        if ( has_rgb )
            rgb_layers();
        if ( alpha )
            alpha_layers();
    }

    _pixel_ratio = 1.0;
    _gamma = 1.0f;

    // Read pixel values
    image_size( dw, dh );
    allocate_pixels( canvas, frame, 4, image_type::kRGBA, pixel_type, dw, dh );

    bool ok = readScanlines(file, (uint32_t*)canvas->data().get(), dw, dh,
                            chan, alpha);
    while(chan) {
        Channel		*prev;
        prev = chan;
        chan = chan->next;
        free(prev);
    }

    fclose(file);

    return ok;
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
                IMG_ERROR( _("Overrun scanline (Repeat) [") << i
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
                IMG_ERROR( _("Overrun scanline (Raw) [") << i
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





static bool ff_pic_writeScanline(FILE *file, uint32_t *line, uint32_t width)
{
    int		same, seqSame, count;
    unsigned    k;
    int         i;
    uint8_t	pixel[128][3], col[3];

    count = 0;
    for(k = 0; k < width; k++) {
        col[0] = (line[k]) & 0xFF;
        col[1] = (line[k] >> 8) & 0xFF;
        col[2] = (line[k] >> 16) & 0xFF;

        if(count == 0) {
            pixel[0][0] = col[0];
            pixel[0][1] = col[1];
            pixel[0][2] = col[2];
            count++;
        } else if(count == 1) {
            seqSame  = (col[0] == pixel[0][0]);
            seqSame &= (col[1] == pixel[0][1]);
            seqSame &= (col[2] == pixel[0][2]);

            if(!seqSame) {
                pixel[count][0] = col[0];
                pixel[count][1] = col[1];
                pixel[count][2] = col[2];
            }
            count++;
        } else if(count > 1) {
            if(seqSame) {
                same  = (col[0] == pixel[0][0]);
                same &= (col[1] == pixel[0][1]);
                same &= (col[2] == pixel[0][2]);
            } else {
                same  = (col[0] == pixel[count - 1][0]);
                same &= (col[1] == pixel[count - 1][1]);
                same &= (col[2] == pixel[count - 1][2]);
            }

            if(same ^ seqSame) {
                if(!seqSame) {
                    putc((uint8_t)(count - 2), file);
                    for(i = 0; i < count - 1; i++) {
                        putc(pixel[i][0], file);
                        putc(pixel[i][1], file);
                        putc(pixel[i][2], file);
                    }
                    pixel[0][0] = pixel[1][0] = col[0];
                    pixel[0][1] = pixel[1][1] = col[1];
                    pixel[0][2] = pixel[1][2] = col[2];
                    count = 2;
                    seqSame = true;
                } else {
                    if(count < 128)
                        putc((uint8_t)(count + 127), file);
                    else {
                        putc(128, file);
                        writeShort(file, count);
                    }
                    putc(pixel[0][0], file);
                    putc(pixel[0][1], file);
                    putc(pixel[0][2], file);
                    pixel[0][0] = col[0];
                    pixel[0][1] = col[1];
                    pixel[0][2] = col[2];
                    count = 1;
                }
            } else {
                if(!same) {
                    pixel[count][0] = col[0];
                    pixel[count][1] = col[1];
                    pixel[count][2] = col[2];
                }
                count++;
                if((count == 128) && !seqSame) {
                    putc(127, file);
                    for(i = 0; i < count; i++) {
                        putc(pixel[i][0], file);
                        putc(pixel[i][1], file);
                        putc(pixel[i][2], file);
                    }
                    count = 0;
                }
                if((count == 65536) && seqSame) {
                    putc(128, file);
                    writeShort(file, count);
                    putc(pixel[0][0], file);
                    putc(pixel[0][1], file);
                    putc(pixel[0][2], file);
                    count = 0;
                }
            }
        }
        if(ferror(file))
            return false;
    }
    if(count) {
        if((count == 1) || (!seqSame)) {
            putc((uint8_t)(count - 1), file);
            for(i = 0; i < count; i++) {
                putc(pixel[i][0], file);
                putc(pixel[i][1], file);
                putc(pixel[i][2], file);
            }
        } else {
            if(count < 128)
                putc((uint8_t)(count + 127), file);
            else {
                putc(128, file);
                writeShort(file, count);
            }
            putc(pixel[0][0], file);
            putc(pixel[0][1], file);
            putc(pixel[0][2], file);
        }
        if(ferror(file))
            return false;
    }

    count = 0;
    for(k = 0; k < width; k++) {
        col[0] = (line[k] >> 24) & 0xFF;

        if(count == 0) {
            pixel[0][0] = col[0];
            count++;
        } else if(count == 1) {
            seqSame  = (col[0] == pixel[0][0]);

            if(!seqSame) {
                pixel[count][0] = col[0];
            }
            count++;
        } else if(count > 1) {
            if(seqSame) {
                same  = (col[0] == pixel[0][0]);
            } else {
                same  = (col[0] == pixel[count - 1][0]);
            }

            if(same ^ seqSame) {
                if(!seqSame) {
                    putc((uint8_t)(count - 2), file);
                    for(i = 0; i < count - 1; i++) {
                        putc(pixel[i][0], file);
                    }
                    pixel[0][0] = pixel[1][0] = col[0];
                    count = 2;
                    seqSame = true;
                } else {
                    if(count < 128)
                        putc((uint8_t)(count + 127), file);
                    else {
                        putc(128, file);
                        writeShort(file, count);
                    }
                    putc(pixel[0][0], file);
                    pixel[0][0] = col[0];
                    count = 1;
                }
            } else {
                if(!same) {
                    pixel[count][0] = col[0];
                }
                count++;
                if((count == 128) && !seqSame) {
                    putc(127, file);
                    for(i = 0; i < count; i++) {
                        putc(pixel[i][0], file);
                    }
                    count = 0;
                }
                if((count == 65536) && seqSame) {
                    putc(128, file);
                    writeShort(file, count);
                    putc(pixel[0][0], file);
                    count = 0;
                }
            }
        }
        if(ferror(file))
            return false;
    }
    if(count) {
        if((count == 1) || (!seqSame)) {
            putc((uint8_t)(count - 1), file);
            for(i = 0; i < count; i++) {
                putc(pixel[i][0], file);
            }
        } else {
            if(count < 128)
                putc((uint8_t)(count + 127), file);
            else {
                putc(128, file);
                writeShort(file, count);
            }
            putc(pixel[0][0], file);
        }
        if(ferror(file))
            return false;
    }

    return true;
}

bool picImage::save( const char* path, const CMedia* img,
                     const ImageOpts* opts )
{
    FILE	*file;
    char	str[80], myPath[4096];
    unsigned  line;

    strcpy(myPath, path);
    if(strlen(myPath) < 4 ||
            strcasecmp(myPath + strlen(myPath) - 4, ".pic") != 0)
        strcat(myPath, ".pic");

    if((file = fl_fopen(myPath, "wb")) == NULL) {
        LOG_ERROR( _("Couldn't open '") << myPath <<
                   _("' for writing. Reason: " ) << strerror(errno));
        return false;
    }

    // Write Softimage file header
    writeInt(file, 0x5380F634);
    writeInt(file, 0x406001A3);
    memset(str, 0, 80);
    sprintf(str, "File written by mrViewer.");
    fwrite(str, 1, 80, file);

    mrv::image_type_ptr pic = img->hires();

    unsigned dw = pic->width();
    unsigned dh = pic->height();


    // Write picture file header
    writeInt(file, 0x50494354);
    writeShort(file, dw);
    writeShort(file, dh);
    writeInt(file, 0x3F800000);
    writeShort(file, 3);
    writeShort(file, 0);

    // Info for RGB stream.
    fputc(1, file);
    fputc(8, file);
    fputc(2, file);
    fputc(0xE0, file);

    // Info for alpha channel stream
    fputc(0, file);
    fputc(8, file);
    fputc(2, file);
    fputc(0x10, file);

    if(ferror(file)) {
        LOG_ERROR( _("Couldn't write out to '") << path
                   << _("'. Reason: ") << strerror(errno) );
        fclose(file);
        return false;
    }

    bool must_convert = false;


    bool  has_alpha = pic->has_alpha();
    image_type::Format format = pic->format();

    if ( ( format != image_type::kRGBA ) ||
            pic->pixel_type() != image_type::kByte ||
            img->gamma() != 1.0f )
        must_convert = true;

    if ( Preferences::use_ocio && Preferences::uiMain->uiView->use_lut() )
        must_convert = true;

    if ( must_convert )
        prepare_image( pic, img, image_type::kRGBA, image_type::kByte );

    uint8_t* pixels = (boost::uint8_t*)pic->data().get();

    for(line = 0; line < dh; ++line) {
        uint32_t		*linePtr;
        linePtr = (uint32_t*) pixels;
        linePtr += line * dw;

        if(!ff_pic_writeScanline(file, linePtr, dw)) {
            LOG_ERROR( _("Couldn't write out to '") << path
                       << _("'. Reason: " ) << strerror(errno) );
        }
    }


    fclose(file);
    return true;
}


} // namespace mrv
