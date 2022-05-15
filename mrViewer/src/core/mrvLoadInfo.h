/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @file   mrvLoadInfo.h
 * @author
 * @date   Thu Oct 12 20:09:31 2006
 *
 * @brief  Struct used to hold the information for loading images, reels, otio
 *         files, etc.
 *
 *
 */
#ifndef mrvLoadInfo_h
#define mrvLoadInfo_h

#include <vector>
#include "core/CMedia.h"
#include "video/mrvGLShape.h"


namespace mrv {

/**
 * Struct used to store information about stuff to load
 *
 */
    struct LoadInfo
    {
        std::string filename;
        std::string right_filename;
        std::string colorspace;
        std::string audio;

        int64_t first;
        int64_t last;
        int64_t start;
        int64_t end;
        int64_t fade_in;
        int64_t fade_out;

        double fps;

        bool    reel;
        bool    otio;
        GLShapeList shapes;
        int64_t audio_offset;

        std::string subtitle;

        bool replace_attrs;
        CMedia::Attributes attrs;

        LoadInfo( const std::string& fileroot,
                  const boost::int64_t sf, const boost::int64_t ef,
                  const boost::int64_t s = AV_NOPTS_VALUE,
                  const boost::int64_t e = AV_NOPTS_VALUE,
                  const double f = -1.0,
                  const std::string& a = "",
                  const std::string& right = "",
                  const boost::int64_t aoffset = 0,
                  const std::string& sub = "" ) :
            filename( fileroot ),
            right_filename( right ),
            audio( a ),
            first( sf ),
            last( ef ),
            start( s ),
            end( e ),
            fade_in( 0 ),
            fade_out( 0 ),
            fps( f ),
            reel( false ),
            otio( false ),
            audio_offset( aoffset ),
            subtitle( sub ),
            replace_attrs( false )
            {
                size_t len = filename.size();
                if ( len > 5 && filename.substr( len - 5, len ) == ".reel" )
                {
                    reel = true;
                }
            }

        LoadInfo( const std::string& fileroot,
                  const boost::int64_t sf, const boost::int64_t ef,
                  const GLShapeList& shl,
                  const boost::int64_t s = AV_NOPTS_VALUE,
                  const boost::int64_t e = AV_NOPTS_VALUE,
                  const double f = -1.0,
                  const std::string& a = "",
                  const std::string& right = "",
                  const boost::int64_t aoffset = 0,
                  const std::string& sub = "" ) :
            filename( fileroot ),
            right_filename( right ),
            audio( a ),
            first( sf ),
            last( ef ),
            start( s ),
            end( e ),
            fade_in( 0 ),
            fade_out( 0 ),
            fps( f ),
            reel( false ),
            otio( false ),
            shapes( shl ),
            audio_offset( aoffset ),
            subtitle( sub ),
            replace_attrs( false )
            {
                size_t len = filename.size();
                if ( len > 5 && filename.substr( len - 5, len ) == ".reel" )
                {
                    reel = true;
                }
                else if ( len > 5 && filename.substr( len - 5, len ) == ".otio" )
                {
                    otio = true;
                }
            }

        LoadInfo( const std::string& file ) :
            filename( file ),
            first( AV_NOPTS_VALUE ),
            last( AV_NOPTS_VALUE ),
            start( AV_NOPTS_VALUE ),
            end( AV_NOPTS_VALUE ),
            fade_in( 0 ),
            fade_out( 0 ),
            fps( -1.0 ),
            reel( false ),
            otio( false ),
            audio_offset( 0 ),
            replace_attrs( false )
            {
                size_t len = filename.size();
                if ( len > 5 && filename.substr( len - 5, len ) == ".reel" )
                {
                    reel = true;
                }
                else if ( len > 5 && filename.substr( len - 5, len ) == ".otio" )
                {
                    otio = true;
                }
            }

        LoadInfo( const LoadInfo& b ) :
            filename( b.filename ),
            right_filename( b.right_filename ),
            colorspace( b.colorspace ),
            audio( b.audio ),
            first( b.first ),
            last( b.last ),
            start( b.start ),
            end( b.end ),
            fade_in( 0 ),
            fade_out( 0 ),
            fps( b.fps ),
            reel( b.reel ),
            otio( b.otio ),
            shapes( b.shapes ),
            audio_offset( b.audio_offset ),
            subtitle( b.subtitle ),
            replace_attrs( b.replace_attrs ),
            attrs( b.attrs )
            {
            }

        inline LoadInfo& operator=( const LoadInfo& b )
            {
                filename = b.filename;
                right_filename = b.right_filename;
                colorspace = b.colorspace;
                audio = b.audio;
                first = b.first;
                last = b.last;
                start = b.start;
                end = b.end;
                fade_in = 0;
                fade_out = 0;
                fps = b.fps;
                reel = b.reel;
                otio = b.otio;
                shapes = b.shapes;
                audio_offset = b.audio_offset;
                subtitle = b.subtitle;
                replace_attrs = b.replace_attrs;
                attrs = b.attrs;
                return *this;
            }

    };

    typedef std::vector< LoadInfo > LoadList;


}  // namespace mrv



#endif // mrvLoadInfo_h
