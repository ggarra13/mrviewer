#include <iostream>

#include "core/mrvI8N.h"

#include <boost/process.hpp>
#ifdef _WIN32
#  include <boost/process/windows.hpp>
#endif

namespace mrv
{

    using namespace boost::process;

    bool YouTube( const std::string& url, std::string& videourl,
                  std::string& audiourl, std::string& title )
    {
        std::string cmd = N_("youtube-dl");
        cmd += N_(" --no-warnings -J --flat-playlist --no-playlist --no-check-certificate -f best -- ") + url + "";


        ipstream pipe_stream;
#ifdef _WIN32
        child c( cmd, std_out > pipe_stream, boost::process::windows::hide );
#else
        child c( cmd, std_out > pipe_stream );
#endif

        std::string line;
        unsigned width = 0, height = 0, max_width = 0;
        size_t pos = 0, p = 0;
        while ( pipe_stream && std::getline(pipe_stream, line) &&
                !line.empty() )
        {
            // Get audio first
            while ( p != std::string::npos )
            {
                p = line.find( N_(" audio only"), p );
                if ( p != std::string::npos )
                {
                    p += 12;
                    pos = p;
                }
            }
            pos = line.find( N_("\"url\": "), pos );
            if ( pos != std::string::npos )
            {
                pos += 8;
                size_t pos2 = line.find( '"', pos );
                audiourl = line.substr( pos, pos2 - pos );
            }
            p = pos = 0;
#if 1
            // This gets you a higher resolution video if available, but
            // without audio sometimes
            while ( p != std::string::npos )
            {
                p = line.find( N_("\"format\": \""), p );
                if ( p != std::string::npos )
                {
                    p += 12;
                    p = line.find( " - ", p );
                    p += 3;
                    size_t pos2 = line.find( '"', p );
                    sscanf( line.substr( p, pos2 - p ).c_str(),
                            "%dx%d", &width, &height );
                    if ( width > max_width )
                    {
                        max_width = width;
                        pos = pos2;
                    }
                }
            }
            pos = line.find( N_("\"url\": "), pos );
#else
            pos = line.rfind( N_("\"url\": ") );
#endif
            if ( pos != std::string::npos )
            {
                pos += 8;
                size_t pos2 = line.find( '"', pos );
                videourl = line.substr( pos, pos2 - pos );
            }
            pos = line.rfind( N_("\"title\": ") );
            if ( pos != std::string::npos )
            {
                pos += 10;
                size_t pos2 = line.find( '"', pos );
                title = line.substr( pos, pos2 - pos );
            }

        }
        c.wait();

        int exit_code = c.exit_code();
        if ( exit_code != 0 ) return false;

        return true;
    }


}
