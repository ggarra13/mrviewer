/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño
x
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
 * @file   mrvImageBrowser.cpp
 * @author gga
 * @date   Sat Oct 14 11:53:10 2006
 *
 * @brief  This class manages an image browser catalogue of all loaded images.
 *
 *
 */

#include <iostream>
#include <algorithm>

#include <inttypes.h>  // for PRId64

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

#include <fltk/ask.h>
#include <fltk/damage.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Choice.h>
#include <fltk/Item.h>
#include <fltk/Menu.h>
#include <fltk/Window.h>
#include <fltk/Monitor.h>
#include <fltk/ProgressBar.h>
#include <fltk/run.h>

#include "core/aviImage.h"
#include "core/stubImage.h"
#include "core/smpteImage.h"
#include "core/clonedImage.h"
#include "core/mrvColorBarsImage.h"
#include "core/slateImage.h"
#include "core/Sequence.h"
#include "core/mrvACES.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvThread.h"
#include "core/mrStackTrace.h"

#include "gui/mrvIO.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvMainWindow.h"
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvElement.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvHotkey.h"
#include "mrvEDLWindowUI.h"
#include "gui/FLU/Flu_File_Chooser.h"

#include "standalone/mrvCommandLine.h"

// #define DEBUG_IMAGES

namespace
{
const char* kModule = "reel";

}


using namespace std;

extern void set_as_background_cb( fltk::Widget* o, mrv::ImageView* v );
extern void toggle_background_cb( fltk::Widget* o, mrv::ImageView* v );

extern void open_cb( fltk::Widget* w, mrv::ImageBrowser* b );
extern void open_single_cb( fltk::Widget* w, mrv::ImageBrowser* b );
extern void open_clip_xml_metadata_cb( fltk::Widget* o,
                                       mrv::ImageView* view );
extern void save_cb( fltk::Widget* o, mrv::ImageView* view );
extern void save_reel_cb( fltk::Widget* o, mrv::ImageView* view );
extern void save_snap_cb( fltk::Widget* o, mrv::ImageView* view );
extern void save_sequence_cb( fltk::Widget* o, mrv::ImageView* view );
extern void save_clip_xml_metadata_cb( fltk::Widget* o,
                                       mrv::ImageView* view );


void clone_all_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->clone_all_current();
}

void clone_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->clone_current();
}

namespace {

void ntsc_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
}

void pal_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
}

void ntsc_hdtv_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC_HDTV );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
}

void pal_hdtv_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL_HDTV );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
}

static void gamma_chart( mrv::ImageBrowser* b, float g )
{
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2,
                             uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kGammaChart, m.w(), m.h() );
    img->gamma(g);
    img->fetch(1);
    img->default_rendering_transform();
    img->gamma(1.0f);
    b->add( img );
}

void gamma_chart_14_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    gamma_chart(b, 1.4f);
}
void gamma_chart_18_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    gamma_chart(b, 1.8f);
}
void gamma_chart_22_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    gamma_chart(b, 2.2f);
}
void gamma_chart_24_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    gamma_chart(b, 2.4f);
}

void linear_gradient_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2,
                             uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kLinearGradient,
                                      m.w(), m.h() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
}

void luminance_gradient_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2,
                             uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kLuminanceGradient,
                                      m.w(), m.h() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
}

void checkered_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::smpteImage;

    smpteImage* img = new smpteImage( smpteImage::kCheckered, 640, 480 );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
}


void slate_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    using mrv::slateImage;

    mrv::media cur = b->current_image();
    if ( !cur ) return;

    slateImage* img = new slateImage( cur->image() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
}

/**
 * Callback used to attach a color profile to the current image
 *
 * @param o     ImageBrowser*
 * @param data  unused
 */
static void attach_color_profile_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->attach_icc_profile();
}


static void attach_ctl_script_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    mrv::media fg = b->current_image();
    if ( ! fg ) return;

    attach_ctl_script( fg->image() );
}
}

namespace mrv {


Element::Element( mrv::media& m ) :
    fltk::Item( m->image()->fileroot() ),
    _elem( m )
{
    CMedia* img = m->image();
    m->create_thumbnail();
    image( m->thumbnail() );


    char info[2048];
    if ( dynamic_cast< clonedImage* >( img ) != NULL )
    {
        sprintf( info,
                 _("Cloned Image\n"
                   "Name: %s\n"
                   "Date: %s\n"
                   "Resolution: %dx%d"),
                 img->name().c_str(),
                 img->creation_date().c_str(),
                 img->width(),
                 img->height()
               );
    }
    else
    {
        sprintf( info,
                 _("Directory: %s\n"
                   "Name: %s\n"
                   "Resolution: %dx%d\n"
                   "Frames: %" PRId64 "-%" PRId64 " FPS %g"),
                 img->directory().c_str(),
                 img->name().c_str(),
                 img->width(),
                 img->height(),
                 img->start_frame(),
                 img->end_frame(),
                 img->fps()
               );
    }
    copy_label( info );
}

Element::~Element()
{
}

/**
 * Constructor
 *
 * @param x window's x position
 * @param y window's y position
 * @param w window's width
 * @param h window's height
 */
ImageBrowser::ImageBrowser( int x, int y, int w, int h ) :
    fltk::Browser( x, y, w, h ),
    _reel( 0 ),
    dragging( NULL )
{
}

ImageBrowser::~ImageBrowser()
{
    clear();
    wait_on_threads();
    uiMain = NULL;
}

void ImageBrowser::wait_on_threads()
{
    thread_pool_t::iterator i = _load_threads.begin();
    thread_pool_t::iterator e = _load_threads.end();
    for ( ; i != e; ++i )
    {
        (*i)->join();
        delete *i;
    }
}

mrv::Timeline* ImageBrowser::timeline()
{
    if ( uiMain == NULL ) return NULL;
    assert( uiMain != NULL );
    assert( uiMain->uiTimeline != NULL );
    return uiMain->uiTimeline;
}

/**
 * @return current reel selected or NULL
 */
mrv::Reel ImageBrowser::current_reel() const
{
    assert( _reels.empty() || _reel < _reels.size() );
    return _reels.empty()? mrv::Reel() : _reels[ _reel ];
}

void ImageBrowser::debug_images() const
{
#ifdef DEBUG_IMAGES
    mrv::Reel reel = current_reel();
    if ( !reel ) {
        LOG_INFO( "NO REEL" );
        return;
    }

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    unsigned idx = 0;
    for ( ; i != e; ++i, ++idx )
    {
        LOG_INFO( "#" << idx << " " << (*i)->image()->name() );
    }
#endif
}

/**
 * Change to a certain reel
 *
 * @param idx reel index
 *
 * @return new reel or NULL if invalid index
 */
mrv::Reel ImageBrowser::reel( const char* name )
{
    mrv::ReelList::const_iterator i = _reels.begin();
    mrv::ReelList::const_iterator e = _reels.end();
    unsigned int idx = 0;
    for ( ; i != e; ++i, ++idx )
    {
        if ( (*i)->name == name ) {
            if ( _reel >= 0 )
            {
                if ( view()->playback() != CMedia::kStopped )
                    view()->stop();
            }
            _reel = idx;
            change_reel();
            return *i;
        }
    }
    return mrv::Reel();
}


/**
 * Change to a certain reel
 *
 * @param idx reel index
 *
 * @return new reel or NULL if invalid index
 */
mrv::Reel ImageBrowser::reel( unsigned idx )
{
    assert( idx < _reels.size() );
    if ( _reel == idx ) {
        change_reel();
        return _reels[ idx ];
    }

    if ( _reel >= 0 )
    {
        view()->stop();
    }

    _reel = idx;
    change_reel();
    return _reels[ idx ];
}

mrv::Reel ImageBrowser::reel_at( unsigned idx )
{
    size_t len = _reels.size();
    if ( len == 0 || idx >= len ) return mrv::Reel();
    return _reels[ idx ];
}

/**
 * @return current image selected or NULL
 */
mrv::media ImageBrowser::current_image()
{
    if ( !view() ) return mrv::media();
    return view()->foreground();
}

/**
 * @return current image selected or NULL
 */
const mrv::media ImageBrowser::current_image() const
{
    if ( !view() ) return mrv::media();
    return view()->foreground();
}

/**
 * Create a new reel, giving it a unique name
 *
 * @param name Name of new reel
 *
 * @return new reel created
 */
mrv::Reel ImageBrowser::new_reel( const char* orig )
{
    mrv::ReelList::const_iterator i = _reels.begin();
    mrv::ReelList::const_iterator e = _reels.end();
    std::string name = orig;
    int idx = 2;
    for ( ; i != e; ++i )
    {
        if ( (*i)->name == name )
        {
            name = orig;
            name += " #";
            char buf[32];
            sprintf( buf, "%d", idx );
            name += buf;
            i = _reels.begin();
            ++idx;
        }
    }

    mrv::Reel reel( new mrv::Reel_t( name.c_str() ) );
    _reels.push_back( reel );
    assert( !_reels.empty() );

    char buf[256];
    sprintf( buf, "Reel \"%s\"", reel->name.c_str() );
    if ( view() ) view()->send_network( buf );

    _reel = (unsigned int) _reels.size() - 1;
    _reel_choice->add( name.c_str() );
    _reel_choice->value( _reel );

    mrv::EDLGroup* eg = edl_group();
    if ( eg )
    {
        eg->add_media_track( _reel );
    }

    if ( !main()->uiEDLWindow ) return reel;

    fltk::Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
    fltk::Choice* c2 = main()->uiEDLWindow->uiEDLChoiceTwo;

    int one = c1->value();
    c1->clear();

    int two = c2->value();
    c2->clear();

    size_t reels = number_of_reels();
    for ( size_t i = 0; i < reels; ++i )
    {
        mrv::Reel r = this->reel( (unsigned int) i );

        c1->add( r->name.c_str() );
        c2->add( r->name.c_str() );
    }

    if ( one == -1 && two == -1 )
    {
        c1->value(0);
        if ( reels > 1 ) c2->value(1);
    }
    else
    {
        c1->value( one );
        if ( two == -1 && reels > 1 )
            c2->value( 1 );
        else
            c2->value( two );
    }
    c1->redraw();
    c2->redraw();

    if ( eg )
    {
        eg->refresh();
        eg->redraw();
    }

    if ( ! _reels.empty() ) change_reel();
    return reel;
}

#undef fprintf
#undef fclose
#undef fopen

/**
 * Save current reel to a disk file
 *
 */
void ImageBrowser::save_reel()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    std::string dir;

    if ( uiMain->uiPrefs->uiPrefsImagePathReelPath->value() )
    {
        mrv::media fg = current_image();
        if ( fg )
        {
            dir = fg->image()->directory();
        }
    }

    const char* file = mrv::save_reel( dir.c_str() );
    if ( !file || strlen(file) == 0 ) return;

    std::string reelname( file );
    if ( reelname.size() < 5 ||
         reelname.substr(reelname.size()-5, 5) != ".reel" )
    {
        reelname += ".reel";
    }

    setlocale( LC_NUMERIC, "C" );

    FILE* f = fltk::fltk_fopen( reelname.c_str(), "w" );
    if (!f)
    {
        mrvALERT("Could not save '" << reelname << "'" );
        return;
    }

    fprintf( f, _("#\n"
                  "# mrViewer Reel \"%s\"\n"
                  "# \n"
                  "# Created with mrViewer\n"
                  "#\n\nVersion 3.0\nGhosting %d %d\n"),
             reel->name.c_str(),
             view()->ghost_previous(),
             view()->ghost_next()
        );

    mrv::MediaList::iterator i = reel->images.begin();
    mrv::MediaList::iterator e = reel->images.end();
    for ( ; i != e; ++i )
    {
        const CMedia* img = (*i)->image();

        std::string path = img->fileroot();

        if ( uiMain->uiPrefs->uiPrefsRelativePaths->value() )
        {
            fs::path parentPath = reelname; //fs::current_path();
            parentPath = parentPath.parent_path();
            fs::path childPath = img->fileroot();
            fs::path relativePath = fs::relative( childPath, parentPath );
            path = relativePath.generic_string();
        }

        fprintf( f, "\"%s\" %" PRId64 " %" PRId64
                 " %" PRId64 " %" PRId64 "\n", path.c_str(),
                 img->first_frame(), img->last_frame(),
                 img->start_frame(), img->end_frame() );
        if ( img->is_sequence() )
        {
            if ( img->has_audio() )
                fprintf( f, "audio: %s\n", img->audio_file().c_str() );
        }

        const CMedia* const right = img->right_eye();

        if ( img->is_stereo() && right )
        {
            fprintf( f, "stereo: %s\n", right->fileroot() );
        }

        const GLShapeList& shapes = img->shapes();
        if ( !shapes.empty() )
        {
            GLShapeList::const_iterator i = shapes.begin();
            GLShapeList::const_iterator e = shapes.end();

            for ( ; i != e; ++i )
            {
                GLPathShape* shape = dynamic_cast< GLPathShape* >( (*i).get() );
                if ( !shape ) continue;

                std::string cmd = shape->send();
                fprintf( f, "%s\n", cmd.c_str() );
            }
        }

    }


    if ( reel->edl )
        fprintf( f, "EDL\n" );



    fclose(f);


    setlocale( LC_NUMERIC, "" );

}

mrv::ImageView* ImageBrowser::view() const
{
    if ( uiMain == NULL ) return NULL;
    return uiMain->uiView;
}


mrv::EDLGroup* ImageBrowser::edl_group() const
{
    if ( uiMain == NULL || uiMain->uiEDLWindow == NULL ) return NULL;
    return uiMain->uiEDLWindow->uiEDLGroup;
}

/**
 * Remove/Erase current reel
 *
 */
void ImageBrowser::remove_reel()
{
    if ( view()->playback() != CMedia::kStopped )
        view()->stop();

    if ( _reels.empty() ) return;

    int ok = fltk::choice( _( "Are you sure you want to\n"
                              "remove the reel?" ),
                           _("Yes"), _("No"), NULL );
    if ( ok == 1 ) return; // No

    _reel_choice->remove(_reel);
    _reels.erase( _reels.begin() + _reel );

    fltk::Choice* c = uiMain->uiEDLWindow->uiEDLChoiceOne;

    int sel = c->value();

    if ( sel >= 0 )
    {
        c->remove( _reel );
        c->value( sel );
        c->redraw();
    }


    c = uiMain->uiEDLWindow->uiEDLChoiceTwo;
    sel = c->value();

    if ( sel >= 0 )
    {
        c->remove( _reel );
        c->value( sel );
        c->redraw();
    }

    if ( _reels.empty() ) new_reel();
    if ( _reel >= (unsigned int)_reels.size() )
        _reel = (unsigned int)_reels.size() - 1;

    _reel_choice->value( _reel );
    _reel_choice->redraw();
    change_reel();
}

/**
 * Create a new image browser's fltk::Item (widget line in browser)
 *
 * @param m media to create new fltk::Item for
 *
 * @return fltk::Item*
 */
Element* ImageBrowser::new_item( mrv::media m )
{
    Element* nw = new Element( m );

    nw->labelsize( 30 );
    nw->box( fltk::BORDER_BOX );
    return nw;
}


/**
 * Insert a new image into image browser list
 *
 * @param idx where to insert new image
 * @param img image to insert
 */
void ImageBrowser::insert( unsigned idx, mrv::media m )
{
    if ( !m ) return;

    mrv::Reel reel = current_reel();
    if ( !reel ) return;


    reel->images.insert( reel->images.begin() + idx, m );

    Element* nw = new_item( m );
    fltk::Browser::insert( *nw, idx );

    char buf[256];

    send_reel( reel );

    sprintf( buf, "InsertImage %d \"%s\"", idx, m->image()->fileroot() );
    if ( view() ) view()->send_network( buf );

    redraw();
}




void ImageBrowser::send_reel( const mrv::Reel& reel )
{
    char buf[128];
    sprintf( buf, "CurrentReel \"%s\"", reel->name.c_str() );
    if ( view() ) view()->send_network( buf );
}

void ImageBrowser::send_current_image( const mrv::media& m )
{
    if (!m) return;

    mrv::ImageView* v = view();
    if (!v) return;

    std::string buf = N_("CurrentImage \"");
    CMedia* img = m->image();
    buf += img->fileroot();
    char txt[256];
    sprintf( txt, N_("\" %" PRId64 " %" PRId64), img->first_frame(),
             img->last_frame() );
    buf += txt;

    v->send_network( buf );

    sprintf( txt, N_("Gamma %g"), v->gamma() );
    v->send_network( txt );

    sprintf(txt, N_("Gain %g"), v->gain() );
    v->send_network( txt );

    char* lbl = v->get_layer_label( v->channel() );
    sprintf(txt, N_("Channel %d %s"), v->channel(), lbl );
    free( lbl );
    v->send_network( txt );

    sprintf(txt, N_("UseLUT %d"), (int)v->use_lut() );
    v->send_network( txt );

    sprintf(txt, N_("SafeAreas %d"), (int)v->safe_areas() );
    v->send_network( txt );

    sprintf(txt, N_("Normalize %d"), (int)v->normalize() );
    v->send_network( txt );

    sprintf(txt, N_("Mask %g"), v->masking() );
    v->send_network( txt );

    sprintf( txt, N_("FPS %5.3g"), v->fps() );
    v->send_network( txt );

    sprintf( txt, N_("Looping %d"), (int)v->looping() );
    v->send_network( txt );
}

void ImageBrowser::send_images( const mrv::Reel& reel)
{
    send_reel( reel );

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    for ( ; i != e; ++i )
    {
        send_current_image( *i );
    }
}

mrv::media ImageBrowser::add( mrv::media& m )
{
    mrv::Reel reel = current_reel();
    if ( !reel ) reel = new_reel();

    reel->images.push_back( m );

    Element* nw = new_item( m );

    fltk::Browser::add( nw );

    if ( reel->images.size() == 1 )
    {
        value(0);
        change_image();
    }

    if ( visible() ) relayout();

    //send_reel( reel );
    //send_current_image( m );


    mrv::EDLGroup* e = edl_group();
    if ( e )
    {
        e->refresh();
        e->redraw();
    }

    adjust_timeline();

    return m;
}

/**
 * Add a new image at the end of image browser list
 *
 * @param img  image to insert
 */
mrv::media ImageBrowser::add( CMedia* img )
{
    if ( img == NULL ) return media();

    mrv::media m( new mrv::gui::media(img) );
    return add( m );
}


mrv::media ImageBrowser::add( const char* filename,
                              const int64_t start, const int64_t end )
{
    std::string file( filename );

#if defined(_WIN32) || defined(_WIN64)
    // Handle cygwin properly
    if ( strncmp( filename, "/cygdrive/", 10 ) == 0 )
    {
        file  = filename[10];
        file += ":/";
        file += filename + 12;
    }
#endif

    size_t len = file.size();
    if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
    {
        load_reel( file.c_str() );
        return current_image();
    }
    else
    {
        return load_image( file.c_str(), start, end, start, end );
    }
}

/**
 * Remove an image from image browser list
 *
 * @param idx index of image to remove
 */
void ImageBrowser::remove( int idx )
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    CMedia::Playback play = view()->playback();
    view()->stop();

    if ( idx < 0 || unsigned(idx) >= reel->images.size() )
    {
        LOG_ERROR( _("ImageBrowser::remove idx value (") << idx <<
                   _(") out of bounds") );
        return;
    }



    // Remove icon from browser
    fltk::Widget* w = child(idx);
    fltk::Browser::remove( idx );
    delete w;

    // Remove image from reel
    mrv::MediaList::iterator i = reel->images.begin();
    reel->images.erase( i + idx );



    char buf[256];
    sprintf( buf, "RemoveImage %d", idx );
    view()->send_network( buf );


    view()->fg_reel( _reel );
    send_reel( reel );



    change_image( idx-1 );
    // if ( unsigned(idx) < reel->images.size() )
    // {
    //  view()->foreground( *(i + idx) );
    // }
    // else if ( unsigned( idx-1) < reel->images.size() )
    // {
    //  view()->foreground( *(i + idx - 1) );
    // }
    // else
    // {
    //  view()->foreground( mrv::media() );
    // }



    mrv::EDLGroup* e = edl_group();
    if ( e )
    {
        // Refresh media tracks
        e->refresh();
        e->redraw();
    }

    if (play) view()->play( play );

    redraw();
}


/**
 * Remove an image from image browser list
 *
 * @param img image to remove
 */
void ImageBrowser::remove( mrv::media m )
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;


    mrv::MediaList::iterator begin = reel->images.begin();
    mrv::MediaList::iterator end   = reel->images.end();
    mrv::MediaList::iterator i = std::find( begin, end, m );
    if ( i == end )
    {
        LOG_ERROR( _("Image") << " " << m->image()->filename()
                   << _(" not found in reel") );
        return;
    }

    CMedia::Playback p = view()->playback();
    view()->stop();

    if ( view()->background() == m )
    {
        view()->bg_reel( -1 );
        view()->background( mrv::media() );
    }

    int idx = (int) (i - begin);
    this->remove( idx );
}



void ImageBrowser::set_bg( mrv::media bg )
{
    mrv::Reel reel = reel_at( _reel );
    if ( !reel ) return;

    view()->bg_reel( _reel );
    view()->background( bg );

    uiMain->uiReelWindow->uiBGButton->value(1);
}

void ImageBrowser::clear_bg()
{
    view()->bg_reel( -1 );
    view()->background( mrv::media() );
    uiMain->uiReelWindow->uiBGButton->value(0);
}


/**
 * Change image in image view display to reflect browser's change
 *
 */
void ImageBrowser::change_reel()
{
    DBG( "Change reel" );
    clear();

    mrv::Reel reel = current_reel();
    if ( !reel ) {
        DBG( "Invalid reel" );
        change_image( -1 );
        return;
    }

    _reel_choice->value( _reel );

    relayout();

    if ( reel->images.empty() )
    {
        DBG( "NO images in reel" );
        change_image( -1 );
    }
    else
    {

        DBG( "Add images in browser" );
        fltk::Browser::clear();

        mrv::MediaList::iterator i = reel->images.begin();
        MediaList::iterator j;
        mrv::MediaList::iterator e = reel->images.end();
        for ( ; i != e; ++i )
        {
            Element* nw = new_item( *i );

            fltk::Browser::add( nw );
        }

        DBG( "Make images contiguous in timeline" );
        adjust_timeline();

        change_image(0);
        seek( view()->frame() );
    }

    if ( reel->edl )
    {
        DBG( "SET EDL" );
        set_edl();
    }
    else
    {
        DBG( "CLEAR EDL" );
        clear_edl();
    }

    if ( view()->bg_reel() == _reel )
    {
        set_bg( view()->foreground() );
    }

    send_reel( reel );

}

/**
 * Change image in image view display to reflect browser's change
 *
 */
void ImageBrowser::change_image()
{
    int sel = value();

    mrv::ImageView* v = view();

    if ( sel < 0 )
    {
        if ( v )
        {
            v->fg_reel( -1 );
            // v->bg_reel( -1 );
            // clear_bg();
            // v->background( mrv::media() );
            v->foreground( mrv::media() );
            v->redraw();
        }
    }
    else
    {
        mrv::Reel reel = current_reel();
        if ( !reel ) return;

        assert( (unsigned)sel < reel->images.size() );

        mrv::media om = current_image();

        int audio_idx = -1;
        int sub_idx = -1;

        mrv::Timeline* t = timeline();


        mrv::media m;
        if ( unsigned(sel) < reel->images.size() ) m = reel->images[sel];


        if ( m != om && m && v )
        {
            DBG( "FG REEL " << _reel << " m: " << m->image()->name() );

            v->fg_reel( _reel );
            v->foreground( m );

            mrv::EDLGroup* e = edl_group();
            if ( e )
            {
                e->redraw();
            }


            adjust_timeline();
            //send_current_image( m );
        }
        else
        {
            DBG( "FG REEL " << _reel << " om: " << om->image()->name() );
            //send_current_image( om );
        }

    }
}


void ImageBrowser::value( int idx )
{
    send_image( idx );
    seek( view()->frame() );
    fltk::Browser::value( idx );
}

int ImageBrowser::value() const
{
    return fltk::Browser::value();
}

void ImageBrowser::send_image( int i )
{
    char buf[128];
    sprintf( buf, "ChangeImage %d", i );
    view()->send_network( buf );

}

void ImageBrowser::change_image(int i)
{
    if ( i >= children() ) {
        LOG_ERROR( _("change_image index ") << i << N_(" >= ")
                   << children() );
        return;
    }

    mrv::Reel reel = current_reel();
    send_reel( reel );
    fltk::Browser::value( i );
    send_image( i );
    change_image();
}

/**
 * Change to last image in image browser
 *
 */
void ImageBrowser::last_image()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    change_image( (int) reel->images.size()-1 );
}

/**
 * Load a stereo (right) image.
 *
 * @param name name of image
 * @param start first frame to load
 * @param end   last frame to load
 *
 * @return new image
 */
void ImageBrowser::load_stereo( mrv::media& fg,
                                const char* name,
                                const int64_t first,
                                const int64_t last,
                                const int64_t start,
                                const int64_t end )
{
    CMedia* img;

    if ( start != AV_NOPTS_VALUE )
        img = CMedia::guess_image( name, NULL, 0, false, start, end, false );
    else
        img = CMedia::guess_image( name, NULL, 0, false, first, last, false );

    if ( img == NULL )
        return;


    if ( first != AV_NOPTS_VALUE )
    {
        img->first_frame( first );
    }

    if ( last != AV_NOPTS_VALUE )
    {
        img->last_frame( last );
    }

    if ( img->has_video() || img->has_audio() )
    {
        img->fetch( img->first_frame() );
    }
    else
    {
        int64_t f = img->first_frame();
        img->find_image( f );
    }

    img->default_icc_profile();
    img->default_rendering_transform();

    PreferencesUI* prefs = ViewerUI::uiPrefs;
    img->audio_engine()->device( prefs->uiPrefsAudioDevice->value() );

    if ( fg )
    {
        CMedia* m = fg->image();

        verify_stereo_resolution( img, m );

        m->right_eye( img );  // Set image as right image of fg
        m->is_stereo( true );
        m->is_left_eye( true );


        img->is_stereo( true );
        img->is_left_eye( false );

        aviImage* aviL = dynamic_cast< aviImage* >( m );
        aviImage* aviR = dynamic_cast< aviImage* >( img );

        if ( aviL && aviR )
        {
            aviR->subtitle_file( aviL->subtitle_file().c_str() );
        }

        view()->update_layers();
    }
    else
    {
        LOG_ERROR( _("No left image to attach a stereo image pair") );
    }
}

/**
 * Load an image
 *
 * @param name name of image
 * @param start first frame to load
 * @param end   last frame to load
 *
 * @return new image
 */
mrv::media ImageBrowser::load_image( const char* name,
                                     const int64_t first,
                                     const int64_t last,
                                     const int64_t start,
                                     const int64_t end,
                                     const bool avoid_seq )
{

    mrv::Reel reel = current_reel();
    if ( !reel ) reel = new_reel();

    if ( first != AV_NOPTS_VALUE ) frame( first );

    CMedia* img;
    if ( start != AV_NOPTS_VALUE )
        img = CMedia::guess_image( name, NULL, 0, false,
                                   start, end, avoid_seq );
    else
        img = CMedia::guess_image( name, NULL, 0, false,
                                   first, last, avoid_seq );

    if ( img == NULL )
        return mrv::media();

    if ( first != AV_NOPTS_VALUE )
    {
        img->first_frame( first );
    }

    if ( last != AV_NOPTS_VALUE )
    {
        img->last_frame( last );
    }

    if ( img->has_video() || img->has_audio() )
    {
        img->seek( img->first_frame() );
    }
    else
    {
        int64_t f = img->first_frame();
        img->find_image( f );
    }

    img->default_icc_profile();
    img->default_rendering_transform();

    PreferencesUI* prefs = ViewerUI::uiPrefs;
    img->audio_engine()->device( prefs->uiPrefsAudioDevice->value() );


    mrv::media m = this->add( img );
    send_reel( reel );
    send_current_image( m );

    mrv::EDLGroup* e = edl_group();

    size_t i = 0;
    for ( i = 0; i < number_of_reels(); ++i )
    {
        if ( e && reel == this->reel( (unsigned int)i ) )
        {
            mrv::media_track* track = e->media_track((int)i);
            if ( track && m )
            {
                track->add( m );
                track->redraw();
            }
        }
    }

    return m;
}

void load_sequence( ImageBrowser::LThreadData* data )
{
    mrv::ImageView* view = data->view;
    delete data;

    while ( view->preload() )
        ;;
}

/**
 * Open new image, sequence or movie file(s) from a load list.
 * If stereo is on, every two files are treated as stereo pairs.
 * If bg image is not "", load image as background
 */
void ImageBrowser::load( const mrv::LoadList& files,
                         const bool stereo,
                         std::string bgimage,
                         const bool progressBar )
{

    view()->update( false );

    if ( bgimage != "" )
    {
        int64_t start = AV_NOPTS_VALUE;
        int64_t end   = AV_NOPTS_VALUE;
        if ( mrv::is_valid_sequence( bgimage.c_str() ) )
        {
            mrv::get_sequence_limits( start, end, bgimage );
        }
        mrv::media bg = load_image( bgimage.c_str(),
                                    start, end, start, end, false );
        view()->bg_reel( _reel );
        view()->background( bg );
    }

    //
    // Create a progress window
    //
    mrv::Reel oldreel = current_reel();
    size_t  numImages = 0;
    if ( oldreel ) numImages = oldreel->images.size();

    if ( view()->playback() != CMedia::kStopped )
        view()->stop();

    fltk::Window* main = uiMain->uiMain;

    fltk::Window* w = NULL;
    fltk::ProgressBar* progress = NULL;

    if ( files.size() > 1 && progressBar )
    {
        w = new fltk::Window( main->x(), main->y() + main->h()/2,
                              main->w(), 80 );
        w->child_of(main);
        w->clear_border();
        w->begin();
        progress = new fltk::ProgressBar( 0, 20, w->w(), w->h()-20 );
        progress->range( 0, double(files.size()) );
        progress->align( fltk::ALIGN_TOP );
        progress->showtext(true);
        w->end();

        w->show();
        fltk::check();
    }

    mrv::LoadList::const_iterator s = files.begin();
    mrv::LoadList::const_iterator i = s;
    mrv::LoadList::const_iterator e = files.end();

    mrv::media fg;
    int idx = 1;
    char buf[1024];
    for ( ; i != e; ++i, ++idx )
    {
        const mrv::LoadInfo& load = *i;

        if ( w )
        {
            snprintf( buf, 1024, _("Loading \"%s\""), load.filename.c_str() );
            progress->label(buf);
            w->redraw();
            fltk::check();
        }

        if ( load.reel )
        {
            load_reel( load.filename.c_str() );
        }
        else
        {
            if ( load.filename == "SMPTE NTSC Color Bars" )
            {
                ntsc_color_bars_cb(NULL, this);
            }
            else if ( load.filename == "PAL Color Bars" )
            {
                pal_color_bars_cb(NULL, this);
            }
            else if ( load.filename == "NTSC HDTV Color Bars" )
            {
                ntsc_hdtv_color_bars_cb(NULL, this);
            }
            else if ( load.filename == "PAL HDTV Color Bars" )
            {
                pal_hdtv_color_bars_cb(NULL, this);
            }
            else if ( load.filename == "Checkered" )
            {
                checkered_cb(NULL, this);
            }
            else if ( load.filename == "Linear Gradient" )
            {
                linear_gradient_cb(NULL, this);
            }
            else if ( load.filename == "Luminance Gradient" )
            {
                luminance_gradient_cb(NULL, this);
            }
            else if ( load.filename == "Gamma 1.4 Chart" )
            {
                gamma_chart_14_cb(NULL, this);
            }
            else if ( load.filename == "Gamma 1.8 Chart" )
            {
                gamma_chart_18_cb(NULL, this);
            }
            else if ( load.filename == "Gamma 2.2 Chart" )
            {
                gamma_chart_22_cb(NULL, this);
            }
            else if ( load.filename == "Gamma 2.4 Chart" )
            {
                gamma_chart_24_cb(NULL, this);
            }
            // @todo: slate image cannot be created since it needs info
            //        from other image.
            else
            {
                if ( stereo && ( idx % 2 == 0 ) )
                {
                    load_stereo( fg,
                                 load.filename.c_str(),
                                 load.first, load.last,
                                 load.start,
                                 load.end );
                }
                else
                {
                    fg = load_image( load.filename.c_str(),
                                     load.first, load.last, load.start,
                                     load.end, (i != s) );
                    if (!fg)
                    {
                        if ( load.filename.find( "ACESclip" ) ==
                                std::string::npos )
                            LOG_ERROR( _("Could not load '")
                                       << load.filename.c_str()
                                       << N_("'") );
                    }
                    else
                    {
                        if ( load.right_filename.size() )
                        {
                            load_stereo( fg,
                                         load.right_filename.c_str(),
                                         load.first, load.last,
                                         load.start,
                                         load.end );
                        }
                    }
                }
            }
            if ( fg )
            {
                CMedia* img = fg->image();

                if ( load.audio != "" )
                {
                    img->audio_file( load.audio.c_str() );
                    img->audio_offset( load.audio_offset );
                    view()->refresh_audio_tracks();
                }

                if ( load.subtitle != "" )
                {
                    aviImage* avi = dynamic_cast< aviImage* >( fg->image() );
                    if ( !avi )
                    {
                        LOG_ERROR( img->name() <<
                                   ": Subtitles are valid on movie files only."
                                 );
                    }
                    else
                    {
                        avi->subtitle_file( load.subtitle.c_str() );
                    }
                }

                GLShapeList& shapes = img->shapes();
                shapes = load.shapes;

                std::string xml = aces_xml_filename( img->fileroot() );
                load_aces_xml( img, xml.c_str() );

            }
        }

        if ( w )
        {
            progress->step(1);
            fltk::check();
        }
    }

    if ( w )
    {
        w->hide();
        w->destroy();
        delete w;
        fltk::check();
    }

    view()->update(true);

    view()->reset_caches(); // Redo preloaded sequence caches

    mrv::Reel reel = current_reel();
    if ( !reel || reel->images.empty() ) return;


    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();

    // If loading images to old non-empty reel, display last image.
    if ( reel == oldreel && numImages > 0 )
    {
        this->change_image( (int)reel->images.size()-1 );
        frame( img->first_frame() );
    }
    else
    {
        // display first image for good EDL playback
        this->change_image( 0 );

        if ( reel->edl )
        {
            int64_t offset = timeline()->offset( img );
            frame( offset + img->first_frame() );
        }
        else
        {
            frame( 1 );
        }
    }

    view()->fit_image();
    adjust_timeline();

    if ( ( img->has_picture() || !CMedia::preload_cache() ||
            !CMedia::cache_active() ) &&
            uiMain->uiPrefs->uiPrefsAutoPlayback->value() &&
            img->first_frame() != img->last_frame() &&
            view()->network_active() )
    {
        bool b = view()->network_active();
        view()->network_active(true);
        view()->play_forwards();
        view()->network_active(b);
    }

}


/**
 * Load an image reel
 *
 * @param name name of reel to load
 */
void ImageBrowser::load_reel( const char* name )
{
    bool edl;
    mrv::LoadList sequences;
    short previous, next;
    if ( ! parse_reel( sequences, edl, previous, next, name ) )
    {
        LOG_ERROR( "Could not parse \"" << name << "\"." );
        return;
    }

    view()->ghost_previous( previous );
    view()->ghost_next( next );

    fs::path path( name );
    std::string reelname = path.leaf().string();
    reelname = reelname.substr(0, reelname.size()-5);

    new_reel( reelname.c_str() );

    load( sequences, false, "", true );

    mrv::Reel reel = current_reel();

    if ( reel->images.empty() ) return;


    if ( edl )
    {
        set_edl();
    }
    else
    {
        clear_edl();
    }

}

void ImageBrowser::load( const stringArray& files,
                         const bool seqs,
                         const bool stereo,
                         const std::string bgfile,
                         const bool progress )
{
    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();

    mrv::LoadList loadlist;

    for ( ; i != e; ++i )
    {
        std::string file = *i;

        if ( file.substr(0, 5) == "file:" )
            file = file.substr( 5, file.size() );


        if ( file.empty() ) continue;

        size_t len = file.size();
        if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
        {
            loadlist.push_back( mrv::LoadInfo( file ) );
        }
        else
        {
            int64_t start = AV_NOPTS_VALUE;
            int64_t end   = AV_NOPTS_VALUE;

            std::string fileroot = file;
            bool load_seq = uiMain->uiPrefs->uiPrefsLoadSequence->value();
            if ( load_seq && seqs )
            {
                mrv::fileroot( fileroot, file );
                mrv::get_sequence_limits( start, end, fileroot );
            }
            loadlist.push_back( mrv::LoadInfo( fileroot, start, end ) );
        }

        retname = file;
    }

    load( loadlist, stereo, bgfile, progress );
}

/**
 * Open new image, sequence or movie file(s).
 *
 */
void ImageBrowser::open()
{
    stringArray files = mrv::open_image_file(NULL,true, uiMain);
    if (files.empty()) return;
    load( files );
}

void ImageBrowser::open_stereo()
{
    stringArray files = mrv::open_image_file(NULL,true, uiMain);
    if (files.empty()) return;

    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();

    if (files.size() > 1 )
    {
        LOG_ERROR( _("You can load only a single stereo image to combine with current one.") );
        return;
    }

    mrv::LoadList loadlist;

    std::string file = files[0];

    if ( file.substr(0, 5) == "file:" )
        file = file.substr( 5, file.size() );

    if ( file.empty() ) return;

    size_t len = file.size();
    if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
    {
        LOG_ERROR( _("You cannot load a reel as a stereo image.") );
        return;
    }
    else
    {
        int64_t start = AV_NOPTS_VALUE;
        int64_t end   = AV_NOPTS_VALUE;
        mrv::get_sequence_limits( start, end, file );
        loadlist.push_back( mrv::LoadInfo( file, start, end ) );
    }

    mrv::media fg = current_image();
    if (!fg) return;

    load_stereo( fg,
                 loadlist[0].filename.c_str(),
                 loadlist[0].first, loadlist[0].last,
                 loadlist[0].start, loadlist[0].end );
}

void ImageBrowser::open_single()
{
    stringArray files = mrv::open_image_file(NULL,false, uiMain);
    if (files.empty()) return;

    load( files, false );
}

void ImageBrowser::open_directory()
{
    std::string dir = mrv::open_directory(NULL, uiMain);
    if (dir.empty()) return;

    mrv::Options opts;
    parse_directory( dir, opts );
    load( opts.files );
}

/**
 * Save current image buffer being displayed,
 * giving it a new dummy filename.
 *
 */
void ImageBrowser::save()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    mrv::media orig = reel->images[sel];
    mrv::save_image_file( orig->image(), NULL,
                          CMedia::aces_metadata(),
                          CMedia::all_layers(), main() );
}

/**
 * Save current sequence being displayed,
 * giving it a new dummy filename.
 *
 */
void ImageBrowser::save_sequence()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    mrv::save_sequence_file( uiMain );
}

/**
 * Clone current image buffer being displayed,
 * giving it a new dummy filename.
 *
 */
void ImageBrowser::clone_current()
{
    int sel = value();
    if ( sel < 0 ) return;

    mrv::Reel reel = current_reel();
    if (!reel) return;


    mrv::media orig = reel->images[sel];
    CMedia* img = orig->image();
    if ( img == NULL ) return;

    clonedImage* copy = new clonedImage( img );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    char buf[256];
    sprintf(buf, "CloneImage \"%s\"", img->fileroot() );
    view()->send_network( buf );

    adjust_timeline();
}


/**
 * Clone current image buffer (with all its channels) being displayed,
 * giving it a new dummy filename.
 *
 */
void ImageBrowser::clone_all_current()
{
    int sel = value();
    if ( sel < 0 ) return;

    mrv::Reel reel = current_reel();
    if (!reel) return;

    mrv::media orig = reel->images[sel];
    CMedia* img = orig->image();
    if (!orig || !img ) return;

    // @hmm.... this needs fixing
    stubImage* copy = new stubImage( img );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    char buf[256];
    sprintf(buf, "CloneImageAll \"%s\"", img->fileroot() );
    view()->send_network( buf );

    adjust_timeline();
}


/**
 * Remove and delete current image buffer being displayed.
 *
 */
void ImageBrowser::remove_current()
{
    mrv::Reel reel = current_reel();
    if (!reel || reel->images.empty() ) return;

    if ( view()->playback() != CMedia::kStopped )
        view()->stop();

    int ok = fltk::choice( _( "Are you sure you want to\n"
                              "remove image from reel?" ),
                           _("Yes"), _("No"), NULL );
    if ( ok == 1 ) return; // No

    int sel = value();
    if ( sel < 0 ) return;

    mrv::media& m = reel->images[sel];

    mrv::EDLGroup* e = edl_group();

    for ( size_t i = 0; i < number_of_reels(); ++i )
    {
        if ( e && reel == this->reel( (unsigned int)i ) )
        {
            mrv::media_track* track = e->media_track((int)i);
            if ( track && m )
            {
                track->remove( m );
                break;
            }
        }
    }

    if ( sel < (int)reel->images.size() )
    {
        value( sel );
    }
    else
    {
        value( (int)reel->images.size()-1 );
    }

    change_image();
    adjust_timeline();
    redraw();
}




/**
 * Change background image to current image
 *
 */
void ImageBrowser::change_background()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    if ( view()->background() == reel->images[sel] )
    {
        DBG( "BG REEL ************* " << -1 );
        clear_bg();
    }
    else
    {
        DBG( "BG REEL ************* " << _reel << "  SEL " << sel
             << " " << reel->images[sel]->image()->name() );
        view()->bg_reel( _reel );
        mrv::media bg = reel->images[sel];
        set_bg( bg );
    }
}


/**
 * Refresh an image's postage stamp widget
 *
 * @param img image to refresh
 */
void ImageBrowser::refresh( mrv::media m )
{
    if ( ! m ) return;

    fltk::Widget* ow = this->find( m->image()->filename() );
    if (ow) ow->relayout();
}

void ImageBrowser::replace( int i, mrv::media m )
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    if ( i < 0 || i >= reel->images.size() ) return;

    CMedia::Playback play = view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    mrv::media fg = current_image();
    if (!fg) return;

    // {
    //  mrv::MediaList::const_iterator j = reel->images.begin();
    //  mrv::MediaList::const_iterator e = reel->images.end();

    //  int idx = 0;
    //  LOG_INFO( "**** REPLACE BEFORE REMOVE" );
    //  for ( ; j != e; ++j, ++idx )
    //  {
    //      fltk::Widget* w = child(idx);
    //      LOG_INFO( "\t" << (*j)->name() );
    //  }
    // }

    // Sanely remove thumbnail from reel
    Element* nw = new_item( m );
    fltk::Widget* w = child( i );
    fltk::Group::replace( i, *nw );
    delete w;

    // Sanely remove image from reel
    mrv::MediaList::const_iterator j = reel->images.begin();
    reel->images.erase( j + i );

    // {
    //  mrv::MediaList::const_iterator j = reel->images.begin();
    //  mrv::MediaList::const_iterator e = reel->images.end();

    //  int idx = 0;
    //  LOG_INFO( "**** REPLACE AFTER REMOVE" );
    //  for ( ; j != e; ++j, ++idx )
    //  {
    //      fltk::Widget* w = child(idx);
    //      LOG_INFO( "\t" << (*j)->name() );
    //  }
    // }

    // Insert item in right place on list
    j = reel->images.begin();
    reel->images.insert( j + i, m );

    // {
    //  mrv::MediaList::const_iterator j = reel->images.begin();
    //  mrv::MediaList::const_iterator e = reel->images.end();

    //  int idx = 0;
    //  LOG_INFO( "**** REPLACE AFTER INSERT" );
    //  for ( ; j != e; ++j, ++idx )
    //  {
    //      fltk::Widget* w = child(idx);
    //      LOG_INFO( "\t" << (*j)->name() );
    //  }
    // }

    view()->foreground( m );

    char buf[128];
    sprintf( buf, "ReplaceImage %d \"%s\"", i, m->image()->fileroot() );
    view()->send_network( buf );
}

/**
 * Go to next image version in disk if available
 *
 */
void ImageBrowser::next_image_version()
{
    image_version( 1 );
}

/**
 * Go to previous image version in disk if available
 *
 */
void ImageBrowser::previous_image_version()
{
    image_version( -1 );
}

/**
 * Go to next or previous image version in disk if available.
 * sum must be 1 or -1.
 *
 */
void ImageBrowser::image_version( int sum )
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    CMedia::Playback play = view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    mrv::media fg = current_image();
    if (!fg) return;

    int sel = (int)reel->index( fg->image() );
    if ( sel < 0 ) sel = 0;

    size_t i;
    {
        size_t num = reel->images.size();
        for ( i = 0; i < num; ++i )
        {
            if ( reel->images[i] == fg )
                break;
        }
        if ( i >= num )
        {
            LOG_ERROR( _("Image not found in reel ") << reel->name );
            return;
        }
    }

    short add = sum;
    unsigned short tries = 0;
    int64_t start = AV_NOPTS_VALUE;
    int64_t end   = AV_NOPTS_VALUE;
    mrv::PreferencesUI* prefs = main()->uiPrefs;
    std::string newfile;
    std::string number;
    int64_t     num;
    std::string prefix = prefs->uiPrefsImageVersionPrefix->value();
    if ( prefix.empty() )
    {
        LOG_ERROR( _("Prefix cannot be an empty string.  Please type some unique characters to distinguish the version in the filename.") );
        return;
    }
    CMedia* img = fg->image();
    unsigned max_tries = prefs->uiPrefsMaxImagesApart->value();
    while ( start == AV_NOPTS_VALUE && tries <= max_tries )
    {
        std::string file = img->fileroot();
        file = fs::path( file ).leaf().string();
        std::string dir = img->directory();
        file = dir + "/" + file;

        size_t pos = 0;
        unsigned padding = 0;
        while ( ( pos = file.find( prefix, pos) ) != std::string::npos )
        {
            pos += prefix.size();
            if ( pos >= file.size() ) continue;
            newfile = file.substr( 0, pos );
            std::string copy = file.substr( pos, file.size() );
            const char* c = copy.c_str();
            number.clear();
            while ( *c >= '0' && *c <= '9' && *c != 0 )
            {
                number += *c;
                ++c;
            }
            size_t padding = number.size();
            if ( !number.empty() )
            {
                num = atoi( number.c_str() );
                char buf[128];
                sprintf( buf, "%0*" PRId64, padding, num + sum );
                newfile += buf;
            }
            newfile += file.substr( pos + padding, file.size() );
            file = newfile;
        }

        if ( newfile.empty() )
        {
            LOG_ERROR( _("No versioning in this clip.  "
                         "Please create an image or directory named with ") << prefix );
            LOG_ERROR( _("Example:  gizmo") << prefix << N_("003.0001.exr") );
            return;
        }

        if ( mrv::is_valid_sequence( newfile.c_str() ) )
        {
            mrv::get_sequence_limits( start, end, newfile, false );
        }
        else
        {
            std::string ext = newfile;
            size_t p = ext.rfind( '.' );
            if ( p != std::string::npos )
            {
                ext = ext.substr( p, ext.size() );
            }
            std::transform( ext.begin(), ext.end(), ext.begin(),
                            (int(*)(int)) tolower );

            if ( mrv::is_valid_movie( ext.c_str() ) )
            {
                if ( fs::exists( newfile ) )
                {
                    start = 1;
                    break;
                }
            }
        }

        ++tries;
        sum += add;
    }

    if ( start == AV_NOPTS_VALUE )
    {
        int num2 = num;
        if ( add > 0 ) {
            num++;
            num2 += sum;
        }
        else {
            num--;
            num2 += sum;
        }
        LOG_ERROR( _("Versions ") << num << _(" to ") << num2
                   << _(" not found on disk for ") << img->name()  );
        return;
    }

    mrv::media m = load_image( newfile.c_str(),
                               start, end, start, end, false );
    if ( !m ) return;

    m->image()->channel( img->channel() );

    // {
    //  mrv::MediaList::const_iterator j = reel->images.begin();
    //  mrv::MediaList::const_iterator e = reel->images.end();

    //  int idx = 0;
    //  LOG_CONN( "**** after load" );
    //  for ( ; j != e; ++j, ++idx )
    //  {
    //      fltk::Widget* w = child(idx);
    //      LOG_CONN( "\t" << (*j)->name() << " " << w->label() );
    //  }
    // }

    CMedia* newImg = m->image();
    int64_t frame = img->frame();

    m->position( fg->position() );

    // Transfer all attributes to new image
    newImg->frame( frame );
    newImg->icc_profile( img->icc_profile() );
    newImg->rendering_transform( img->rendering_transform() );
    newImg->ocio_input_color_space( img->ocio_input_color_space() );
    newImg->gamma( img->gamma() );
    newImg->decode_video( frame );
    newImg->find_image( frame );
    view()->update_layers();
    newImg->fps( img->fps() );
    newImg->play_fps( img->play_fps() );


    // Sanely remove icon item from browser and replace it with another
    this->replace( int(i), m );

    // Remove last loaded element from reel (ie. m)
    this->remove( children() - 1 );

    change_image( int(i) );

    seek( view()->frame() );

    mrv::EDLGroup* e = edl_group();

    if (e)
    {
        e->refresh();
        e->redraw();
    }

    // We need two calls to foreground as it was previously set to m
    // and would return early.
    // view()->foreground( fg );
    // view()->foreground( m );
    // view()->redraw();


    img->clear_cache();

    adjust_timeline();


    if ( play ) view()->play(play);
}


/**
 * Go to next image in browser's list
 *
 */
void ImageBrowser::next_image()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    DBG( "reel name " << reel->name );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    mrv::media fg = current_image();
    unsigned sel = 0;
    if ( fg )
    {
        sel = (int)reel->index( fg->image() ) + 1;
    }

    DBG( "image selected #" << sel << "  size: " << reel->images.size() );

    if ( sel == 0 ) return;

    CMedia* orig = fg->image();

    if ( sel >= reel->images.size() )
        sel = 0;

    change_image( sel );


    if ( reel->edl )
    {
        mrv::media m = reel->images[sel];
        int64_t pos = m->position();
        DBG( "seek to " << pos );
        seek( pos );
    }
    else
    {
        fg = current_image();
        if ( fg )
        {
            CMedia* img = fg->image();
            DBG( "******* NOT REEL" );
            img->seek( orig->frame() );
            img->do_seek();
        }
    }


    if ( play ) view()->play(play);
}


/**
 * Go to previous image in browser's list
 *
 */
void ImageBrowser::previous_image()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    mrv::media fg = current_image();
    int sel = 0;
    if ( fg )
    {
        sel = (int)reel->index( fg->image() ) - 1;
    }

    int size = (int) reel->images.size() - 1;
    if ( size == -1 ) return;

    if ( sel < 0 ) sel = size;


    CMedia* orig = fg->image();

    change_image(sel);

    if ( reel->edl )
    {
        mrv::media m = reel->images[sel];
        int64_t pos = m->position() + m->duration() - 1;
        seek( pos );
    }
    else
    {
        fg = current_image();
        if ( fg )
        {
            CMedia* img = fg->image();
            DBG( "******* NOT REEL" );
            img->seek( orig->frame() );
            img->do_seek();
        }
    }


    if ( play ) view()->play(play);
}


/**
 * Handle a mouse push
 *
 * @param x window's x coordinate
 * @param y window's y coordinate
 *
 * @return 1 if handled, 0 if not
 */
int ImageBrowser::mousePush( int x, int y )
{
    int old_sel = value();

    int ok = fltk::Browser::handle( fltk::PUSH );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int button = fltk::event_button();
    int sel = value();

    DBG( "Clicked on " << sel );

    if ( button == 1 )
    {
        int clicks = fltk::event_clicks();
        if ( sel < 0 )
        {
            value( old_sel );
        }
        else
        {
            if ( sel == old_sel && clicks > 0 )
            {
                fltk::event_clicks(0);
                uiMain->uiImageInfo->uiMain->show();
                view()->update_image_info();
                //view()->send_network( "MediaInfoWindow 1" );
                if ( play != CMedia::kStopped )
                    view()->play( play );
                return ok;
            }

            DBG( "sel " << sel << "  old_sel " << old_sel );

            if ( sel == old_sel ) return ok;

            lastX = x;
            lastY = y;
            change_image();

            mrv::media m = current_image();
            if ( timeline()->edl() && m )
            {
                int64_t s = m->position();
                DBG("seek to " << s );
                seek( s );
            }
        }
        if ( play != CMedia::kStopped )
            view()->play( play );
        return 1;
    }
    else if ( button == 3 )
    {

        fltk::Menu menu(0,0,0,0);

        mrv::Reel reel = current_reel();
        if (!reel) {
            new_reel("reel");
            reel = current_reel();
            if ( !reel ) return 0;
        }

        CMedia* img = NULL;
        bool valid = false;

        menu.add( _("File/Open/Movie or Sequence"), kOpenImage.hotkey(),
                  (fltk::Callback*)open_cb, this);
        menu.add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
                  (fltk::Callback*)open_single_cb, this);

        if ( sel >= 0 )
        {
            change_image();

            mrv::media m = reel->images[sel];
            img = m->image();

            menu.add( _("File/Open/Clip XML Metadata"),
                      kOpenClipXMLMetadata.hotkey(),
                      (fltk::Callback*)open_clip_xml_metadata_cb, view() );
            menu.add( _("File/Save/Movie or Sequence As"),
                      kSaveSequence.hotkey(),
                      (fltk::Callback*)save_sequence_cb, view() );
            menu.add( _("File/Save/Reel As"), kSaveReel.hotkey(),
                      (fltk::Callback*)save_reel_cb, view() );
            menu.add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                      (fltk::Callback*)save_cb, view() );
            menu.add( _("File/Save/GL Snapshots As"), kSaveSnapshot.hotkey(),
                      (fltk::Callback*)save_snap_cb, view() );
            menu.add( _("File/Save/Clip XML Metadata As"),
                      kSaveClipXMLMetadata.hotkey(),
                      (fltk::Callback*)save_clip_xml_metadata_cb, view() );

            valid = ( dynamic_cast< slateImage* >( img ) == NULL &&
                      dynamic_cast< smpteImage* >( img ) == NULL &&
                      dynamic_cast< clonedImage* >( img ) == NULL );


            const stubImage* stub = dynamic_cast< const stubImage* >( img );
            if ( stub )
            {
                menu.add( _("Image/Clone"), 0,
                          (fltk::Callback*)clone_image_cb, this);
                menu.add( _("Image/Clone All Channels"), 0,
                          (fltk::Callback*)clone_all_cb,
                          this, fltk::MENU_DIVIDER);
            }
            else
            {
                if ( valid )
                {
                    menu.add( _("Image/Clone"), 0,
                              (fltk::Callback*)clone_image_cb, this,
                              fltk::MENU_DIVIDER);
                }
            }

            if ( valid )
            {
                menu.add( _("Image/Attach ICC Color Profile"), 0,
                          (fltk::Callback*)attach_color_profile_cb, this,
                          fltk::MENU_DIVIDER);
                menu.add( _("Image/Attach CTL Script"), 0,
                          (fltk::Callback*)attach_ctl_script_cb, this,
                          fltk::MENU_DIVIDER);
            }

            menu.add( _("Image/Set as Background"), 0,
                      (fltk::Callback*)set_as_background_cb,
                      (void*) view() );

            if ( valid )
            {
                valid = ( dynamic_cast< stubImage*  >( img ) == NULL );
            }
        }

        menu.add( _("Create/Color Bars/SMPTE NTSC"), 0,
                  (fltk::Callback*)ntsc_color_bars_cb, this);
        menu.add( _("Create/Color Bars/SMPTE NTSC HDTV"), 0,
                  (fltk::Callback*)ntsc_hdtv_color_bars_cb, this);
        menu.add( _("Create/Color Bars/PAL"), 0,
                  (fltk::Callback*)pal_color_bars_cb, this);
        menu.add( _("Create/Color Bars/PAL HDTV"), 0,
                  (fltk::Callback*)pal_hdtv_color_bars_cb, this);
        menu.add( _("Create/Gamma Chart/1.4"), 0,
                  (fltk::Callback*)gamma_chart_14_cb, this);
        menu.add( _("Create/Gamma Chart/1.8"), 0,
                  (fltk::Callback*)gamma_chart_18_cb, this);
        menu.add( _("Create/Gamma Chart/2.2"), 0,
                  (fltk::Callback*)gamma_chart_22_cb, this);
        menu.add( _("Create/Gamma Chart/2.4"), 0,
                  (fltk::Callback*)gamma_chart_24_cb, this);
        menu.add( _("Create/Gradient/Linear"), 0,
                  (fltk::Callback*)linear_gradient_cb, this);
        menu.add( _("Create/Gradient/Luminance"), 0,
                  (fltk::Callback*)luminance_gradient_cb, this);
        menu.add( _("Create/Checkered"), 0,
                  (fltk::Callback*)checkered_cb, this);

        if (valid)
            menu.add( _("Create/Slate"), 0, (fltk::Callback*)slate_cb, this);

        menu.popup( fltk::Rectangle( x, y, 80, 1) );
        return 1;
    }

    return ok;
}


/**
 * Handle a Drag and Drop operation on this widget (or image view)
 *
 */
void ImageBrowser::handle_dnd()
{
    std::string filenames = fltk::event_text();

    stringArray files;
#if defined(_WIN32) || defined(_WIN64)
    mrv::split_string( files, filenames, "\n" );
#else
    mrv::split_string( files, filenames, "\r\n" );
#endif

    std::sort( files.begin(), files.end() );

    mrv::Options opts;
    int64_t frameStart = AV_NOPTS_VALUE;
    int64_t frameEnd   = AV_NOPTS_VALUE;
    stringArray::iterator i = files.begin();
    stringArray::iterator e = files.end();
    std::string oldroot, oldview, oldext;

    if ( i != e ) {
        retname = *i;  // to open file requester in last directory
    }

    for ( ; i != e; ++i )
    {
#ifdef LINUX
        std::string file = hex_to_char_filename( *i );
#else
        std::string file = *i;
#endif

        if ( file.substr(0, 5) == "file:" )
            file = file.substr( 5, file.size() );

        if ( file.empty() ) continue;


        if ( mrv::is_directory( file.c_str() ) )
        {
            parse_directory( file, opts );
            continue;
        }
        else
        {
            std::string root, frame, view, ext;
            bool ok = mrv::split_sequence( root, frame, view, ext, file );

            bool load_seq = uiMain->uiPrefs->uiPrefsLoadSequence->value();

            if ( load_seq && ok &&
                    root != "" && frame != "" && root != oldroot && ext != oldext )
            {
                oldroot = root;
                oldext = ext;

                file = root;
                file += view;
                if ( frame[0] == '0' )
                {
                    for ( size_t i = 0; i < frame.size(); ++i )
                    {
                        if ( frame[i] >= '0' && frame[i] <= '9' ) file += '@';
                        else break;
                    }
                }
                else
                {
                    file += '@';
                }
                file += ext;

                std::string fileroot;
                mrv::fileroot( fileroot, file );
                mrv::get_sequence_limits( frameStart, frameEnd, fileroot );
                opts.files.push_back( mrv::LoadInfo( fileroot, frameStart,
                                                     frameEnd, frameStart,
                                                     frameEnd ) );
            }
            else
            {
                opts.files.push_back( mrv::LoadInfo( file, frameStart,
                                                     frameEnd, frameStart,
                                                     frameEnd ) );
            }
        }
    }

    load( opts.files );

    if ( opts.files.size() > 1 )
    {
        set_edl();
    }

    last_image();
}

/**
 * Drag an element up and down image list
 *
 * @param x new x position
 * @param y new y position
 *
 * @return 1 if drag was success, 0 if not
 */
int ImageBrowser::mouseDrag( int x, int y )
{
    int sel = value();
    if (sel < 0) return 0;

    dragging = (Element*) child(sel);
    if (y < 0) lastY = 0;
    else       lastY = y;
    redraw();
    return 1;
}

void ImageBrowser::exchange( int oldsel, int sel )
{
    if ( sel < 0 || oldsel < 0 || sel == oldsel )
    {
        redraw();
        return;
    }

    Element* e = (Element*) child(oldsel);
    if ( sel > oldsel ) sel += 1;
    fltk::Browser::insert( *e, sel );

    redraw();

    mrv::Reel reel = current_reel();
    if (!reel)
    {
        redraw();
        return;
    }

    mrv::media m = reel->images[oldsel];
    CMedia* img = m->image();

    mrv::Timeline* t = timeline();

    int64_t f = (int64_t) uiMain->uiFrame->value();
    int64_t g = t->offset( img );
    f -= g;

    char buf[1024];
    sprintf( buf, "ExchangeImage %d %d", oldsel, sel );
    view()->send_network( buf );

    reel->images.erase( reel->images.begin() + oldsel );
    if ( oldsel < sel ) sel -= 1;


    reel->images.insert( reel->images.begin() + sel, m );

    //
    // Adjust timeline position
    //
    adjust_timeline();

    //
    // Redraw EDL window
    //

    mrv::EDLGroup* eg = edl_group();
    if ( eg )
    {
        eg->redraw();
    }

    if ( t && t->edl() )
    {
        int64_t x = t->offset( img );
        f += x;
        DBG( "set frame to " << f );
        frame( f );
    }
}

int ImageBrowser::mouseRelease( int x, int y )
{

    int oldsel = value();
    if (oldsel < 0 || !dragging )
    {
        redraw();
        return 0;
    }

    dragging = NULL;

    mrv::Reel reel = current_reel();
    if (!reel)
    {
        redraw();
        return 0;
    }

    mrv::media m = reel->images[oldsel];
    CMedia* img = m->image();

    mrv::Timeline* t = timeline();

    int64_t f = (int64_t) uiMain->uiFrame->value();
    int64_t g = t->offset( img );
    f -= g;

    if ( y < 0 ) fltk::e_y = 0;

    fltk::Browser::handle( fltk::PUSH );

    int sel = value();

    exchange( oldsel, sel );


    return 1;
}

/**
 * Event handler for the image browser window
 *
 * @param event fltk::event enumeration
 *
 * @return 1 if handled, 0 if not
 */
int ImageBrowser::handle( int event )
{
    switch( event )
    {
    case fltk::KEY:
    {
        int ok = view()->handle( event );
        if (!ok) ok = fltk::Browser::handle( event );
        return ok;
    }
    case fltk::ENTER:
        take_focus();
        window()->show();
        return 1;
    case fltk::PUSH:
        return mousePush( fltk::event_x(), fltk::event_y() );
    case fltk::DRAG:
        return mouseDrag( fltk::event_x(), fltk::event_y() );
    case fltk::RELEASE:
        return mouseRelease( fltk::event_x(), fltk::event_y() );
    case fltk::DND_ENTER:
    case fltk::DND_LEAVE:
    case fltk::DND_DRAG:
    case fltk::DND_RELEASE:
        return 1;
    case fltk::PASTE:
        handle_dnd();
        return 1;
    }
    int old_sel = value();
    int ok = fltk::Browser::handle( event );
    if ( value() != old_sel ) {
        change_image();
    }
    return ok;
}



/**
 * Switch to a new frame on one image if in EDL mode or
 * in all images of reel if not.
 *
 * @param tframe new frame in timeline
 */
void ImageBrowser::seek( const int64_t tframe )
{
    int64_t f = tframe;

    char buf[64];
    sprintf( buf, "seek %" PRId64, f );
    view()->send_network(buf);


    frame( tframe );


    CMedia::Playback playback = view()->playback();


    if ( playback != CMedia::kStopped )
        view()->stop();

    mrv::Timeline* t = timeline();

    mrv::Reel reel = reel_at( view()->fg_reel() );
    mrv::Reel bgreel = reel_at( view()->bg_reel() );
    // We do not check bgreel validity here
    if ( reel && reel != bgreel && reel->edl )
    {
        // Check if we need to change to a new sequence based on frame
        mrv::media m = reel->media_at( tframe );
        if (! m ) return;

        CMedia* img = m->image();
        if ( ! img ) return;


        DBG( "SEEK FRAME " << f << " IMAGE " << img->name() );


        if ( f < t->display_minimum() )
        {
            f = int64_t(t->display_maximum() - t->display_minimum()) - f + 1;
        }
        else if ( f > t->display_maximum() )
        {
            f = int64_t(t->display_minimum() - t->display_maximum()) + f - 1;
        }


        DBG( "seek f: " << f );

        mrv::media fg = view()->foreground();

        if ( m != fg && fg )
        {
            size_t i = reel->index( f );
            img = reel->image_at( f );
            f = reel->global_to_local( f );
            if ( !img ) return;

            DBG( "seek f local1: " << f );

            img->seek( f );

            if ( (int) i < children() )
                change_image((int)i);

        }
        else
        {
            f = reel->global_to_local( f );
            DBG( "seek f local2: " << f );
            img->seek( f );
        }

        mrv::Reel reel = reel_at( view()->bg_reel() );
        if ( reel )
        {
            mrv::media bg = view()->background();
            if ( bg )
            {
                bg = reel->media_at( tframe );
                img = bg->image();

                if ( bg )
                {
                    f = reel->global_to_local( tframe );

                    img = bg->image();
                    img->seek( f );
                }
            }
        }

    }
    else
    {
        mrv::Reel reel = current_reel();
        if (!reel) return;

        mrv::media fg = view()->foreground();
        if (!fg) return;

        CMedia* img = fg->image();
        img->seek( f );
        DBG( "img->seek( " << f << " )" );
        DBG( "img->stopped? " << img->stopped() );

        mrv::media bg = view()->background();
        if ( bg )
        {
            img = bg->image();
            img->seek( f );
        }
    }

    if ( playback != CMedia::kStopped )
    {
        view()->play( playback );
    }



    view()->redraw();
    redraw();
}


/**
 * Switch to a new frame, changing timeline value and uiFrame.
 * This function does not check frame is in range in timeline.
 *
 * @param f new frame in timeline units
 */
void ImageBrowser::frame( const int64_t f )
{

    uiMain->uiFrame->value( f );
    uiMain->uiFrame->redraw();

    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->value( double(f) );
        t->redraw();
    }

    if ( !uiMain->uiEDLWindow ) return;

    t = uiMain->uiEDLWindow->uiTimeline;
    if (!t) return;

    t->value( double(f) );
    t->redraw();
}

void ImageBrowser::clear_edl()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    reel->edl = false;

    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->edl( false );
        t->redraw();
    }

    uiMain->uiReelWindow->uiEDLButton->value(0);

    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    int64_t f = img->frame();

    if ( !img ) return;

    frame( f );

    adjust_timeline();

    char buf[64];
    sprintf( buf, "EDL 0" );
    view()->send_network( buf );
}

void ImageBrowser::set_edl()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    reel->edl = true;
    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->edl( true );
        t->redraw();
    }

    uiMain->uiReelWindow->uiEDLButton->value(1);

    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    if ( !img ) return;

    int64_t f = img->frame() - img->first_frame() + t->location( img );
    frame( f );

    char buf[64];
    sprintf( buf, "EDL 1" );
    view()->send_network( buf );

}

void ImageBrowser::toggle_edl()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    if ( reel->edl ) clear_edl();
    else set_edl();
}


void ImageBrowser::attach_icc_profile()
{
    mrv::media m = current_image();
    if ( ! m ) return;

    CMedia* img = m->image();
    mrv::attach_icc_profile( img );
}

void ImageBrowser::attach_ctl_script()
{
    mrv::media m = current_image();
    if ( ! m ) return;

    CMedia* img = m->image();
    mrv::attach_ctl_script( img );
}



void ImageBrowser::adjust_timeline()
{

    int64_t first, last, f = view()->frame();

    mrv::Reel reel = current_reel();
    if ( !reel || reel->images.empty() ) return;

    if ( reel->edl )
    {
        mrv::MediaList::iterator i = reel->images.begin();
        MediaList::iterator j;
        mrv::MediaList::iterator e = reel->images.end();
        if ( i != e )
        {
            (*i)->position( 1 );

            for ( j = i, ++i; i != e; j = i, ++i )
            {
                int64_t frame = (*j)->position() + (*j)->duration();
                DBG( (*i)->image()->name() << " moved to frame " << frame );
                (*i)->position( frame );
            }
        }
    }


    if ( ! reel->edl )
    {
        mrv::media m = current_image();
        if ( ! m ) return;
        CMedia* img = m->image();

        first = img->first_frame();
        f = img->frame();
        last  = img->last_frame();

        if (f > last ) f = last;
        if (f < first ) f = first;

    }
    else
    {
        mrv::EDLGroup* e = edl_group();
        if ( e )
        {
            e->redraw();
        }

        first = 1;

        mrv::media m = current_image();
        if (! m ) return;

        CMedia* img = m->image();

        f = m->position() + img->frame() - img->first_frame();

        m = reel->images.back();
        if (! m ) return;

        last = m->position() + m->duration() - 1;
    }

    // for ( int i = 0; i < reel->images.size(); ++i )
    // {
    //    DBG( "Image " << i << " " << reel->images[i]->position() );
    // }

    // DBG( "first " << first << " f=" << f << " last " << last );

    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->minimum( double(first) );
        t->maximum( double(last) );
    }



    uiMain->uiStartButton->value(0);
    uiMain->uiStartFrame->value( first );
    uiMain->uiEndButton->value(0);
    uiMain->uiEndFrame->value( last );

    frame( f );
}


void ImageBrowser::draw()
{
    fltk::Browser::draw();

    if ( dragging )
    {
        fltk::push_matrix();
        fltk::translate( 0, lastY );
        dragging->draw();
        fltk::pop_matrix();
    }
}


} // namespace mrv
