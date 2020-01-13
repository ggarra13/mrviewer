#include <iostream>

#include "core/mrvI8N.h"
#include "gui/mrvIO.h"

#include <boost/process.hpp>
#ifdef _WIN32
#  include <boost/process/windows.hpp>
#endif

namespace {
    const char* kModule = "ytb-dl";
}


namespace mrv
{

    using namespace boost::process;

    bool YouTube( const std::string& url, std::string& videourl,
                  std::string& audiourl, std::string& title )
    {
        std::string cmd = N_("youtube-dl");
        cmd += N_(" --no-warnings -F --flat-playlist --no-playlist --no-check-certificate -- ") + url;


        ipstream pipe_stream;
#ifdef _WIN32
        child c( cmd, std_out > pipe_stream, boost::process::windows::hide );
#else
        child c( cmd, std_out > pipe_stream );
#endif

        std::string line;
        char format[16];
        unsigned audio_id = 0, video_id = 0, id = 0;
        unsigned width = 0, height = 0, max_width = 0;
        while ( pipe_stream && std::getline(pipe_stream, line) &&
                !line.empty() )
        {
            DBGM2( line );
            size_t pos = line.find( "audio only" );
            if ( pos != std::string::npos )
            {
                audio_id = atoi( line.substr( 0, 4 ).c_str() );
            }
            int num = sscanf( line.c_str(), "%d %s %dx%d", &id, format,
                              &width, &height );
            if ( num != 4 ) continue;
            if ( width >= max_width )
            {
                max_width = width;
                video_id  = id;
                if ( line.find( "video only" ) == std::string::npos )
                {
                    audio_id = video_id;
                    break;
                }
            }
        }
        c.wait();

        int exit_code = c.exit_code();
        if ( exit_code != 0 ) return false;


        char buf[256];
        sprintf( buf, N_("youtube-dl --no-warnings -J -f %d --flat-playlist --no-playlist --no-check-certificate -- "), audio_id );
        cmd = buf + url;


        {
            ipstream pipe_stream;
#ifdef _WIN32
            child c( cmd, std_out > pipe_stream,
                     boost::process::windows::hide );
#else
            child c( cmd, std_out > pipe_stream );
#endif

            while ( pipe_stream && std::getline(pipe_stream, line) &&
                    !line.empty() )
            {
                size_t p = line.rfind( "\"title\": " );
                if ( p != std::string::npos )
                {
                    p += 10;
                    size_t p2 = line.find( ',', p );
                    title = line.substr( p, p2 - p - 1);
                }

                sprintf( buf, "\"format_id\": \"%d\"", audio_id );
                p = line.rfind(buf);
                if ( p == std::string::npos ) continue;

                p = line.find( "\"url\": ", p );
                if ( p == std::string::npos ) continue;

                p += 8;
                size_t p2 = line.find( ',', p );
                audiourl = line.substr( p, p2 - p - 1 );
            }

            c.wait();

            exit_code = c.exit_code();
            if ( exit_code != 0 ) return false;

            if ( video_id == audio_id )
            {
                videourl = audiourl;
                return true;
            }
        }

        sprintf( buf, N_("youtube-dl --no-warnings -J -f %d --flat-playlist --no-playlist --no-check-certificate -- "), video_id );
        cmd = buf + url;

        {
            ipstream pipe_stream;
#ifdef _WIN32
            child c( cmd, std_out > pipe_stream,
                     boost::process::windows::hide );
#else
            child c( cmd, std_out > pipe_stream );
#endif
            while ( pipe_stream && std::getline(pipe_stream, line) &&
                    !line.empty() )
            {
                size_t p = line.rfind( "\"url\": " );
                if ( p == std::string::npos ) continue;

                p += 8;
                size_t p2 = line.find( ',', p );
                videourl = line.substr( p, p2 - p - 1);
            }

            c.wait();

            exit_code = c.exit_code();
            if ( exit_code != 0 ) return false;
        }

        return true;
    }


}
