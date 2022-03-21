#include <iostream>
#include <vector>
#include <map>

#include "core/mrvI8N.h"
#undef snprintf
#include "gui/mrvIO.h"

#include "core/picojson.h"

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

    struct AVInfo
    {
        bool video_only;
        bool audio_only;
        unsigned width, height, samples;
        std::string url;

        AVInfo() {};

        AVInfo( const AVInfo& b ) :
            video_only( b.video_only ),
            audio_only( b.audio_only ),
            width( b.width ),
            height( b.height ),
            samples( b.samples ),
            url( b.url )
            {
            };
    };

    using namespace picojson;

    std::string escape_url( const std::string& url )
    {
        char buf[64];
        unsigned len = url.size();
        sprintf( buf, "%%%d%%", len );
        return buf + url;
    }

    void analyze_json(
        const picojson::value& v,
        std::string& videourl,
        std::vector<std::string> videofragments,
        std::string& audiourl,
        std::vector<std::string> audiofragments
        )
    {
        if (v.is<picojson::null>()) {
        } else if (v.is<bool>()) {
        } else if (v.is<double>()) {
        } else if (v.is<std::string>()) {
        } else if (v.is<array>()) {
            const array &a = v.get<array>();
            for (array::const_iterator i = a.begin(); i != a.end(); ++i) {
                analyze_json( *i, videourl, videofragments,
                              audiourl, audiofragments );
            }
        } else if (v.is<object>()) {
            const object &o = v.get<object>();
            for (object::const_iterator i = o.begin(); i != o.end(); ++i)
            {
                analyze_json( i->second, videourl,
                              videofragments, audiourl,
                              audiofragments );
            }
        }
    }


    bool YouTube3( const std::string& url, std::string& videourl,
                   std::string& audiourl, std::string& title )
    {
        std::string cmd = N_("youtube-dl --no-warnings -J --flat-playlist --yes-playlist --no-check-certificate -- ") + url;

        ipstream pipe_stream, err_stream;
#ifdef _WIN32
        child c( cmd, std_out > pipe_stream, std_err > err_stream, boost::process::windows::hide );
#else
        child c( cmd, std_out > pipe_stream, std_err > err_stream );
#endif
        std::string line;

        std::vector< std::string > videofragments;
        std::vector< std::string > audiofragments;

        while ( pipe_stream && std::getline(pipe_stream, line) &&
                !line.empty() )
        {
            value v;
            std::string err = picojson::parse( v, line );
            if ( ! err.empty() ) {
                LOG_ERROR( err ); continue;
            }
            std::cout << line << std::endl;
            std::cout << "---- analyzing input ----" << std::endl;
            analyze_json( v, videourl, videofragments,
                          audiourl, audiofragments );

            if ( v.contains( "title" ) )
            {
                title = v.get("title").get<std::string>();
            }
            //std::cerr << v << std::endl;
        }

        return true;
    }

    bool YouTube1( const std::string& url, std::string& videourl,
                   std::string& audiourl, std::string& title )
    {
        std::string cmd = N_("youtube-dl --no-warnings -J --flat-playlist --no-playlist --no-check-certificate -- ") + url;

        ipstream pipe_stream, err_stream;
#ifdef _WIN32
        child c( cmd, std_out > pipe_stream, std_err > err_stream, boost::process::windows::hide );
#else
        child c( cmd, std_out > pipe_stream, std_err > err_stream );
#endif
        std::string line;

        size_t p = 0, p2 = 0;

        typedef std::map< std::string, AVInfo > AVInfos;
        AVInfos infos;

        char buf[256];

        while ( pipe_stream && std::getline(pipe_stream, line) &&
                !line.empty() )
        {
            DBGM1( line );

            p = line.rfind( "\"title\": " );
            if ( p != std::string::npos )
            {
                p += 10;
                size_t p2 = line.find( ',', p );
                title = line.substr( p, p2 - p - 1);
            }

            p = 0;

            while ( p != std::string::npos )
            {
                p = line.find( "\"format\": \"", p );
                if ( p == std::string::npos ) break;

                p += 11;
                p2 = line.find( '"', p );

                std::string format = line.substr( p, p2 - p );
                bool audio_only = false, video_only = true;
                std::string id;
                unsigned width = 0, height = 0, samples = 0;
                if ( format.find( "audio only" ) != std::string::npos )
                {
                    audio_only = true;
                    video_only = false;
                    p2 = format.rfind( '+' );
                    if ( p2 != std::string::npos )
                    {
                        continue;
                        // std::string audio = format.substr( p2+1,
                        //                                    format.size() );
                        // id = atoi( audio.c_str() );
                    }

                    p2 = line.rfind( "{\"asr\": ", p );
                    if ( p2 != std::string::npos )
                    {
                        p2 += 8;
                        size_t p3 = line.find( ',', p2 );
                        std::string asr = line.substr( p2, p3 - p2 );
                        if ( asr != "null" )
                        {
                            samples = atoi( asr.c_str() );
                        }
                    }
                }
                else
                {
                    int num = sscanf( format.c_str(), "%s - %dx%d", buf,
                                      &width, &height );
                    if ( num != 3 ) continue;
                    id = buf;
                }
                if ( id.empty() )
                {
                    p2 = format.find( " - " );
                    id = format.substr( 0, p2 );
                }

                std::string url;

                {
                    p = line.find( "\"url\": \"", p );
                    if ( p == std::string::npos ) break;
                    p += 8;

                    p2 = line.find( '"', p );
                    url = line.substr( p, p2 - p );
                }

                p = line.find( "\"acodec\": \"", p );
                if ( p != std::string::npos )
                {
                    p += 11;

                    p2 = line.find( '"', p );
                    if ( p2 != std::string::npos )
                    {
                        std::string codec = line.substr( p, p2 - p );
                        if ( codec != "none" )
                        {
                            video_only = false;

                            p2 = line.rfind( "{\"asr\": ", p );
                            if ( p2 != std::string::npos )
                            {
                                p2 += 8;
                                size_t p3 = line.find( ',', p2 );
                                std::string asr = line.substr( p2,
                                                               p3 - p2 );
                                if ( asr != "null" )
                                {
                                    samples = atoi( asr.c_str() );
                                    audio_only = true;
                                }
                            }
                        }
                        else
                        {
                            video_only = true;
                        }
                    }
                }

                AVInfo info;
                info.video_only = video_only;
                info.audio_only = audio_only;
                info.samples = samples;
                info.width = width;
                info.height = height;
                info.url = url;

                if ( infos.find( id ) != infos.end() )
                {
                    continue;
                }

                infos.insert( std::make_pair( id, info ) );

            }
        }

        while ( err_stream && std::getline(err_stream, line) &&
                !line.empty() )
        {
            LOG_ERROR( line );
        }

        c.wait();

        if ( c.exit_code() != 0 ) return false;


        unsigned max_width = 0, max_height = 0;
        unsigned max_samples = 0;
        AVInfos::const_iterator i = infos.begin();
        AVInfos::const_iterator e = infos.end();
        for ( ; i != e; ++i )
        {
            if ( i->second.width > max_width )
            {
                max_width = i->second.width;
                max_height = i->second.height;
                videourl = i->second.url;
            }
            if ( i->second.samples > max_samples )
            {
                max_samples = i->second.samples;
                audiourl = i->second.url;
            }
        }

        DBGM1( "VIDEO URL: " << videourl );
        LOG_INFO( title << " V: " << max_width << "x" << max_height );
        DBGM2( "AUDIO URL: " << audiourl );
        LOG_INFO( title << " A: " << max_samples << " hz." );


        if ( videourl == audiourl )
        {
            audiourl.clear();
        }

        if ( videourl.empty() && audiourl.empty() )
        {
            videourl = url;
            title = url;
        }

        return true;
    }

    bool YouTube2( const std::string& url, std::string& videourl,
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
            DBGM1( line );
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
        sprintf( buf, N_("youtube-dl --no-warnings -g -f %d --flat-playlist --no-playlist --no-check-certificate -- "), audio_id );
        cmd = buf + url;

        std::cerr << cmd << std::endl;
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
                audiourl = line;
            }

            c.wait();

            exit_code = c.exit_code();
            if ( exit_code != 0 ) return false;

            if ( video_id == audio_id )
            {
                videourl = audiourl;
                audiourl.clear();
                return true;
            }
        }

        std::cerr << "AUDIO_ID: " << audio_id << std::endl;
        std::cerr << "VIDEO_ID: " << video_id << std::endl;

        sprintf( buf, N_("youtube-dl --no-warnings -g -f %d --flat-playlist --no-playlist --no-check-certificate -- "), video_id );
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
                videourl = line;
            }

            std::cerr << "AUDIO URL: " << audiourl << std::endl;
            std::cerr << "VIDEO URL: " << videourl << std::endl;

            c.wait();

            exit_code = c.exit_code();
            if ( exit_code != 0 ) return false;
        }

        return true;
    }

    bool YouTube( const std::string& url, std::string& videourl,
                  std::string& audiourl, std::string& title )
    {
        return YouTube1( url, videourl, audiourl, title );
    }

}
