
#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>
#  include <windows.h>
#endif

#include <GL/gl.h>

#include "core/aviImage.h"
#include "core/Sequence.h"
#include "core/mrvImageOpts.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvProgressReport.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvSave.h"
#include "gui/mrvImageBrowser.h"
#include "mrViewer.h"
#include "aviSave.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace {

static const char* kModule = "save";

}

namespace mrv
{

bool save_xml( const CMedia* img, mrv::ImageOpts* ipts,
               const char* file )
{
    if ( ipts && ipts->ACES_metadata() )
    {
        const std::string& xml = aces_xml_filename( file );
        save_aces_xml( img, xml.c_str() );
    }
    return true;
}


void decode_some( CMedia* img, int64_t& frame )
{
    bool found = false;
    int64_t audio_frame = frame - 1;
    CMedia::DecodeStatus status = CMedia::kDecodeOK;
    if ( !found && img->audio_packets().size() > 0 )
    {
        ++audio_frame;
        status = img->decode_audio( audio_frame );
        found = img->find_audio( audio_frame );
    }

    status = img->decode_video( frame );
    img->find_image( frame );
}


void save_movie_or_sequence( const char* file, const mrv::ViewerUI* uiMain,
                             const bool opengl )
{
    std::string ext = file;
    std::transform( ext.begin(), ext.end(), ext.begin(),
                    (int(*)(int)) tolower);
    size_t pos = ext.rfind( '.' );
    if ( pos != std::string::npos )
    {
        ext = ext.substr( pos, ext.size() );
    }

    bool ffmpeg_handle = (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                          ext == ".tif" || ext == ".tiff" );
    bool movie = is_valid_movie( ext.c_str() ) ||
                 is_valid_audio( ext.c_str() ) || ffmpeg_handle;

    std::string root, fileseq = file;
    bool ok = mrv::fileroot( root, fileseq, false );
    if ( !ok && !movie ) {
        mrvALERT( _("Could not save sequence or movie, "
                    "only single frame specified.  Use %%d syntax") );
        return;
    }

    mrv::Timeline* timeline = uiMain->uiTimeline;
    int64_t first = int64_t( timeline->display_minimum() );
    int64_t last  = int64_t( timeline->display_maximum() );

    if ( movie )
    {
        root = root.substr( 0, root.size() - ext.size() );
    }

    fltk::Window* main = (fltk::Window*)uiMain->uiMain;
    mrv::ProgressReport* w = new mrv::ProgressReport( main, first, last );

    mrv::Reel reel = uiMain->uiReelWindow->uiBrowser->current_reel();
    int64_t dts = first;
    int64_t frame = first;
    int64_t failed_frame = frame-1;

    const char* fileroot = root.c_str();

    mrv::media old;
    double time = 0.0;
    bool open_movie = false;
    int movie_count = 1;

    bool edl = uiMain->uiTimeline->edl();


    int audio_stream = -1;

    ImageOpts* ipts = NULL;
    CMedia* img = NULL;

    if ( !movie )
    {
        mrv::media fg = uiMain->uiView->foreground();

        img = fg->image();

        bool has_deep_data = false;
        if ( img ) has_deep_data = img->has_deep_data();

        ipts = ImageOpts::build( uiMain, ext, has_deep_data );
        if ( !fg || !ipts->active() ) {
            delete ipts;
            return;
        }
        // ipts->opengl( opengl );


        save_xml( img, ipts, file );
    }

    float* data = NULL; // OpenGL temporary data frame

    if ( opengl )
    {
        unsigned w = uiMain->uiView->w();
        unsigned h = uiMain->uiView->h();
        data = new float[ 4 * w * h ];
    }

    bool skip = false;
    dts = frame - 1;
    int64_t audio_frame = frame;



#if 1
    for ( ; frame <= last; ++frame )
    {
        uiMain->uiView->seek( frame );

        mrv::media fg = uiMain->uiView->foreground();
        if (!fg) return;

        img = fg->image();
#else
    for ( ; frame <= last; ++frame )
    {
        mrv::media fg = uiMain->uiView->foreground();
        if (!fg) return;

        img = fg->image();

        if ( !skip )
        {
            ++dts;
            img->frame( dts );
        }

        decode_some( img, frame );

        img->debug_audio_stores( frame, "decode", true );

        size_t vsize = img->video_packets().size();
        size_t asize = img->audio_packets().size();

#define kMIN_SIZE 25
        if ( vsize > kMIN_SIZE || asize > kMIN_SIZE )
            skip = true;
        else
            skip = false;

        if ( !skip )
        {
            dts = img->dts();
        }
#endif

        if ( old != fg )
        {
            old = fg;
            if ( open_movie )
            {
                aviImage::close_movie(img);
                img->audio_stream( audio_stream );
                open_movie = false;
            }
            if ( movie )
            {
                char buf[4096];
                if ( edl && movie_count > 1 )
                {
                    sprintf( buf, "%s%d%s", root.c_str(), movie_count,
                             ext.c_str() );
                }
                else
                {
                    sprintf( buf, "%s%s", root.c_str(), ext.c_str() );
                }


                if ( fs::exists( buf ) )
                {
                    char text[4096];
                    sprintf( text, _("Do you want to replace '%s'?"),
                             buf );
                    int ok = fltk::choice( text, _("Yes"), _("No"), NULL );
                    if (ok == 1) // No
                    {
                        break;
                    }
                }

                char label[1024];
                if ( movie )
                {
                    sprintf( label, "Saving movie(s) '%s'", buf );
                }
                else
                {
                    sprintf( label, "Saving images '%s'", buf );
                }
                w->window()->copy_label( label );

                AviSaveUI* opts;
                if ( ffmpeg_handle )
                {
                    opts = new AviSaveUI( NULL );
                    opts->video_bitrate = 100;
                    opts->video_codec = ext.substr(1, ext.size() );
                    if ( opts->video_codec == "tif" )
                        opts->video_codec = "tiff";
                    opts->fps = img->fps();

                    ipts = ImageOpts::build( uiMain, ext, false );
                    if ( !ipts->active() ) {
                        delete ipts;
                        delete opts;
                        delete w;
                        w = NULL;
                        return;
                    }
                    opts->metadata = ipts->ACES_metadata();
                }
                else
                    opts = new AviSaveUI( uiMain );
                if ( ( opts->video_bitrate == 0 &&
                        opts->audio_bitrate == 0 ) ||
                        ( opts->audio_codec == _("None") &&
                          opts->video_codec == _("None") ) )
                {
                    delete opts;
                    delete w;
                    w = NULL;
                    break;
                }

                audio_stream = img->audio_stream();
                if ( opts->audio_codec == _("None") )
                {
                    img->audio_stream( -1 );
                }


                if ( opengl )
                {
                    unsigned w = uiMain->uiView->w();
                    unsigned h = uiMain->uiView->h();
                    img->width( w );
                    img->height( h );
                }

                if ( aviImage::open_movie( buf, img, opts ) )
                {
                    if ( ffmpeg_handle )
                    {
                        LOG_INFO( _("Save frames '") << buf << "'" );
                    }
                    else
                    {
                        LOG_INFO( _("Open movie '") << buf << _("' to save.") );
                    }
                    open_movie = true;
                    ++movie_count;
                }
                else
                {
                    delete opts;
                    delete w;
                    w = NULL;
                    break;
                }

                if ( opengl )
                {
                    unsigned w = img->hires()->width();
                    unsigned h = img->hires()->height();
                    img->width( w );
                    img->height( h );
                }

                delete opts;
            } // if (movie)
        } // old != fg

        if ( w )  w->show();

        if ( mrv::LogDisplay::show == true )
        {
            mrv::LogDisplay::show = false;
            if (uiMain->uiLog && uiMain->uiLog->uiMain )
                uiMain->uiLog->uiMain->show();
        }

        {
            if ( opengl )
            {
                // Force a swap buffer to actualize back buffer.
                uiMain->uiView->draw();
                uiMain->uiView->swap_buffers();
                uiMain->uiView->draw();
                uiMain->uiView->swap_buffers();
            }

            // Store real frame image we may replace
            float gamma = img->gamma();
            mrv::image_type_ptr old_i = img->hires();

            if ( opengl )
            {
                unsigned w = uiMain->uiView->w();
                unsigned h = uiMain->uiView->h();

                mrv::image_type_ptr hires(
                    new mrv::image_type( img->frame(),
                                         w, h, 4,
                                         mrv::image_type::kRGBA,
                                         mrv::image_type::kFloat )
                );

                // glReadBuffer( GL_BACK );
                glPixelStorei( GL_PACK_ALIGNMENT, 1 );

                glReadPixels( 0, 0, w, h, GL_RGBA, GL_FLOAT, data );

                // Flip image vertically
                unsigned w4 = w*4;
                size_t line = w4*sizeof(float);
                unsigned lastline = h*w4;
                unsigned y2 = (h-1) * w4;

                float* flip = (float*) hires->data().get();

                for ( unsigned y = 0; y < lastline; y += w4, y2 -= w4 )
                {
                    memcpy( flip + y2, data + y, line );
                }

                // Set new hires image from snapshot
                img->gamma( 1.0f );
                img->hires( hires );
            } // opengl

            //
            // Save frame into movie or file.
            //
            if (movie && open_movie)
            {
                aviImage::save_movie_frame( img );
            }
            else if ( !movie )
            {
                char buf[1024];
                sprintf( buf, fileroot, frame );
                img->save( buf, ipts );
            } // !movie

            if ( opengl )
            {
                // Restore the image from the snapshot
                img->hires( old_i );
                img->gamma( gamma );
            } // opengl
        }

        if ( ! w->tick() ) break;
    }

    delete [] data;

    delete ipts;

    if ( open_movie && img )
    {
        aviImage::close_movie(img);
        img->audio_stream( audio_stream );
        open_movie = false;
    }


    delete w;
}

}
