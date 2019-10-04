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
 * @file   iffImage.cpp
 * @author gga
 * @date   Thu Jul 19 13:02:54 2007
 *
 * @brief  Class used to read Maya IFF files
 *
 *
 */

#include <iostream>
#include <limits>

#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#include <windows.h>  // for htonl, etc.
#undef max
#else
#include <netinet/in.h>
#endif

extern "C" {
#include <libavutil/avassert.h>
}

#include <FL/fl_utf8.h>
#include <FL/Fl.H>

#include "iffImage.h"
#include "byteSwap.h"
#include "mrvIO.h"
#include "core/mrvI8N.h"
#include "mrvThread.h"

namespace
{
const char* kModule = "iff";
}


namespace mrv {

const char* iffImage::kCompression[] = {
    _("None"),
    _("RLE"),
};

#define MAKE_TAG(a,b,c,d) (unsigned int) ( (unsigned int)(a << 24)	\
					   + (unsigned int)(b << 16)	\
					   + (unsigned int)(c << 8)	\
					   + (unsigned int)d)

static const unsigned int kFORM_TAG = MAKE_TAG('F','O','R','M');
static const unsigned int kCAT_TAG  = MAKE_TAG('C','A','T',' ');


// Common IFF tags
static const unsigned int kCIMG_TAG = MAKE_TAG('C','I','M','G');
static const unsigned int kRGBA_TAG = MAKE_TAG('R','G','B','A');

// Amiga IFF tags
static const unsigned int kBMHD_TAG = MAKE_TAG('B','M','H','D');  // Amiga IFF header info
static const unsigned int kILBM_TAG = MAKE_TAG('I','L','B','M');

// Maya IFF tags
static const unsigned int kFOR4_TAG = MAKE_TAG('F','O','R','4');
static const unsigned int kIFF_MAGIC = kFOR4_TAG;
static const unsigned int kTBHD_TAG = MAKE_TAG('T','B','H','D');  // Maya  IFF header info
static const unsigned int kAUTH_TAG = MAKE_TAG('A','U','T','H');
static const unsigned int kDATE_TAG = MAKE_TAG('D','A','T','E');
static const unsigned int kCLPZ_TAG = MAKE_TAG('C','L','P','Z');  // clipping planes, two floats
static const unsigned int kESXY_TAG = MAKE_TAG('E','S','X','Y');  // eye x-y ratios
static const unsigned int kHIST_TAG = MAKE_TAG('H','I','S','T');
static const unsigned int kVERS_TAG = MAKE_TAG('V','E','R','S');
static const unsigned int kTBMP_TAG = MAKE_TAG('T','B','M','P');
static const unsigned int kZBUF_TAG = MAKE_TAG('Z','B','U','F');

/// Ignored tags
static const unsigned int kPICS_TAG = MAKE_TAG('P','I','C','S');
static const unsigned int kTEXT_TAG = MAKE_TAG('T','E','X','T');
static const unsigned int kPROP_TAG = MAKE_TAG('P','R','O','P');
static const unsigned int kLIST_TAG = MAKE_TAG('L','I','S','T');

enum Flags
{
    kHasRGB = 1,
    kHasAlpha = 2,
    kHasZ = 4,
    kHas12Bit = 8192,
};


struct iffHeader
{
    unsigned int width;
    unsigned int height;
    unsigned short prnum;
    unsigned short prden;
    unsigned int flags;
    unsigned short bytes;
    unsigned short tiles;
    unsigned int compress;
    int orgx, orgy;

    int depth;

    iffHeader() :
        orgx( 0 ),
        orgy( 0 )
    {
    }

    void swap()
    {
        width  = ntohl( width );
        height = ntohl( height );
        prnum  = ntohs( prnum );
        prden  = ntohs( prden );
        flags  = ntohl( flags );
        bytes  = ntohs( bytes );
        tiles  = ntohs( tiles );
        compress = ntohl( compress );
        orgx   = ntohl( orgx );
        orgy   = ntohl( orgy );

        depth = 3 * ((flags & kHasRGB) != 0) + 1 * ((flags & kHasAlpha) != 0 );

#ifdef DEBUG_IFF
        using namespace std;
        cerr << "width: " << width << endl;
        cerr << "height: " << height << endl;
        cerr << "prnum: " << prnum << endl;
        cerr << "prden: " << prden << endl;
        cerr << "flags: " << flags << endl;
        cerr << "bytes: " << bytes << endl;
        cerr << "tiles: " << tiles << endl;
        cerr << "compress: " << compress << endl;
        cerr << "orgx: " << orgx << endl;
        cerr << "orgy: " << orgy << endl;
#endif
    }
};

struct iffChunk
{
    unsigned int tag;
    unsigned int size;

    void swap()
    {
        tag   = ntohl( tag );
        size  = ntohl( size );
    }
};


struct iffPixelBlock
{
    unsigned short x1, y1, x2, y2;
    void swap()
    {
        x1 = ntohs( x1 );
        x2 = ntohs( x2 );
        y1 = ntohs( y1 );
        y2 = ntohs( y2 );
    }
};



using namespace std;


iffImage::iffImage() :
    CMedia()
{
}

iffImage::~iffImage()
{
}


/*! Test a block of data read from the start of the file to see if it
  looks like the start of an .iff file. This returns true if the
  data contains IFF's magic number (0x10) and a short of 1 in the
  as the number of components.
*/
bool iffImage::test(const boost::uint8_t *data, unsigned len)
{
    iffChunk* header = (iffChunk*) data;
    if ( ntohl( header->tag ) != kIFF_MAGIC )
        return false;

    return true;
}





void iffImage::end_read_chunk( FILE* f, iffChunk& chunk )
{
    int size = chunk.size % 4;
    if ( size == 0 ) return;
    fseek( f, 4-size, SEEK_CUR );
}

void iffImage::read_chunk( FILE* f, iffChunk& chunk )
{
    size_t r = fread( &chunk, sizeof(iffChunk), 1, f );
    if ( r == 0 ) return;
    chunk.swap();
}

void iffImage::read_uncompressed_tile( FILE* file,
				       mrv::image_type_ptr& canvas,
				       boost::uint8_t* src,
                                       const unsigned int compsize,
                                       const unsigned x1, const unsigned y1,
                                       const unsigned w,
                                       const unsigned h,
                                       const short depth, const short bytes,
                                       const bool z )
{
    size_t r = fread( src, compsize, 1, file );

    unsigned int dw = width();
    unsigned int dh = height() - 1;

    Pixel* pixels = (Pixel*)canvas->data().get();

    for ( unsigned y = 0; y < h; ++y )
    {
        unsigned offset = (dh - (y + y1)) * dw + x1;
        Pixel* p = pixels + offset;

        for ( unsigned x = 0; x < w; ++x, ++p )
        {
            for ( short c = 0; c < depth; ++c, ++src )
            {
                boost::uint8_t t[4];
                boost::uint8_t* d = src;
                for ( short b = 0; b < bytes; ++b )
                {
                    t[bytes-b-1] = *d;
                    d += w * h * depth;
                }

                float v = 0.f;
                switch( bytes )
                {
                case 1:
                    v = (*src) / 255.0f;
                    break;
                case 2:
                {
                    unsigned short val = ( (t[1] << 8) +
                                           (t[0] << 0) );
                    v = (float)val / 65535.0f;
                    break;
                }
                case 4:
                {
                    v = *((float*)t);
                    break;
                }
                }

                float* rgba = (float*) p;
                rgba[3-c] = v;
            }
        }
    }
}



static void decompress_rle(uint8_t* data, uint32_t delta, uint32_t numBytes,
                           uint8_t* compressedData,
                           uint32_t compressedDataSize,
                           uint32_t* compressedIndex )
{
#ifdef __IFF_DEBUG_
    LOG_INFO( "Decompressing data " <<  numBytes );
#endif

    uint32_t FROM = *compressedIndex;
    uint32_t TO = 0;

    while (TO < numBytes) {

        if (FROM >= compressedDataSize) {
            LOG_ERROR( _("Not enough compressed data") );
            while (TO < numBytes)
                data[delta * TO++] = 0;
            break;
        }

        uint8_t nextChar = compressedData[FROM++];
        unsigned count = (nextChar & 0x7f) + 1;

        if ( ( TO + count ) > numBytes ) {
            LOG_ERROR( _("Count bad, ") << TO << "+" << count << " > " << numBytes);
            count = numBytes - TO;
            //       printf("Start at %d of %d: ", *compressedIndex, compressedDataSize);
            //       for (uint32_t i = *compressedIndex; i <= FROM;) {
            //     uint8_t n = compressedData[i++];
            //     printf("%02x ", n);
            //     if (n & 0x80) i++;
            //     else {i += (n&0x7f) + 1;}
            //       }
            //       printf("\n");
        }

        if ( nextChar & 0x80 ) {

            // We have a duplication run

            nextChar = compressedData[FROM++];
            for (uint32_t i = 0; i < count; ++i )
                data[delta * TO++] = nextChar;

        }
        else {

            // We have a verbatim run
            for (uint32_t i = 0; i < count; ++i )
                data[delta * TO++] = compressedData[FROM++];
        }
    }

    *compressedIndex = FROM;
}


static uint8_t* read_tile(FILE* file, int size, int depth, int datasize, int* offsets)
{
    uint8_t* result = (uint8_t*)malloc( size * depth );
    if (datasize >= size * depth) {
        size_t r = fread(result, 1, size * depth, file);
	if ( r < datasize )
	{
	    LOG_ERROR( "Corrupt tile in iff file" );
	}
    }
    else {
        // compressed tile
        uint8_t* data = (uint8_t*)malloc(datasize);
        datasize = int(fread(data, 1, datasize, file));
        uint32_t index = 0;
        for (int i = 0; i < depth; i++)
            decompress_rle(result + offsets[i], depth, size,
                           data, datasize, &index);
        free(data);
    }
    return result;
}



void iffImage::read_pixel_chunk( FILE* file,
				 image_type_ptr& canvas,
                                 const int depth, const int bytes,
                                 const iffChunk& chunk )
{
    iffPixelBlock block;
    size_t r = fread( &block, sizeof(iffPixelBlock), 1, file );
    block.swap();
    int x1 = block.x1;
    int y1 = block.y1;
    unsigned tile_width = (block.x2 - block.x1)+1;
    unsigned tile_height = (block.y2 - block.y1)+1;
    unsigned compsize = chunk.size - sizeof(iffPixelBlock);

    if ( _channel && strcmp( _channel, N_("Z") ) == 0 )
    {
        if ( chunk.tag == kZBUF_TAG )
        {
            static int offsets[] = {
                0, 1, 2, 3
            };
            float* tileData = (float*)read_tile(file,
                                                tile_width * tile_height, 4,
                                                compsize, offsets);

            if ( tileData ) {

                uint32_t* t = (uint32_t*)tileData;
                for ( int i = 0; i < tile_width*tile_height; ++i )
                {
                    t[i] = ntohl( t[i] );
                }

                unsigned w = width();
                unsigned h = height() - 1;
                float* buffer = (float*)canvas->data().get();

                unsigned base = y1 * w + x1;
                for (int i = 0; i < tile_height; i++) {
                    unsigned iw = i * w;
                    unsigned it = i * tile_width;
                    for (int j = 0; j < tile_width; j++) {
                        buffer[base + iw + j] = -tileData[it + j];
                    } /* End DEPTH dump */
                }
                free( tileData );
            }
        }
    }
    else if ( bytes == 3 )
    {
        static int offsets[4][16] = {
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 4, 1, 5, 2, 6, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
            { 0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11, 12, 13, 14, 15 },
            { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 }
        };
        float* tileData = (float*)read_tile(file,
                                            tile_width * tile_height,
                                            4 * depth,
                                            compsize,
                                            offsets[depth - 1]);
        if ( tileData ) {
            uint32_t* t = (uint32_t*)tileData;
            for ( int i = 0; i < tile_width*tile_height*depth; ++i )
            {
                t[i] = ntohl( t[i] );
            }
            unsigned w = width();
            unsigned h = height() - 1;
            float* from = tileData;
            float* pixels = (float*)canvas->data().get();
            for (unsigned i = 0; i < tile_height; i++) {
                unsigned offset = (w * (h - (y1+i))) + x1;
                float* to = pixels + depth * offset;
                for (unsigned j = 0; j < tile_width; j++) {
                    for (unsigned k = 0; k < depth; k++)
                        to[k] = from[depth - k - 1];
                    to += depth;
                    from += depth;
                }
            }
            free( tileData );
        }

    }
    else if ( bytes == 1 )
    {
        static int offsets[4][8] = {
            { 0, 1, 2, 3, 4, 5, 6, 7 },
            { 0, 2, 1, 3, 4, 5, 6, 7 },
            { 0, 2, 4, 1, 3, 5, 6, 7 },
            { 0, 2, 4, 6, 1, 3, 5, 7 }
        };
        uint16_t* tileData = (uint16_t*)read_tile(file,
                             tile_width * tile_height,
                             2 * depth,
                             compsize,
                             offsets[depth - 1]);

        if ( tileData ) {
            uint16_t* t = tileData;
            for ( int i = 0; i < tile_width*tile_height*depth; ++i )
            {
                t[i] = ntohs( t[i] );
            }
            unsigned w = width();
            unsigned h = height() - 1;
            uint16_t* pixels = (uint16_t*)canvas->data().get();
            uint16_t* from = tileData;
            for (unsigned i = 0; i < tile_height; i++) {
                unsigned offset = (w * (h - (y1+i))) + x1;
                uint16_t* to = pixels + depth * offset;
                for (unsigned j = 0; j < tile_width; j++) {
                    for (unsigned k = 0; k < depth; k++)
                        to[k] = from[depth - k - 1];
                    to += depth;
                    from += depth;
                }
            }
            free( tileData );
        }
    }
    else if ( bytes == 0 )
    {
        static int offsets[] = {
            0, 1, 2, 3
        };
        uint8_t* tileData = read_tile(file, tile_width * tile_height, depth,
                                      compsize, offsets);
        if ( tileData ) {
            uint8_t* pixels = (uint8_t*)canvas->data().get();
            uint8_t* from = tileData;
            unsigned w = width();
            unsigned h = height() - 1;
            for (unsigned i = 0; i < tile_height; i++) {
                unsigned offset = (w * (h - (y1+i))) + x1;
                uint8_t* to = pixels + depth * offset;
                for (unsigned j = 0; j < tile_width; j++) {
                    for (unsigned k = 0; k < depth; k++)
                        to[k] = from[depth - k - 1];
                    to += depth;
                    from += depth;
                }
            }
            free( tileData );
        }
    }
    else
    {
        IMG_ERROR( _("Problem with tile") << " ["
                   << block.x1 << "," << block.y1 << "]-["
                   << block.x2 << "," << block.y2 << "]");
        fseek( file, compsize, SEEK_CUR );
    }
}

bool iffImage::fetch( mrv::image_type_ptr& canvas,
		      const boost::int64_t frame )
{
    _gamma = 1.0f;
    _compression = kNoCompression;

    FILE* f = fl_fopen( sequence_filename(frame).c_str(), "rb" );
    if (!f) return false;

    iffHeader header;
    bool found = false;
    unsigned tile = 0;
    char buf[1024];

    while( !feof(f) )
    {
        iffChunk chunk;
        read_chunk( f, chunk );
        if ( chunk.tag == kTBHD_TAG )
        {
            found = true;
            size_t r = fread( &header, chunk.size, 1, f );
            header.swap();

            // _depth = 8 * (header.bytes+1);

            image_size( header.width, header.height );

            image_type::Format format = image_type::kLumma;

            _num_channels = 0;
            _layers.clear();

            if ( header.flags & kHasRGB )
            {
                rgb_layers();
                lumma_layers();
                format = image_type::kRGB;
            }
            if ( header.flags & kHasAlpha )
            {
                alpha_layers();
                format = image_type::kRGBA;
            }
            if ( header.flags & kHas12Bit )
            {
                // 		_depth = 12;
            }
            if ( header.flags & kHasZ )
            {
                _layers.push_back( N_("Z") );
                ++_num_channels;
            }

            image_type::PixelType pixel_type = image_type::kByte;

            if ( _channel && strcmp( _channel, N_("Z") ) == 0 )
            {
                format = image_type::kLumma;
                header.bytes = 3;
            }

            switch( header.bytes )
            {
            case 0:
                pixel_type = image_type::kByte;
                break;
            case 1:
                pixel_type = image_type::kShort;
                break;
            case 3:
                pixel_type = image_type::kFloat;
                break;
            default:
                LOG_ERROR( _("Unknown pixel type") );
            }

            int channels = 1;
            switch( format )
            {
            case image_type::kRGBA:
                channels = 4;
                break;
            case image_type::kRGB:
                channels = 3;
                break;
            case image_type::kLumma:
                channels = 1;
                break;
            default:
                LOG_ERROR( _("Unknown channel type" ) );
            }
            allocate_pixels( canvas, frame, channels, format, pixel_type,
                             header.width, header.height );

        }
        else if ( chunk.tag == kAUTH_TAG )
        {
            size_t r = fread( buf, chunk.size, 1, f );
            buf[chunk.size] = 0;
            end_read_chunk( f, chunk );
        }
        else if ( chunk.tag == kDATE_TAG )
        {
            size_t r = fread( buf, chunk.size, 1, f );
            buf[chunk.size] = 0;
            end_read_chunk( f, chunk );
        }
        else if ( chunk.tag == kFOR4_TAG )
        {
            unsigned int type;
            size_t r = fread( &type, 1, sizeof(unsigned int), f );
            type = ntohl( type );
            if ( type != kTBMP_TAG ) {
                end_read_chunk( f, chunk );
                continue;
            }

            while ( !feof(f) && (tile < header.tiles) )
            {
                iffChunk chunk;
                read_chunk( f, chunk );
                if ( chunk.tag == kRGBA_TAG && _channel == NULL )
                {
                    ++tile;
                    read_pixel_chunk( f, canvas, header.depth, header.bytes,
				      chunk );
                }
                else if ( chunk.tag == kZBUF_TAG && _channel &&
                          strcmp( _channel, N_("Z") ) == 0 )
                {
                    ++tile;
                    read_pixel_chunk( f, canvas, 4, 1, chunk );
                }
                else
                {
                    // Skip unknown data or not being shown
                    fseek( f, chunk.size, SEEK_CUR );
                }
                end_read_chunk( f, chunk );
            } // while

            break; // finished reading all image data
        }
        else
        {
            // We don't recognize this chunk.  Skip it.
            int size = chunk.size % 4;
            if ( size != 0 ) chunk.size += 4 - size;
            fseek( f, chunk.size, SEEK_CUR );
        }
    }

    fclose(f);

    if (!found) return false;

    refresh();
    return true;
}


} // namespace mrv
