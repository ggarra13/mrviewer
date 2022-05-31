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
 * @file   mrvImageBrowser.cpp
 * @author gga
 * @date   Sat Oct 14 11:53:10 2006
 *
 * @brief  This class manages an image browser catalogue of all loaded images.
 *
 *
 */
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "core/mrvFrame.h"

#include <iostream>
#include <algorithm>


#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
namespace fs = boost::filesystem;

#define FLTK_ABI_VERSION 10401

#include <FL/fl_draw.H>
#include <FL/Fl_Tree_Prefs.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl.H>
#include <FL/names.h>

#include "core/R3dImage.h"
#include "core/aviImage.h"
#include "core/stubImage.h"
#include "core/smpteImage.h"
#include "core/clonedImage.h"
#include "core/mrvColorBarsImage.h"
#include "core/mrvBlackImage.h"
// #include "core/slateImage.h"
#include "core/Sequence.h"
#include "core/mrvACES.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvMath.h"
#include "core/mrvThread.h"
#include "core/mrStackTrace.h"
#include "AMFReader.h"

#include "gui/mrvAsk.h"
#include "gui/mrvIO.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvMainWindow.h"
#include "mrViewer.h"
#include "mrvImageInfo.h"
#include "mrvReelUI.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvElement.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvHotkey.h"
#include "mrvPreferencesUI.h"
#include "mrvEDLWindowUI.h"
#include "gui/FLU/Flu_File_Chooser.h"

#include "mrvVectorscopeUI.h"
#include "mrvHistogramUI.h"
#include "mrvWaveformUI.h"
#include "mrvGl3dView.h"
#include "mrvColorAreaUI.h"

#include "standalone/mrvCommandLine.h"

#ifdef LINUX
// Turn off format-security warning as it is incorrect here
#  pragma GCC diagnostic ignored "-Wformat-security"
#endif

namespace
{
const char* kModule = "reel";

}


// #define DEBUG_IMAGES_ORDER

using namespace std;

extern void set_as_background_cb( Fl_Widget* o, mrv::ImageView* v );
extern void toggle_background_cb( Fl_Widget* o, mrv::ImageView* v );

extern void open_cb( Fl_Widget* w, mrv::ImageBrowser* b );
extern void open_single_cb( Fl_Widget* w, mrv::ImageBrowser* b );
extern void save_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_reel_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_snap_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_sequence_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_session_as_cb( Fl_Widget* o, mrv::ImageView* view );

    void select_single_cb( Fl_Menu_* o, mrv::ImageBrowser* v )
    {
        v->selectmode( FL_TREE_SELECT_SINGLE_DRAGGABLE );
        Fl_Tree_Item* item = v->first_selected_item();
        v->deselect_all( NULL, 0 );
        if ( item ) v->select( item, 0 );
    }

    void select_multi_cb( Fl_Menu_* o, mrv::ImageBrowser* v )
    {
        v->selectmode( FL_TREE_SELECT_MULTI );
    }

    void first_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* v )
    {
        mrv::ImageView* view = v->view();

        mrv::CMedia::Playback play = view->playback();
        if ( play != mrv::CMedia::kStopped )
            view->stop();

        Fl_Tree_Item* item = v->first();
        typedef std::map< size_t, mrv::media > mediaMap;

        mediaMap a;
        size_t i = 0;
        for ( ; item; item = v->next(item) )
        {
            mrv::Element* w = (mrv::Element*) item->widget();
            if ( !w ) continue;
            if ( v->is_selected(item) )
                a.insert( std::make_pair( i, w->media() ) );
            ++i;
        }

        mediaMap::iterator ai = a.begin();
        mediaMap::iterator ae = a.end();
        for ( ; ai != ae; ++ai )
        {
            i = ai->first;
            mrv::media m = ai->second;
            v->image_version( i, -1, m, true );
        }
        if ( play ) view->play(play);
    }

    void previous_image_version_cb( Fl_Menu_* o, mrv::ImageBrowser* v )
    {
        mrv::ImageView* view = v->view();

        mrv::CMedia::Playback play = view->playback();
        if ( play != mrv::CMedia::kStopped )
            view->stop();

        Fl_Tree_Item* item = v->first();
        typedef std::map< size_t, mrv::media > mediaMap;

        mediaMap a;
        size_t i = 0;
        for ( ; item; item = v->next(item) )
        {
            mrv::Element* w = (mrv::Element*) item->widget();
            if ( !w ) continue;
            if ( v->is_selected(item) )
                a.insert( std::make_pair( i, w->media() ) );
            ++i;
        }

        mediaMap::iterator ai = a.begin();
        mediaMap::iterator ae = a.end();
        for ( ; ai != ae; ++ai )
        {
            i = ai->first;
            mrv::media m = ai->second;
            v->image_version( i, -1, m );
        }

        if ( play ) view->play(play);
    }

    void next_image_version_cb( Fl_Menu_* o, mrv::ImageBrowser* v )
    {
        mrv::ImageView* view = v->view();

        mrv::CMedia::Playback play = view->playback();
        if ( play != mrv::CMedia::kStopped )
            view->stop();

        Fl_Tree_Item* item = v->first();
        typedef std::map< size_t, mrv::media > mediaMap;

        mediaMap a;
        size_t i = 0;
        for ( ; item; item = v->next(item) )
        {
            mrv::Element* w = (mrv::Element*) item->widget();
            if ( !w ) continue;
            if ( v->is_selected(item) )
                a.insert( std::make_pair( i, w->media() ) );
            ++i;
        }

        mediaMap::iterator ai = a.begin();
        mediaMap::iterator ae = a.end();
        for ( ; ai != ae; ++ai )
        {
            i = ai->first;
            mrv::media m = ai->second;
            v->image_version( i, 1, m );
        }

        if ( play ) view->play(play);
    }


    void last_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* v )
    {
        mrv::ImageView* view = v->view();

        mrv::CMedia::Playback play = view->playback();
        if ( play != mrv::CMedia::kStopped )
            view->stop();

        Fl_Tree_Item* item = v->first();
        typedef std::map< size_t, mrv::media > mediaMap;

        mediaMap a;
        size_t i = 0;
        for ( ; item; item = v->next(item) )
        {
            mrv::Element* w = (mrv::Element*) item->widget();
            if ( !w ) continue;
            if ( v->is_selected(item) )
                a.insert( std::make_pair( i, w->media() ) );
            ++i;
        }

        mediaMap::iterator ai = a.begin();
        mediaMap::iterator ae = a.end();
        for ( ; ai != ae; ++ai )
        {
            i = ai->first;
            mrv::media m = ai->second;
            v->image_version( i, 1, m, true );
        }

        if ( play ) view->play(play);
    }

void media_info_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->main()->uiImageInfo->uiMain->show();
    b->view()->update_image_info();
}

void clone_all_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->clone_all_current();
}

void clone_image_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->clone_current();
}

namespace {

    mrv::media black_gap( const mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::BlackImage;
        BlackImage*  img = new BlackImage();
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->internal( true );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, i.first );
        img->cache( canvas );
        return b->add( img );
    }

    void black_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        using mrv::BlackImage;
        mrv::LoadInfo i( "Black Gap" );
        i.start = i.first = 1; i.end = i.last = 48; i.fps = 24.0;
        black_gap( i, b );
    }

    mrv::media checkered( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::smpteImage;

        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 48;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;

        smpteImage* img = new smpteImage( smpteImage::kCheckered, 640, 480 );
        img->filename( i.filename.c_str() );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->seek( i.first );
        return b->add( img );
    }


    void checkered_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        mrv::LoadInfo i( _("Checkered") );
        i.start = i.end = 1;
        i.first = 1; i.last = 48; i.fps = 24.0;
        checkered( i, b );
    }

    mrv::media ntsc_color_bars( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::ColorBarsImage;
        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 48;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->internal( true );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        img->seek( i.first );
        return b->add( img );
    }

    void ntsc_color_bars_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        mrv::LoadInfo i( _("SMPTE NTSC Color Bars") );
        i.start = i.end = 1;
        i.first = 1; i.last = 60; i.fps = 30.0;
        ntsc_color_bars( i, b );
    }

    mrv::media pal_color_bars( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::ColorBarsImage;
        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 50;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->internal( true );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        img->seek( i.first );
        return b->add( img );
    }

    void pal_color_bars_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        mrv::LoadInfo i( _("PAL Color Bars") );
        i.start = i.end = 1;
        i.first = 1; i.last = 50; i.fps = 25.0;
        pal_color_bars( i, b );
    }

    mrv::media ntsc_hdtv_color_bars( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::ColorBarsImage;
        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 60;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        ColorBarsImage* img =
            new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC_HDTV );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->internal( true );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        img->seek( i.first );
        return b->add( img );
    }

    void ntsc_hdtv_color_bars_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        mrv::LoadInfo i( _("NTSC HDTV Color Bars") );
        i.start = i.end = 1;
        i.first = 1; i.last = 60; i.fps = 30.0;
        ntsc_hdtv_color_bars( i, b );
    }

    mrv::media pal_hdtv_color_bars( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::ColorBarsImage;
        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 50;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL_HDTV );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        img->internal( true );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        img->seek( i.first );
        return b->add( img );
    }

    void pal_hdtv_color_bars_cb( Fl_Widget* o , mrv::ImageBrowser* b )
    {
        mrv::LoadInfo i( _("PAL HDTV Color Bars") );
        i.start = i.end = 1;
        i.first = 1; i.last = 50; i.fps = 25.0;
        pal_hdtv_color_bars( i, b );
    }

static mrv::media gamma_chart( mrv::LoadInfo& i, mrv::ImageBrowser* b,
                               float g )
{
    using mrv::smpteImage;

    int X,Y,W,H;
    Fl::screen_xywh( X, Y, W, H );;
    smpteImage* img = new smpteImage( smpteImage::kGammaChart,
                                      X + W/2, Y+H/2 );
    if ( i.first == AV_NOPTS_VALUE )
        i.first = 1;
    if ( i.last == AV_NOPTS_VALUE )
        i.last = 1;
    if ( i.start == AV_NOPTS_VALUE )
        i.start = 1;
    if ( i.end == AV_NOPTS_VALUE )
        i.end = i.start;
    img->gamma(g);
    img->start_frame( i.start );
    img->first_frame( i.first );
    img->end_frame( i.end );
    img->last_frame( i.last );
    img->fps( i.fps );
    img->play_fps( i.fps );
    img->internal( true );
    mrv::image_type_ptr canvas;
    img->fetch( canvas, 1 );
    img->cache( canvas );
    img->gamma( 1.0f );
    img->seek( i.first );
    return b->add( img );
}

void gamma_chart_14_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Gamma 1.4 Chart") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    gamma_chart(i, b, 1.4f);
}
void gamma_chart_18_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Gamma 1.8 Chart") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    gamma_chart(i, b, 1.8f);
}
void gamma_chart_22_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Gamma 2.2 Chart") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    gamma_chart(i, b, 2.2f);
}
void gamma_chart_24_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Gamma 2.4 Chart") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    gamma_chart(i, b, 2.4f);
}

    mrv::media linear_gradient( mrv::LoadInfo& i, mrv::ImageBrowser* b )
    {
        using mrv::smpteImage;

        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 1;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        int X,Y,W,H;
        Fl::screen_xywh( X, Y, W, H );;
        smpteImage* img = new smpteImage( smpteImage::kLinearGradient,
                                          X + W/2, Y+H/2 );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        img->seek( i.first );
        return b->add( img );
    }

void linear_gradient_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Linear Gradient") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    linear_gradient(i, b);
}

    mrv::media luminance_gradient( mrv::LoadInfo& i,
                                   mrv::ImageBrowser* b )
    {
        using mrv::smpteImage;

        if ( i.first == AV_NOPTS_VALUE )
            i.first = 1;
        if ( i.last == AV_NOPTS_VALUE )
            i.last = 1;
        if ( i.start == AV_NOPTS_VALUE )
            i.start = 1;
        if ( i.end == AV_NOPTS_VALUE )
            i.end = i.start;
        int X,Y,W,H;
        Fl::screen_xywh( X, Y, W, H );;
        smpteImage* img = new smpteImage( smpteImage::kLuminanceGradient,
                                          X + W/2, Y+H/2 );
        img->start_frame( i.start );
        img->first_frame( i.first );
        img->end_frame( i.end );
        img->last_frame( i.last );
        img->fps( i.fps );
        img->play_fps( i.fps );
        mrv::image_type_ptr canvas;
        img->fetch( canvas, 1 );
        img->cache( canvas );
        return b->add( img );
    }

void luminance_gradient_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    mrv::LoadInfo i( _("Luminance Gradient") );
    i.start = i.first = 1; i.end = i.last = 1; i.fps = 24.0;
    luminance_gradient(i, b);
}



// void slate_cb( Fl_Widget* o, mrv::ImageBrowser* b )
// {

//     using mrv::slateImage;

//     mrv::media cur = b->current_image();
//     if ( !cur ) return;

//     slateImage* img = new slateImage( cur->image() );
//     mrv::image_type_ptr canvas;
//     img->fetch( canvas, 1 );
//     img->cache( canvas );
//     img->refresh();
//     b->add( img );
// }
}

//
// Attach a new OCIO Input Color Space to selected images.
//
void attach_ocio_ics_cb2( const std::string& ret, mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::ImageView* view = v->view();
    size_t i = 0;
    for ( ; item; item = v->next_selected_item(item), ++i )
    {
        mrv::Element* w = (mrv::Element*) item->widget();
        mrv::media m = w->media();
        mrv::CMedia* img = m->image();
        img->ocio_input_color_space( ret );
        if ( i == 0 ) view->update_ICS(m);
    }

    view->redraw();
}

void attach_ocio_ics_cb( Fl_Widget* o, mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::Element* w = (mrv::Element*) item->widget();
    mrv::media m = w->media();
    mrv::CMedia* img = m->image();
    std::string ret = make_ocio_chooser( img->ocio_input_color_space(),
                                         mrv::OCIOBrowser::kInputColorSpace );

    attach_ocio_ics_cb2( ret, v );
}

namespace mrv {

/**
 * Constructor
 *
 * @param x window's x position
 * @param y window's y position
 * @param w window's width
 * @param h window's height
 */
ImageBrowser::ImageBrowser( int x, int y, int w, int h ) :
Fl_Tree( x, y, w, h ),
_loading( false ),
_reel( 0 ),
_value( -1 ),
dragging( NULL ),
old_dragging( NULL )
{
    showroot(0);// don't show root of tree
    //selectmode(FL_TREE_SELECT_MULTI);
    selectmode(FL_TREE_SELECT_SINGLE_DRAGGABLE);
    item_draw_mode(FL_TREE_ITEM_HEIGHT_FROM_WIDGET);
    item_reselect_mode( FL_TREE_SELECTABLE_ALWAYS );
    connectorstyle( FL_TREE_CONNECTOR_NONE );
    connectorwidth( 0 );
    widgetmarginleft( 0 );
    marginleft(0);
    usericonmarginleft( 0 );
    labelmarginleft( 0 );
    linespacing(3);
}

ImageBrowser::~ImageBrowser()
{
    clear();
    uiMain = NULL;
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
mrv::Reel ImageBrowser::current_reel()
{
    if ( _reels.empty() ) {
        new_reel("reel");
        _reel = 0;
    }
    return _reels[ _reel ];
}


/**
 * Change to a certain reel
 *
 * @param name name of reel to look for
 *
 * @return new reel or old reel with same name
 */
mrv::Reel ImageBrowser::reel( const char* name )
{
    mrv::ReelList::const_iterator i = _reels.begin();
    mrv::ReelList::const_iterator e = _reels.end();
    unsigned int idx = 0;
    for ( ; i != e; ++i, ++idx )
    {
        if ( (*i)->name == name ) {
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
    assert0( idx < _reels.size() );
    if ( _reel == idx ) {
        // change_reel();
        return _reels[ idx ];
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
    // Verify reel name.  If already exists, create a new one as name #idx
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
    sprintf( buf, N_("CurrentReel \"%s\""), reel->name.c_str() );
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

    Fl_Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
    Fl_Choice* c2 = main()->uiEDLWindow->uiEDLChoiceTwo;

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

void ImageBrowser::save_session()
{
    std::string dir;
    std::string file;
    for ( unsigned i = 0; i < number_of_reels(); ++i )
    {
        mrv::Reel reel = reel_at(i);

        if ( uiMain->uiPrefs->uiPrefsImagePathReelPath->value() )
        {
            mrv::media fg = current_image();
            if ( fg && i == 0 )
            {
                dir = fg->image()->directory();
            }
        }

        if ( i == 0 ) file = mrv::save_session( dir.c_str() );
        if ( file.empty() ) return;
        size_t pos = file.find( ".session" );
        if ( pos == std::string::npos )
        {
            file += ".session";
        }

        if ( i == 0 ) save_session_(file);
        std::string reelfile = file;
        fs::path session = file;
        session = session.filename();
        session += ".reels";
        reelfile = file.substr( 0, pos );
        fs::path path = reelfile;
        path = path.parent_path();
        path /= session;
        path /= reel->name;
        reelfile = path.string();
        reelfile += ".reel";
        save_reel_( reel, reelfile );
    }
}

    void ImageBrowser::clear_reels()
    {
        clear_items();
        _reels.clear();
    }

    void ImageBrowser::save_session_( const std::string& file )
    {
        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );

        FILE* f = fl_fopen( file.c_str(), "w" );
        if (!f)
        {
            mrvALERT("Could not save '" << file << "'" );
            setlocale( LC_NUMERIC, oldloc );
            av_free( oldloc );
            return;
        }

        // Declaring argument for time()
        time_t tt;

        // Declaring variable to store return value of
        // localtime()
        struct tm * ti;

        // Applying time()
        time (&tt);

        // Using localtime()
        ti = localtime(&tt);

        char date[128];
        // Monday February 12 2019 20:14:05
        strftime( date, 128, "%A %B %e %Y %H:%M:%S", ti );

        fprintf( f, _("#\n"
                      "# mrViewer Session \n"
                      "# \n"
                      "# Created with mrViewer\n"
                      "#\n"
                      "# on %s\n"
                      "#\n\nVersion 6.0\n"),
                 date );

        for ( unsigned i = 0; i < number_of_reels(); ++i )
        {
            mrv::Reel reel = reel_at( i );
            std::string reelfile = file;

            // @WARNING:  Do not use generic_string() as it has problems
            //            on Windows and returns an empty string.
            size_t pos = reelfile.find( ".session" );
            fs::path subdir = reelfile;
            fs::path session = reelfile;
            if ( pos != std::string::npos )
            {
                reelfile = reelfile.substr( 0, pos );
            }
            subdir = subdir.filename();
            subdir += ".reels";


            session = session.parent_path();

            fs::path path = reelfile;
            path = path.parent_path();
            path /= subdir;
            try
            {
                fs::create_directories( path );
            }
            catch ( const fs::filesystem_error& e )
            {
                LOG_ERROR( "Error creating dirs: " << e.what() );
            }
            catch ( const std::bad_alloc& e )
            {
                LOG_ERROR( "Error creating dirs: " << e.what() );
            }

            // Save full path to reel
            path /= reel->name;
            path += ".reel";
            reelfile = path.string();

            reelfile = relative_path( reelfile, session.string() );
            fprintf( f, "%s\n", reelfile.c_str() );
        }

        Fl_Window* w = window();
        int on = (int)w->visible();
        fprintf( f, "ReelWindow %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiImageInfo->uiMain;
        on = (int)w->visible();
        fprintf( f, "InfoWindow %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiColorArea->uiMain;
        on = (int)w->visible();
        fprintf( f, "ColorInfo %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiColorControls->uiMain;
        on = (int)w->visible();
        fprintf( f, "ColorControls %d %d %d %d %d %d %g %g %g %g\n", on,
                 w->x(), w->y(), w->w(), w->h(),
                 (int)uiMain->uiColorControls->uiActive->value(),
                 (float)uiMain->uiColorControls->uiHue->value(),
                 (float)uiMain->uiColorControls->uiBrightness->value(),
                 (float)uiMain->uiColorControls->uiContrast->value(),
                 (float)uiMain->uiColorControls->uiSaturation->value() );

        w = uiMain->uiHistogram->uiMain;
        on = (int)w->visible();
        fprintf( f, "Histogram %d %d %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h(),
                 (int)uiMain->uiHistogram->uiHistogram->channel(),
                 (int)uiMain->uiHistogram->uiHistogram->histogram_type() );

        w = uiMain->uiVectorscope->uiMain;
        on = (int)w->visible();
        fprintf( f, "Vectorscope %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiPaint->uiMain;
        on = (int)w->visible();
        fprintf( f, "Paint %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiEDLWindow->uiMain;
        on = (int)w->visible();
        fprintf( f, "EDLWindow %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        w = uiMain->uiAttrsWindow;
        if ( w )
        {
            on = (int)w->visible();
            fprintf( f, "AttrsWindow %d %d %d %d %d\n", on,
                     w->x(), w->y(), w->w(), w->h() );
        }

        w = uiMain->uiGL3dView->uiMain;
        on = (int)w->visible();
        fprintf( f, "GL3dView %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h() );

        StereoUI* st = uiMain->uiStereo;
        w = st->uiMain;
        on = (int)w->visible();
        fprintf( f, "Stereo %d %d %d %d %d %d %d %d %d\n", on,
                 w->x(), w->y(), w->w(), w->h(),
                 (int)st->uiVR360Sphere->value(),
                 (int)st->uiVR360Cube->value(),
                 (int)st->uiStereoInput->value(),
                 (int)st->uiStereoOutput->value());

        const mrv::Rectd& s = uiMain->uiView->selection();
        if ( s.w() > 0.0 && s.h() > 0.0 )
        {
            fprintf( f, "Selection %g %g %g %g\n", s.x(), s.y(), s.w(), s.h() );
        }

        setlocale( LC_NUMERIC, oldloc );
        av_free( oldloc );

        fclose( f );
    }
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

        std::string file = mrv::save_reel( dir.c_str() );
        if ( file.empty() ) return;

        if ( fs::exists( file ) )
          {
            int ok = mrv::fl_choice( _("Are you sure you want to "
                                       "overwrite '%s'?"),
                                     _("Yes"), _("No"), NULL,
                                     file.c_str() );
            if ( ok == 1 ) // No
              return;
          }

        save_reel_( reel, file );
    }

    void ImageBrowser::save_reel_( mrv::Reel reel,
                                   const std::string& file )
    {

        std::string reelname( file );
        if ( reelname.size() < 5 ||
             ( reelname.substr(reelname.size()-5, 5) != ".reel" &&
               reelname.substr(reelname.size()-5, 5) != ".otio" ) )
        {
            reelname += ".reel";
        }

        if ( reelname.substr(reelname.size()-5, 5) == ".otio" )
        {
            return save_otio( reel, reelname );
        }

        // Declaring argument for time()
        time_t tt;

        // Declaring variable to store return value of
        // localtime()
        struct tm * ti;

        // Applying time()
        time (&tt);

        // Using localtime()
        ti = localtime(&tt);

        char date[128];
        // Monday February 12 2019 20:14:05
        strftime( date, 128, "%A %B %e %Y %H:%M:%S", ti );

        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );

        FILE* f = fl_fopen( reelname.c_str(), "w" );
        if (!f)
        {
            setlocale( LC_NUMERIC, oldloc );
            av_free( oldloc );
            mrvALERT("Could not save '" << reelname << "'" );
            return;
        }


        fprintf( f, _("#\n"
                      "# mrViewer Reel \"%s\"\n"
                      "# \n"
                      "# Created with mrViewer\n"
                      "#\n"
                      "# on %s\n"
                      "#\n\nVersion 5.0\nGhosting %d %d\n"),
                 reel->name.c_str(),
                 date,
                 view()->ghost_previous(),
                 view()->ghost_next()
            );

        mrv::MediaList::iterator i = reel->images.begin();
        mrv::MediaList::iterator e = reel->images.end();
        for ( ; i != e; ++i )
        {
            const CMedia* img = (*i)->image();

            std::string path = relative_path( img->fileroot(), reelname );

            fprintf( f, "\"%s\" %" PRId64 " %" PRId64
                     " %" PRId64 " %" PRId64 " %3.6g\n", path.c_str(),
                     img->first_frame(), img->last_frame(),
                     img->start_frame(), img->end_frame(), img->fps() );


            if ( img->has_audio() && img->audio_file() != "" )
            {
                std::string path = relative_path( img->audio_file(), reelname );

                fprintf( f, "audio: %s\n", path.c_str() );
                fprintf( f, "audio offset: %" PRId64 "\n",
                         img->audio_offset() );
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
                std::string cmd;
                for ( ; i != e; ++i )
                {
                    GLPathShape* shape = dynamic_cast< GLPathShape* >( (*i).get() );
                    if ( !shape ) {
                        GLCircleShape* shape = dynamic_cast< GLCircleShape* >( (*i).get() );
                        if ( !shape ) continue;
                        cmd = shape->send();
                    }
                    else
                    {
                        cmd = shape->send();
                    }
                    fprintf( f, "%s\n", cmd.c_str() );
                }
            }

        }




        if ( reel->edl )
            fprintf( f, "EDL\n" );



        fclose(f);


        setlocale( LC_NUMERIC, oldloc );
        av_free( oldloc );

        char buf[1024];
        sprintf( buf, _("Reel '%s' saved"), file.c_str() );
        mrv::alert( buf );

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

        int ok = mrv::fl_choice( _( "Are you sure you want to\n"
                                    "remove the reel?" ),
                                 _("Yes"), _("No"), NULL );
        if ( ok == 1 ) return; // No

        mrv::Reel reel = current_reel();
        if (!reel) return;

        size_t num = reel->images.size();
        for ( size_t i = 0; i < num; ++i )
        {
            mrv::media m = reel->images[i];
            Fl_Tree_Item* item = media_to_item( m );
            if (!item) continue;
            mrv::Element* elem = (mrv::Element*) item->widget();
            delete elem;
            item->widget( NULL );
        }

        _reel_choice->remove(_reel);

        reel->images.clear();
        _reels.erase( _reels.begin() + _reel );

        Fl_Choice* c = uiMain->uiEDLWindow->uiEDLChoiceOne;

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

        timeline()->clear_thumb();

        if ( _reels.empty() ) {
            new_reel();
        }
        if ( _reel >= (unsigned int)_reels.size() )
            _reel = (unsigned int)_reels.size() - 1;

        _reel_choice->value( _reel );
        _reel_choice->redraw();

        view()->clear_old();

        change_reel();
    }

/**
 * Create a new image browser's Fl_Item (widget line in browser)
 *
 * @param m media to create new Fl_Item for
 *
 * @return Fl_Item*
 */
    Element* ImageBrowser::new_item( mrv::media m )
    {
        Fl_Group::current(this);
        Element* nw = new Element( this, m );
        if ( !nw )
        {
            LOG_ERROR( _("Could not allocate new element" ) );
        }

        nw->labelsize( 30 );
        nw->box( FL_BORDER_BOX );
        return nw;
    }


/**
 * Insert a new image into image browser list
 *
 * @param idx where to insert new image
 * @param m   media image to insert
 */
    void ImageBrowser::insert( int idx, mrv::media m )
    {
        if ( !m ) return;

        mrv::Reel reel = current_reel();

        if ( idx > (int)reel->images.size() )
        {
            LOG_ERROR( _("Index ") << idx << (" too big for images in reel ")
                       << reel->images.size() );
            return;
        }


        std::string path = media_to_pathname( m );
        Fl_Tree_Item* item = Fl_Tree::insert( root(), path.c_str(),
                                              idx );
        Element* nw = new_item( m );
        item->widget( nw );

        match_tree_order();

        int64_t first, last;
        adjust_timeline( first, last );
        set_timeline( first, last );

        send_reel( reel );

        CMedia* img = m->image();
        std::string file = img->fileroot();

        char buf[256];
        sprintf( buf, "InsertImage %d \"%s\"", idx, file.c_str() );
        if ( view() ) view()->send_network( buf );

        redraw();
    }




    void ImageBrowser::send_reel( const mrv::Reel& reel )
    {
        char buf[128];
        sprintf( buf, N_("CurrentReel \"%s\""), reel->name.c_str() );
        if ( view() ) view()->send_network( buf );
    }

    void ImageBrowser::send_current_image( int64_t idx, const mrv::media& m )
    {
        if (!m) return;

        mrv::ImageView* v = view();
        if (!v) return;

        char text[64];
        sprintf( text, N_("CurrentImage %" PRId64 " \""), idx );
        std::string buf = text;
        CMedia* img = m->image();
        std::string file = img->fileroot();
        buf += file;
        char txt[256];
        sprintf( txt, N_("\" %" PRId64 " %" PRId64), img->first_frame(),
                 img->last_frame() );
        buf += txt;

        v->send_network( buf );

#if 1
        sprintf( txt, N_("Gamma %g"), v->gamma() );
        v->send_network( txt );

        sprintf(txt, N_("Gain %g"), v->gain() );
        v->send_network( txt );

        char* lbl = v->get_layer_label( v->channel() );
        sprintf(txt, N_("Channel %d %s"), v->channel(), lbl );
        av_free( lbl );
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
#endif

    }



    std::string ImageBrowser::media_to_pathname( const mrv::media m )
    {
        if ( !m ) {
            LOG_ERROR( _("Empty media passed to media_to_pathname") );
            return "";
        }

        CMedia* img = m->image();
        if ( !img || !img->fileroot() ) {
            if ( !img )
                LOG_ERROR( _("Empty image in media passed to media_to_pathname") );
            else
                LOG_ERROR( _("Empty img->fileroot() passed to media_to_pathname") );
            return "";
        }

        std::string path = comment_slashes( img->fileroot() );
        return path;
    }

    mrv::media ImageBrowser::add( const mrv::media m )
    {
        mrv::Reel reel = current_reel();

        add_to_tree( m );
        match_tree_order();


        if ( reel->images.size() == 1 )
        {
            change_image(0);
        }

        send_reel( reel );


        if ( !_loading )
        {
            int64_t first, last;
            adjust_timeline( first, last );
            set_timeline( first, last );

            mrv::EDLGroup* e = edl_group();
            if ( e )
            {
                e->refresh();
                e->redraw();
            }

            view()->fit_image();
        }

        send_current_image( reel->images.size() - 1, m );
        redraw();

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
        if ( strncmp( filename, N_("/cygdrive/"), 10 ) == 0 )
        {
            file  = filename[10];
            file += ":/";
            file += filename + 12;
        }
#endif

        size_t len = file.size();
        if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
        {
            LoadInfo load( file );
            load_reel( load );
            return current_image();
        }
        else if ( len > 5 && file.substr( len - 5, 5 ) == ".otio" )
        {
            LoadInfo load( file );
            load_otio( load );
            return current_image();
        }
        else
        {
            return load_image_in_reel( file.c_str(), start, end, start, end,
                                       24.0 );
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

        if ( idx < 0 || unsigned(idx) >= reel->images.size() )
        {
            LOG_ERROR( _("ImageBrowser::remove idx value (") << idx <<
                       _(") out of bounds") );
            return;
        }

        remove( reel->images[idx] );
    }


/**
 * Remove an image from image browser list
 *
 * @param img image to remove
 */
    void ImageBrowser::remove( mrv::media m )
    {
        mrv::Reel reel = current_reel();

        mrv::MediaList::iterator begin = reel->images.begin();
        mrv::MediaList::iterator end   = reel->images.end();
        mrv::MediaList::iterator i = std::find( begin, end, m );
        if ( i == end )
        {
            LOG_ERROR( _("Image") << " " << m->image()->filename()
                       << _(" not found in reel") );
            return;
        }

        CMedia::Playback play = view()->playback();
        if ( play ) view()->stop();

        int idx = int(i-begin);

        // Remove icon from browser
        Fl_Tree_Item* item = root()->child(idx);
        if ( !item )
        {
            LOG_ERROR( _("Removal item not found in tree.") );
            return;
        }
        delete item->widget(); item->widget(NULL);

        Fl_Tree::remove( item );

        callback_item( NULL );


        // if ( view()->background() == m )
        // {
        //     view()->bg_reel( -1 );
        //     view()->background( mrv::media() );
        // }

        char buf[256];
        sprintf( buf, N_("RemoveImage %d"), idx );
        view()->send_network( buf );

        // Remove image from reel
        reel->images.erase( i );


        // Select the next image to the foreground
        size_t num = reel->images.size();
        mrv::media fg;
        if ( (size_t)idx < num && num > 0 )
            fg = reel->images[idx];
        else if ( num != 0 )
            fg = reel->images[num - 1];

        view()->foreground( fg );

        CMedia* img = NULL;
        int64_t pos = 1;
        if ( fg ) {
            img = fg->image();
            pos = fg->position() - img->first_frame() + img->frame();
        }
        seek( pos );

        // clear dragging in case we were dragging the removed media
        dragging = NULL;

        match_tree_order();

        int64_t first, last;
        adjust_timeline( first, last );
        set_timeline( first, last );

        mrv::EDLGroup* e = edl_group();
        if ( e )
        {
            // Refresh media tracks
            e->refresh();
            e->redraw();
        }

        view()->clear_old();
        if (play) view()->play( play );

        view()->redraw();
        redraw();

    }



    void ImageBrowser::set_bg( mrv::media bg )
    {
        mrv::Reel reel = reel_at( _reel );
        if ( !reel ) return;

        if ( bg )
        {
            Fl_Tree_Item* bgitem = media_to_item( bg );
            if ( bgitem )
            {
                mrv::Element* elem = (mrv::Element*) bgitem->widget();
                elem->Label()->box( FL_PLASTIC_DOWN_BOX );
                elem->Label()->color( FL_YELLOW );
                redraw();
            }
        }

        view()->bg_reel( _reel );
        view()->background( bg );

        mrv::media fg = view()->foreground();

        if ( bg && fg != bg )
        {
            CMedia* bimg = bg->image();
            bimg->is_stereo( true );
            bimg->is_left_eye( false );

            CMedia* img = fg->image();
            img->is_stereo( true );
            img->right_eye( bimg );
            img->is_left_eye( true );
            img->owns_right_eye( false );

        }

        uiMain->uiBButton->copy_label( "B" );
        uiMain->uiBButton->down_box( FL_PLASTIC_DOWN_BOX );
        uiMain->uiBButton->selection_color( FL_YELLOW );
        uiMain->uiBButton->value(1);
        uiMain->uiReelWindow->uiBGButton->value(1);
    }

    void ImageBrowser::clear_bg()
    {
        view()->bg_reel( -1 );

        mrv::media fg = view()->foreground();
        mrv::media bg = view()->background();
        if ( bg && bg != fg )
        {
            CMedia* bimg = bg->image();
            bimg->is_stereo( false );
            bimg->is_left_eye( false );

            if ( fg )
            {
                CMedia* img = fg->image();
                img->is_stereo( false );
                img->right_eye( NULL );
                img->is_left_eye( false );
            }

        }

        clear_items();


        uiMain->uiBButton->copy_label( "A/B" );
        uiMain->uiBButton->selection_color( FL_BACKGROUND_COLOR );
        uiMain->uiBButton->value(0);

        view()->background( mrv::media() );
        uiMain->uiReelWindow->uiBGButton->value(0);
    }


/**
 * Change reel in browser to display to reflect browser's change
 *
 */
    void ImageBrowser::change_reel()
    {
        DBGM3( "Change reel" );

        CMedia::Playback play = view()->playback();
        if ( play ) view()->stop();

        mrv::Reel reel = current_reel();

        _reel_choice->value( _reel );

        {
            Fl_Tree_Item* i = NULL;
            for ( i = first(); i; i = next(i) )
            {
                if ( ! i->widget() ) {
                    continue;
                }

                mrv::Element* elem = (mrv::Element*) i->widget();
                delete elem;
            }
        }


        clear_children( root() );
        dragging = NULL;
        callback_item( NULL );

        if ( reel->images.empty() )
        {
            DBGM3( "NO images in reel" );

            change_image( -1 );
        }
        else
        {
            mrv::MediaList::iterator i = reel->images.begin();
            MediaList::iterator j;
            mrv::MediaList::iterator e = reel->images.end();


            for ( ; i != e; ++i )
            {
                add_to_tree( *i );
            }

            int64_t first, last;
            adjust_timeline( first, last );
            set_timeline( first, last );

            change_image(0);

            view()->fit_image();
        }

        if ( reel->edl )
        {
            DBGM3( "SET EDL" );

            set_edl();
        }
        else
        {

            DBGM3( "CLEAR EDL" );
            clear_edl();
        }


        send_reel( reel );

        if ( play != CMedia::kStopped ) view()->play( play );


        redraw();
    }



    void ImageBrowser::send_image( int i )
    {
        char buf[128];
        sprintf( buf, N_("ChangeImage %d"), i );
        view()->send_network( buf );

    }

    void ImageBrowser::real_change_image( int v, int i )
    {
        mrv::Reel reel = current_reel();

        int ok;
        Fl_Tree_Item* item;
        if ( v >= 0 && v < (int)reel->images.size() )
        {
            mrv::media orig = reel->images[v];
            CMedia* img = orig->image();
            item = root()->child(v);
            ok = deselect( item, 0 );
            if ( ok < 0 )
            {
                LOG_ERROR( _("Old item was not found in tree.") );
                return;
            }
        }

        send_reel( reel );

        value( i );

        mrv::media m;
        if ( i >= 0 && i < (int)reel->images.size() )
        {
            m = reel->images[i];

            item = root()->child(i);
            ok = select( item, 0 );
            if ( ok < 0 )
            {
                LOG_ERROR( _("New item was not found in tree.") );
                return;
            }
        }

        TRACE2( "CHANGE IMAGE TO INDEX " << i << " " << m->name() );
        view()->foreground( m );

        if ( !_loading )
        {
            int64_t first, last;
            adjust_timeline( first, last );
            mrv::Timeline* t = timeline();
            if ( t )
            {
                t->clear_thumb();
                set_timeline( first, last );
            }
        }

        send_image( i );

        CMedia* img = NULL;
        if ( m ) img = m->image();
        if ( reel->edl && img )
        {
            TRACE( img->name() << " img frame is " << img->frame() );
#if 0
            if ( !_loading )
            {
                int64_t pos = reel->local_to_global( img->frame(), img );
                if ( !play ) seek( pos );
            }
#endif
        }
        else
        {
            if ( img ) img->seek( img->first_frame() );
        }

        add_menu( main()->uiReelWindow->uiMenuBar );

    }

    void ImageBrowser::change_image( int i )
    {
        TRACE( "CHANGE IMAGE TO " << i );

        mrv::Reel reel = current_reel();
        if ( i < 0 ) {
            view()->foreground( mrv::media() );
            view()->background( mrv::media() );
            return;
        }

        if ( size_t(i) >= reel->images.size() ) {
            LOG_ERROR( _("change_image index ") << i << N_(" >= ")
                       << reel->images.size() );
            return;
        }

        mrv::media fg = view()->foreground();
        mrv::media bg = view()->background();
        CMedia* FGimg = NULL, *BGimg = NULL;
        if ( fg ) FGimg = fg->image();
        if ( bg ) BGimg = bg->image();

        int v = value();
        if ( i == v ) {
            TRACE2( "CHANGE IMAGE TO " << i << " == " << v );
            return;
        }


        CMedia::Playback FGplay = CMedia::kStopped;
        if ( FGimg ) FGplay = FGimg->playback();
        CMedia::Playback BGplay = CMedia::kStopped;
        if ( BGimg ) BGplay = BGimg->playback();

        if ( FGplay ) FGimg->stop();
        if ( BGplay ) BGimg->stop( false );

        real_change_image( v, i );

        if ( FGplay ) FGimg->play( FGplay, uiMain, true );
        if ( BGplay ) BGimg->play( BGplay, uiMain, false );

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
                                    const int64_t end,
                                    const double fps )
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
            image_type_ptr canvas;
            img->fetch( canvas, img->first_frame() );
        }
        else
        {
            int64_t f = img->first_frame();
            img->find_image( f );
        }


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

    void ImageBrowser::value( int idx )
    {
        _value = idx;
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
/**
 * Load an image
 *
 * @param name name of image
 * @param start first frame to load
 * @param end   last frame to load
 *
 * @return new image
 */
    mrv::CMedia* ImageBrowser::load_image( const char* name,
                                           const int64_t first,
                                           const int64_t last,
                                           const int64_t start,
                                           const int64_t end,
                                           const double fps,
                                           const bool avoid_seq )
    {

        CMedia* img;
        if ( start != AV_NOPTS_VALUE )
        {
            img = CMedia::guess_image( name, NULL, 0, false,
                                       start, end, avoid_seq );
        }
        else
        {
            img = CMedia::guess_image( name, NULL, 0, false,
                                       first, last, avoid_seq );
        }

        if ( img == NULL )
        {
            return NULL;
        }

        if ( ( img->first_frame() == AV_NOPTS_VALUE ||
               first > img->first_frame() ) && first != AV_NOPTS_VALUE )
        {
            img->first_frame( first );
            img->in_frame( first );
        }

        if ( ( img->last_frame() == AV_NOPTS_VALUE ||
               last < img->last_frame() ) && last != AV_NOPTS_VALUE )
        {
            img->last_frame( last );
            img->out_frame( last );
        }

        if ( fps > 0.0 )
        {
            if ( !img->has_video() )
            {
                img->fps( fps );
                img->play_fps( fps );
            }
#if 0
            else
            {
                if ( !mrv::is_equal( fps, img->fps(), 0.001 ) )
                {
                    double firstfps = img->first_frame() / fps;
                    double lastfps  = img->last_frame() / fps;
                    double firstimg = img->first_frame() / img->fps();
                    double lastimg  = img->last_frame() / img->fps();
                    int64_t first = img->first_frame() * img->fps() / fps;
                    int64_t last  = img->last_frame()  * img->fps() / fps;

                    img->first_frame( first );
                    img->in_frame( first );
                    img->last_frame( last );
                    img->out_frame( last );
                    img->fps( fps );
                    img->play_fps( fps );
                }
            }
#endif
        }

        if ( img->has_video() || img->has_audio() )
        {
            DBGM1( "img->seek( img->first_frame() )" );
            //img->seek( img->first_frame() );
            DBGM1( "img->seeked( img->first_frame() )" );
        }
        else
        {
            int64_t f = img->first_frame();
            img->find_image( f );
        }

        img->default_color_corrections();

        PreferencesUI* prefs = ViewerUI::uiPrefs;
        assert( img->audio_engine() );
        assert( prefs );
        assert( prefs->uiPrefsAudioDevice );
        img->audio_engine()->device( prefs->uiPrefsAudioDevice->value() );

        return img;
    }

    mrv::media ImageBrowser::load_image_in_reel( const char* name,
                                                 const int64_t first,
                                                 const int64_t last,
                                                 const int64_t start,
                                                 const int64_t end,
                                                 const double fps,
                                                 const bool avoid_seq )
    {

        CMedia* img = load_image( name, first, last, start, end, fps,
                                  avoid_seq );
        if ( !img ) return mrv::media();


        mrv::media m = this->add( img );

        return m;
    }

/**
 * Open new image, sequence or movie file(s) from a load list.
 * If stereo is on, every two files are treated as stereo pairs.
 * If bg image is not "", load image as background
 */
    void ImageBrowser::load( const mrv::LoadList& files,
                             const bool stereo,
                             std::string bgimage,
                             const bool edl,
                             const bool progressBar )
    {
        DBGM1( "load LoadList" );
        bool net = view()->network_active();
        view()->update( false );

        if ( bgimage != "" )
        {
            int64_t start = AV_NOPTS_VALUE;
            int64_t end   = AV_NOPTS_VALUE;
            float fps = -1.0;
            if ( mrv::is_valid_sequence( bgimage.c_str() ) )
            {
                fps = 24.0f;
                mrv::get_sequence_limits( start, end, bgimage );
            }
            mrv::media bg = load_image_in_reel( bgimage.c_str(),
                                                start, end, start, end, fps,
                                                false );
            set_bg( bg );
        }

        //
        // Create a progress window
        //
        mrv::Reel oldreel = current_reel();
        size_t  numImages = 0;
        if ( oldreel ) numImages = oldreel->images.size();

        if ( view()->playback() != CMedia::kStopped )
            view()->stop();

        Fl_Window* main = uiMain->uiMain;
        Fl_Window* w = NULL;
        Fl_Progress* progress = NULL;



        mrv::LoadList::const_iterator s = files.begin();
        mrv::LoadList::const_iterator i = s;
        mrv::LoadList::const_iterator e = files.end();

        mrv::media fg;
        int idx = 1;
        char buf[1024];
        for ( ; i != e; ++i, ++idx )
        {
            mrv::LoadInfo& load = (mrv::LoadInfo&) *i;

            if ( files.size() > 10 && progressBar && idx == 2)
            {
                Fl_Group::current( main );
                w = new Fl_Window( main->x(), main->y() + main->h()/2,
                                   main->w(), 80 );
                w->clear_border();
                w->begin();
                progress = new Fl_Progress( 0, 20, w->w(), w->h()-20 );
                progress->minimum( 0 );
                progress->maximum( float(files.size()-1) );
                progress->align( FL_ALIGN_TOP );
                w->end();

                w->show();
                if ( net ) Fl::check();
            }

            if ( w )
            {
                snprintf( buf, 1024, _("Loading \"%s\""), load.filename.c_str() );
                progress->label(buf);
                w->redraw();
                if ( net ) Fl::check();
            }


            if ( load.reel )
            {
                load_reel( load );
                return;
            }
            else if ( load.otio )
            {
                load_otio( load );
                return;
            }
            else
            {
                if ( load.filename == "SMPTE NTSC Color Bars" )
                {
                    fg = ntsc_color_bars( load, this);
                }
                else if ( load.filename == "Black Gap" )
                {
                    fg = black_gap( load, this );
                }
                else if ( load.filename == "PAL Color Bars" )
                {
                    fg = pal_color_bars( load, this);
                }
                else if ( load.filename == "NTSC HDTV Color Bars" )
                {
                    fg = ntsc_hdtv_color_bars( load, this);
                }
                else if ( load.filename == "PAL HDTV Color Bars" )
                {
                    fg = pal_hdtv_color_bars( load, this);
                }
                else if ( load.filename.substr(0,9) == "Checkered" )
                {
                    fg = checkered(load, this);
                }
                else if ( load.filename == "Linear Gradient" )
                {
                    fg = linear_gradient( load, this);
                }
                else if ( load.filename == "Luminance Gradient" )
                {
                    fg = luminance_gradient( load, this);
                }
                else if ( load.filename == "Gamma 1.4 Chart" )
                {
                    fg = gamma_chart( load, this, 1.4f);
                }
                else if ( load.filename == "Gamma 1.8 Chart" )
                {
                    fg = gamma_chart( load, this, 1.8f);
                }
                else if ( load.filename == "Gamma 2.2 Chart" )
                {
                    fg = gamma_chart( load, this, 2.2f);
                }
                else if ( load.filename == "Gamma 2.4 Chart" )
                {
                    fg = gamma_chart( load, this, 2.4f);
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
                                     load.end, load.fps );
                    }
                    else
                    {
                        bool avoid_seq = true;
                        if ( mrv::is_valid_sequence( load.filename.c_str() ) &&
                             load.first != AV_NOPTS_VALUE )
                        {
                            avoid_seq = false;
                        }
                        fg = load_image_in_reel( load.filename.c_str(),
                                                 load.first, load.last,
                                                 load.start,
                                                 load.end, load.fps,
                                                 avoid_seq );
                        if (!fg)
                        {
                            if ( load.filename.rfind( ".session" ) !=
                                 std::string::npos )
                            {
                                load_session( load.filename.c_str() );
                                continue;
                            }
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
                                             load.end, load.fps );
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

                    if ( load.colorspace != "" )
                    {
                        img->ocio_input_color_space( load.colorspace );
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

                    CMedia::Attributes& attrs = img->attributes();
                    if ( load.replace_attrs )
                    {
                        attrs.clear();
                    }
                    auto ati = load.attrs.begin();
                    auto ate = load.attrs.end();
                    for ( ; ati != ate; ++ati )
                    {
                        attrs.insert( *ati );
                    }


                    std::string amf = aces_amf_filename( img->fileroot() );
                    bool ok = load_amf( img, amf.c_str() );
                    if ( ok == false )
                    {
                        std::string xml = aces_xml_filename( img->fileroot() );
                        load_aces_xml( img, xml.c_str() );
                    }
                }
            }

            if ( w )
            {
                progress->value( progress->value() + 1 );
                if ( net ) Fl::check();
            }
        }

        if ( w )
        {
            w->hide();
            delete w;
            if ( net ) Fl::check();
        }

        if ( edl )
        {
            current_reel()->edl = true;
            uiMain->uiTimeline->edl( true );
        }

        match_tree_order();
        int64_t first, last;
        adjust_timeline( first, last );
        set_timeline( first, last );

        mrv::EDLGroup* eg = edl_group();
        if ( eg )
        {
            eg->refresh();
            eg->redraw();
        }

        if ( view() )
        {
            view()->update(true);
            view()->redraw();
            if ( net ) Fl::check();

            view()->reset_caches(); // Redo preloaded sequence caches
            view()->fit_image();
        }

        mrv::Reel reel = current_reel();
        if ( reel->images.empty() ) return;


        mrv::media m = current_image();
        if (!m) return;

        CMedia* img = m->image();

        // If loading images to old non-empty reel, display last image.
        if ( reel == oldreel && numImages > 0 )
        {
            this->change_image( (int)reel->images.size()-1 );
        }
        else
        {
            // display first image for good EDL playback
            this->change_image( 0 );
        }

        int64_t f = reel->local_to_global( img->first_frame(), img );
        frame( f );

        if ( ( reel->edl || img->first_frame() != img->last_frame() )
             && uiMain->uiPrefs->uiPrefsAutoPlayback->value() )
        {
            bool b = view()->network_active();
            view()->network_active(true);
            view()->play_forwards();
            view()->network_active(b);
        }

    }

    void ImageBrowser::open_session()
    {
        std::string file = mrv::open_session(NULL, uiMain);
        if ( file.empty() ) return;

        load_session( file.c_str() );
    }

    void ImageBrowser::load_session( const char* name )
    {
        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );


        fs::path orig = fs::current_path();


        FILE* f = fl_fopen( name, "r" );
        if (!f ) {
            setlocale( LC_NUMERIC, oldloc );
            av_free( oldloc );
            return;
        }

        clear_reels();

        fs::path path = name;
        fs::path dir = path.parent_path();
        fs::current_path( dir );

        uiMain->uiReelWindow->uiMain->hide();
        uiMain->uiImageInfo->uiMain->hide();
        uiMain->uiColorArea->uiMain->hide();
        uiMain->uiColorControls->uiMain->hide();
        uiMain->uiPaint->uiMain->hide();
        uiMain->uiEDLWindow->uiMain->hide();
        uiMain->uiHistogram->uiMain->hide();
        uiMain->uiVectorscope->uiMain->hide();

        char buf[2048];
        float version = 6.0;
        while ( !feof(f) )
        {
            char* c = NULL;
            while ( (c = fgets( buf, 2047, f )) )
            {
                if ( c[0] == '#' ) continue;  // comment line
                // skip starting whitespace
                while ( *c != 0 && ( *c == ' ' || *c == '\t' ) ) ++c;
                if ( strlen(c) <= 1 ) continue; // empty line
                c[ strlen(c)-1 ] = 0;  // remove newline

                if ( strncmp( "Version ", c, 8 ) == 0)
                {
                    version = atof( c+8 );
                    if ( version < 6 )
                        LOG_ERROR( "Invalid version " << version );
                    continue;
                }
                else if ( strncmp( "ReelWindow", c, 10 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d\n", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        this->window()->show();
                        this->window()->resize( x, y, w, h );
                    }
                    else
                    {
                        this->window()->hide();
                    }
                    continue;
                }
                else if ( strncmp( "InfoWindow", c, 10 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiImageInfo->uiMain->show();
                        uiMain->uiImageInfo->uiMain->resize( x, y, w, h );
                    }
                    else
                    {
                        uiMain->uiImageInfo->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "ColorInfo", c, 9 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiColorArea->uiMain->show();
                        uiMain->uiColorArea->uiMain->resize( x, y, w, h );
                    }
                    else
                    {
                        uiMain->uiColorArea->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "ColorControls", c, 13 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h, active;
                    float hue, brightness, contrast, saturation;
                    sscanf( c, "%s %d %d %d %d %d %d %g %g %g %g", cmd,
                            &on, &x, &y, &w, &h,
                            &active, &hue, &brightness, &contrast, &saturation);
                    ColorControlsUI* cc = uiMain->uiColorControls;
                    cc->uiActive->value( active );
                    cc->uiHue->value( hue );
                    cc->uiBrightness->value( brightness );
                    cc->uiContrast->value( contrast );
                    cc->uiSaturation->value( saturation );
                    if ( on )
                    {
                        cc->uiMain->show();
                        cc->uiMain->resize( x, y, w, h );
                    }
                    else
                    {
                        cc->uiActive->value( 0 );
                        cc->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "Histogram", c, 9 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h, ch, type;
                    sscanf( c, "%s %d %d %d %d %d %d %d", cmd, &on,
                            &x, &y, &w, &h, &ch, &type );
                    HistogramUI* hg = uiMain->uiHistogram;
                    if ( on )
                    {
                        hg->uiHistogram->channel( (mrv::Histogram::Channel)ch );
                        hg->uiHistogram->histogram_type(
                            (mrv::Histogram::Type)type );
                        hg->uiMain->resize( x, y, w, h );
                        hg->uiMain->show();
                    }
                    else
                    {
                        hg->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "Waveform", c, 8 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiWaveform->uiMain->resize( x, y, w, h );
                        uiMain->uiWaveform->uiMain->show();
                    }
                    else
                    {
                        uiMain->uiWaveform->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "Vectorscope", c, 11 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiVectorscope->uiMain->resize( x, y, w, h );
                        uiMain->uiVectorscope->uiMain->show();
                    }
                    else
                    {
                        uiMain->uiVectorscope->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "Paint", c, 5 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiPaint->uiMain->resize( x, y, w, h );
                        uiMain->uiPaint->uiMain->show();
                    }
                    else
                    {
                        uiMain->uiPaint->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "EDLWindow", c, 9 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiEDLWindow->uiMain->resize( x, y, w, h );
                        uiMain->uiEDLWindow->uiMain->show();
                    }
                    else
                    {
                        uiMain->uiEDLWindow->uiMain->hide();
                    }
                    continue;
                }
                else if ( strncmp( "AttrsWindow", c, 11 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( !uiMain->uiAttrsWindow )
                    {
                        uiMain->uiAttrsWindow = uiMain->make_attrs_window();
                    }
                    if ( on )
                    {
                        uiMain->uiAttrsWindow->resize( x, y, w, h );
                        uiMain->uiAttrsWindow->show();
                    }
                    else
                    {
                        uiMain->uiAttrsWindow->hide();
                    }
                    continue;
                }
                else if ( strncmp( "GL3dView", c, 8 ) == 0 )
                {
                    char cmd[20];
                    int on, x, y, w, h;
                    sscanf( c, "%s %d %d %d %d %d", cmd, &on, &x, &y, &w, &h );
                    if ( on )
                    {
                        uiMain->uiGL3dView->uiMain->resize( x, y, w, h );
                        uiMain->uiGL3dView->uiMain->show();
                    }
                    else
                {
                    uiMain->uiGL3dView->uiMain->hide();
                }
                continue;
            }
            else if ( strncmp( "Stereo", c, 6 ) == 0 )
            {
                char cmd[20];
                int on, x, y, w, h, vs360, vc360, si, so;
                sscanf( c, "%s %d %d %d %d %d %d %d %d %d", cmd,
                        &on, &x, &y, &w, &h, &vs360, &vc360, &si, &so );

                StereoUI* s = uiMain->uiStereo;
                s->uiVR360Sphere->value( vs360 );
                s->uiVR360Cube->value( vc360 );
                s->uiStereoOutput->value( so );
                s->uiStereoInput->value( si );

                mrv::media fg = uiMain->uiView->foreground();
                if (fg)
                {
                    CMedia* img = fg->image();
                    img->stereo_output( mrv::CMedia::to_stereo_output( so ) );
                    img->stereo_input( mrv::CMedia::to_stereo_input( si ) );
                }
                if ( on )
                {
                    s->uiMain->resize( x, y, w, h );
                    s->uiMain->show();
                }
                else
                {
                    s->uiMain->hide();
                }
                continue;
            }
            else if ( strncmp( "Selection", c, 9 ) == 0 )
            {
                char cmd[20];
                float x,y,w,h;
                mrv::Rectd r;
                sscanf( c, "%s %g %g %g %g", cmd, &x, &y, &w, &h );
                r.x( x ); r.y( y ); r.w( w ); r.h( h );
                uiMain->uiView->selection( r );
                continue;
            }
                LoadInfo load( c );
                load_reel( load );
        }
    }

    fclose(f);

    fs::current_path( orig );
    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

}


void append_attributes( const LoadInfo& info, LoadList& sequences )
{
    LoadList::iterator i = sequences.begin();
    LoadList::iterator e = sequences.end();
    for ( ; i != e; ++i )
    {
        LoadInfo& d = *i;
        d.replace_attrs = info.replace_attrs;

        auto ai = info.attrs.begin();
        auto ae = info.attrs.end();
        for ( ; ai != ae; ++ai )
        {
            d.attrs.insert( std::make_pair( ai->first, ai->second->copy() ) );
        }
    }
}

/**
 * Load an image reel
 *
 * @param name name of reel to load
 */
void ImageBrowser::load_otio( const LoadInfo& info )
{
    _loading = true;
    bool edl = true;

    fs::path path( info.filename );
    std::string reelname = path.leaf().string();
    reelname = reelname.substr(0, reelname.size()-5);

    new_reel( reelname.c_str() );
    mrv::Reel reel = current_reel();

    mrv::LoadList sequences;
    if ( ! parse_otio( sequences, reel->transitions, info.filename.c_str() ) )
    {
        LOG_ERROR( "Could not parse \"" << info.filename << "\"." );
        _loading = false;
        return;
    }

    append_attributes( info, sequences );

    load( sequences, false, "", edl, true );

    _loading = false;

    if ( reel->images.empty() ) return;

    set_edl();


    // Initialize images as if they had no transitions
    for ( const auto& i : reel->images )
    {
        CMedia* img = i->image();
        img->in_frame( img->first_frame() );
        img->out_frame( img->last_frame() );
    }

    if ( reel->transitions.empty() ) return;

    // Initialize transitions entry points
    for ( const auto& t : reel->transitions )
    {
        int64_t start = t.start();
        int64_t   end = t.end();
        CMedia* Aimg = reel->image_at( start );
        CMedia* Bimg = reel->image_at( end );
        if ( !Bimg || !Aimg ) continue;



        int64_t Bend  = reel->global_to_local( end );
        int64_t Astart = reel->global_to_local( start );

        int64_t outlen = Bend - Bimg->first_frame();
        int64_t  inlen = Aimg->last_frame() - Astart;

        int64_t out = Aimg->last_frame() + outlen;
        int64_t in  = Bimg->first_frame() - inlen;

        if ( dynamic_cast< BlackImage* >( Aimg ) )
        {
            if ( out > Aimg->end_frame() )
            {
                Aimg->end_frame( out );
            }
        }
        if ( dynamic_cast< BlackImage* >( Bimg ) )
        {
            if ( in < Bimg->start_frame() )
            {
                Bimg->start_frame( in );
            }
        }


        Aimg->out_frame( out );
        Aimg->play_fps( Aimg->fps() );

        Bimg->in_frame( in );
        Bimg->seek( Bimg->in_frame() ); // prepare image


        TRACE( "start " << start << " end " << end );

        TRACE( "Aimg " << Aimg->first_frame() << " - " << Aimg->last_frame() );
        TRACE( "Bimg " << Bimg->name() << " " << Bimg->first_frame() << " - " << Bimg->last_frame() );
        TRACE( "G Aimg "
                  << reel->local_to_global( Aimg->first_frame(), Aimg ) << " - "
                  << reel->local_to_global( Aimg->last_frame(), Aimg )
                  );
        TRACE( "G Bimg " << Bimg->name() << " "
                  << reel->local_to_global( Bimg->first_frame(), Bimg ) << " - "
                  << reel->local_to_global( Bimg->last_frame(), Bimg )
                  );
        TRACE( "I/O Aimg " << Aimg->in_frame() << " - " << Aimg->out_frame() );
        TRACE( "I/O Bimg " << Bimg->name() << " "<< Bimg->in_frame() << " - " << Bimg->out_frame() );
        TRACE( "GI/O Aimg "
                  << reel->local_to_global( Aimg->in_frame(), Aimg ) << " - "
                  << reel->local_to_global( Aimg->out_frame(), Aimg )
                  );
        TRACE( "GI/O Bimg " << Bimg->name() << " "
                  << reel->local_to_global( Bimg->in_frame(), Bimg ) << " - "
                  << reel->local_to_global( Bimg->out_frame(), Bimg )
                  );
    }

}

/**
 * Load an image reel
 *
 * @param name name of reel to load
 */
void ImageBrowser::load_reel( const LoadInfo& info )
{
    _loading = true;
    bool edl;
    mrv::LoadList sequences;
    short previous, next;
    if ( ! parse_reel( sequences, edl, previous, next, info.filename.c_str() ) )
    {
        LOG_ERROR( "Could not parse \"" << info.filename << "\"." );
        _loading = false;
        return;
    }

    view()->ghost_previous( previous );
    view()->ghost_next( next );

    append_attributes( info, sequences );

    fs::path path( info.filename );
    std::string reelname = path.leaf().string();
    reelname = reelname.substr(0, reelname.size()-5);

    new_reel( reelname.c_str() );
    load( sequences, false, "", edl, true );

    mrv::Reel reel = current_reel();

    _loading = false;
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
                         const bool edl,
                         const bool progress )
{
    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();

    mrv::LoadList loadlist;

    for ( ; i != e; ++i )
    {
        std::string file = *i;

        if ( file.substr(0, 7) == "file://" )
            file = file.substr( 7, file.size() );


        if ( file.empty() ) continue;

        size_t len = file.size();
        if ( len > 5 && ( file.substr( len - 5, 5 ) == ".reel" ||
                          file.substr( len - 5, 5 ) == ".otio" ) )
        {
            loadlist.push_back( mrv::LoadInfo( file ) );
        }
        else
        {
            int64_t start = AV_NOPTS_VALUE;
            int64_t end   = AV_NOPTS_VALUE;

            std::string fileroot = file;
            if ( seqs )
            {
                mrv::fileroot( fileroot, file );
                mrv::get_sequence_limits( start, end, fileroot );
            }
            loadlist.push_back( mrv::LoadInfo( fileroot, start, end ) );
        }

        retname = file;
    }

    load( loadlist, stereo, bgfile, edl, progress );
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

/**
 * Open new image, sequence or movie file(s).
 *
 */
void ImageBrowser::open_amf()
{

    Fl_Tree_Item* item = first_selected_item();
    if ( !item ) return;

    mrv::Element* e = (mrv::Element*) item->widget();
    mrv::media m = e->media();
    mrv::CMedia* img = m->image();

    std::string file = mrv::open_amf_file(img, uiMain);
    if (file.empty()) return;

    for ( ; item; item = next_selected_item(item) )
    {
        e = (mrv::Element*) item->widget();
        m = e->media();
        img = m->image();
        load_amf( img, file.c_str() );
    }

    view()->redraw();
}

void ImageBrowser::open_stereo()
{
    stringArray files = mrv::open_image_file(NULL,true, uiMain);
    if (files.empty()) return;

    if (files.size() > 1 )
    {
        LOG_ERROR( _("You can load only a single stereo image to combine with current one.") );
        return;
    }

    mrv::LoadList loadlist;

    std::string file = files[0];

    if ( file.substr(0, 7) == "file://" )
        file = file.substr( 7, file.size() );

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
                 loadlist[0].start, loadlist[0].end, loadlist[0].fps );
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


    mrv::media orig = reel->images[sel];
    CMedia* img = orig->image();
    if ( img == NULL ) return;

    clonedImage* copy = new clonedImage( img );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    char buf[256];
    std::string file = img->directory() + '/' + img->name();
    sprintf(buf, N_("CloneImage \"%s\""), file.c_str() );
    view()->send_network( buf );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );
}


void ImageBrowser::set_timeline( const int64_t& first, const int64_t& last )
{
    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->minimum( double(first) );
        t->maximum( double(last) );
        t->redraw();
    }
    uiMain->uiStartFrame->value( first );
    uiMain->uiEndFrame->value( last );
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

    mrv::media orig = reel->images[sel];
    CMedia* img = orig->image();
    if (!orig || !img ) return;

    // @hmm.... this needs fixing
    stubImage* copy = new stubImage( img );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    char buf[256];
    sprintf(buf, N_("CloneImageAll \"%s\""), img->fileroot() );
    view()->send_network( buf );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );
}


/**
 * Remove and delete current image buffer being displayed.
 *
 */
void ImageBrowser::remove_current()
{
    mrv::Reel reel = current_reel();
    if (!reel || reel->images.empty() ) return;


    Fl_Tree_Item_Array items;
    int num = get_selected_items( items );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )  view()->stop();

    int ok;
    if ( num == 1 )
        ok = mrv::fl_choice( _( "Are you sure you want to\n"
                                "remove image from reel?" ),
                             _("Yes"), _("No"), NULL );
    else
        ok = mrv::fl_choice( _( "Are you sure you want to\n"
                                "remove all selected images from reel?" ),
                             _("Yes"), _("No"), NULL );
    if ( ok == 1 ) return; // No

    for ( int i = 0; i < num; ++i )
    {
        Fl_Tree_Item* item = items[i];
        if ( ! item ) continue;
        mrv::Element* elem = (mrv::Element*) item->widget();
        mrv::media m = elem->media();
        remove( m );
    }



    // mrv::Reel r = current_reel();
    // int v = value();

    // int sel = v;

    // match_tree_order();

    // if ( sel >= (int) r->images.size() )
    //     sel = r->images.size() - 1;

    // real_change_image( v, sel, play );

    // view()->fit_image();

    // int64_t first, last;
    // adjust_timeline( first, last );
    // set_timeline( first, last );

    // redraw();
}


void ImageBrowser::clear_items()
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    size_t num = reel->images.size();
    for ( size_t i = 0; i < num; ++i )
    {
        mrv::media m = reel->images[i];
        Fl_Tree_Item* item = media_to_item( m );
        if (!item) continue;
        mrv::Element* elem = (mrv::Element*) item->widget();
        elem->Label()->box( FL_NO_BOX );
        elem->redraw();
    }

    mrv::EDLGroup* e = edl_group();
    if ( e )
    {
        e->refresh();
        e->redraw();
    }

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
        DBGM3( "BG REEL ************* " << -1 );
        clear_bg();
        uiMain->uiBButton->value(0);
    }
    else
    {
        DBGM3( "BG REEL ************* " << _reel << "  SEL " << sel
             << " " << reel->images[sel]->image()->name() );
        clear_items();

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

    const Fl_Tree_Item* item = media_to_item( m );
    if ( item->widget() )
    {
        item->widget()->redraw();
    }
}

void ImageBrowser::replace( int i, mrv::media m )
{
    mrv::Reel reel = current_reel();
    if (!reel) return;

    if ( i < 0 || size_t(i) >= reel->images.size() ) {
        LOG_ERROR( _("Replace image index is out of range") );
        return;
    }

    mrv::media fg = reel->images[i];

    CMedia::Playback play = view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();


    // Sanely remove item from tree
    Fl_Tree_Item* item = media_to_item( fg );
    if ( !item )
    {
        LOG_ERROR( _("Image item not found for ") << fg->image()->name() );
        return;
    }
    Element* oldnw = (Element*)item->widget();
    int X = oldnw->x();
    int Y = oldnw->y();
    int W = oldnw->w();
    int H = oldnw->h();
    delete oldnw;  // Delete old element and item
    Fl_Tree::remove( item );

    // Create new item
    std::string newpath = media_to_pathname( m );
    Element* nw = new_item( m );

    // Insert new item and select it
    Fl_Tree_Item* newitem = Fl_Tree::insert( root(), newpath.c_str(), i );
    if ( !newitem )
    {
        LOG_ERROR( _("New item not created at ") << i << " " << newpath );
        return;
    }
    Fl_Tree::select( newitem, 0 );

    // We resize new widget to old to avoid redraw issues
    nw->resize( X, Y, W, H );

    // We attach widget to tree
    newitem->widget( nw );

    redraw();
    window()->redraw();

    //match_tree_order();

    // Sanely remove image from reel
    mrv::MediaList::iterator j = reel->images.begin();
    reel->images.erase( j + i );


    // Insert item in right place on list
    j = reel->images.begin() + i;
    reel->images.insert( j, m );


    view()->foreground( m );

    char buf[1024];
    CMedia* img = m->image();
    std::string file = img->directory() + '/' + img->name();
    sprintf( buf, "ReplaceImage %d \"%s\"", i, file.c_str() );
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

    image_version( i, sum, fg );

    if ( play ) view()->play(play);
}



void ImageBrowser::image_version( size_t i, int sum, mrv::media fg,
                                  bool max_files )
{

    short add = sum;
    unsigned short tries = 0;
    int64_t start = AV_NOPTS_VALUE;
    int64_t end   = AV_NOPTS_VALUE;
    PreferencesUI* prefs = main()->uiPrefs;
    std::string newfile, loadfile;
    int64_t     num = 1;

    std::string suffix;
    static std::string short_prefix;
    std::string prefix = prefs->uiPrefsImageVersionPrefix->value();
    if ( prefix.empty() )
    {
        LOG_ERROR( _("Prefix cannot be an empty string.  Please type some regex to distinguish the version in the filename.  If unsure, use _v.") );
        return;
    }

    if ( prefix.size() < 5 )
    {
        short_prefix = prefix;
        LOG_INFO( _("Regex ") << prefix <<
                  (" replaced by complex regex.") );
        prefix = "([\\w:/\\\\]*?[/\\\\._]*" + prefix +
                 ")(\\d+)([%\\w\\d./\\\\]*)";
    }
    prefs->uiPrefsImageVersionPrefix->value( prefix.c_str() );

    boost::regex expr;
    try
    {
        expr = prefix;
    }
    catch ( const boost::regex_error& e )
    {
        LOG_ERROR( _("Regular expression error: ") << e.what()  );
        return;
    }

    CMedia* img = fg->image();
    unsigned max_tries = (unsigned) prefs->uiPrefsMaxImagesApart->value();
    while ( (max_files || start == AV_NOPTS_VALUE) && tries <= max_tries )
    {
        std::string file = img->fileroot();
        file = fs::path( file ).leaf().string();
        std::string dir = img->directory();
        file = dir + "/" + file;

        std::string::const_iterator tstart, tend;
        tstart = file.begin();
        tend = file.end();
        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;

        newfile.clear();

        try
        {
            while ( boost::regex_search( tstart, tend, what, expr, flags ) )
            {
                std::string prefix = what[1];
                std::string number = what[2];
                suffix = what[3];

                LOG_INFO( "Matched prefix=" << prefix );
                LOG_INFO( "Matched number=" << number );
                LOG_INFO( "Matched suffix=" << suffix );

                newfile += prefix;

                if ( !number.empty() )
                {
                    int padding = int( number.size() );
                    num = atoi( number.c_str() );
                    char buf[128];
                    sprintf( buf, "%0*" PRId64, padding, num + sum );
                    newfile += buf;
                }

                tstart = what[3].first;
                flags |= boost::match_prev_avail;
                flags |= boost::match_not_bob;
            }
        }
        catch ( const boost::regex_error& e )
        {
            LOG_ERROR( _("Regular expression error: ") << e.what()  );
        }


        if ( newfile.empty() )
        {
            LOG_ERROR( _("No versioning in this clip.  "
                         "Please create an image or directory named with "
                         "a versioning string." ) );

            LOG_ERROR( _("Example:  gizmo") + short_prefix + N_("003.0001.exr") );
            return;
        }

        newfile += suffix;


        if ( mrv::is_valid_sequence( newfile.c_str() ) )
        {
            mrv::get_sequence_limits( start, end, newfile, false );
            if ( start != AV_NOPTS_VALUE ) {
                char fmt[1024], buf[1024];
                sprintf( fmt, "%s", newfile.c_str() );
                sprintf( buf, fmt, start );
                if ( fs::exists( buf ) )
                {
                    loadfile = newfile;
                }
            }
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
                    loadfile = newfile;
                    start = 1;
                    if ( !max_files ) break;
                }
            }
        }

        ++tries;
        sum += add;
    }

    if ( start == AV_NOPTS_VALUE  )
    {
        int64_t num2 = num;
        if ( add > 0 ) {
            num++;
            num2 += sum;
        }
        else {
            num--;
            num2 += sum;
        }
        if ( sum > 0 )
            LOG_ERROR( img->name()
                       << _(" is already the last version on disk." ) );
        else
            LOG_ERROR( img->name()
                       << _(" is already the first version on disk." ) );
        return;
    }

    CMedia* newImg = load_image( loadfile.c_str(),
                                 start, end, start, end, img->fps(), false );
    if ( !newImg ) return;



    mrv::media m( new mrv::gui::media( newImg ) );
    m->image()->channel( img->channel() );

    int64_t frame = img->frame();

    m->position( fg->position() );

    // Transfer all attributes to new image
    newImg->idt_transform( img->idt_transform() );
    newImg->inverse_ot_transform( img->inverse_ot_transform() );
    newImg->inverse_odt_transform( img->inverse_odt_transform() );
    newImg->inverse_rrt_transform( img->inverse_rrt_transform() );
    for (size_t i = 0; i < img->number_of_lmts(); ++i )
    {
        newImg->append_look_mod_transform( img->look_mod_transform( i ) );
    }
    newImg->asc_cdl( img->asc_cdl() );
    newImg->rendering_transform( img->rendering_transform() );
    newImg->ocio_input_color_space( img->ocio_input_color_space() );
    newImg->gamma( img->gamma() );
    newImg->fps( img->fps() );
    newImg->play_fps( img->play_fps() );
    newImg->seek( img->frame() );
    if ( img->frame() < newImg->first_frame() )
        newImg->seek( newImg->first_frame() );
    if ( img->frame() > newImg->last_frame() )
        newImg->seek( newImg->last_frame() );
    // newImg->decode_video( frame );
    // newImg->find_image( frame );
    timeline()->value( newImg->frame() );
    uiMain->uiFrame->value( newImg->frame() );
    view()->frame( newImg->frame() );
    view()->update_layers();


    // Sanely remove icon item from browser and replace it with another
    this->replace( int(i), m );

    change_image( int(i) );


    mrv::EDLGroup* e = edl_group();

    if (e)
    {
        e->refresh();
        e->redraw();
    }



    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );


}


/**
 * Go to next image in browser's list
 *
 */
void ImageBrowser::next_image()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    DBGM3( "reel name " << reel->name );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int v = value();

    ++v;
    if ( size_t(v) >= reel->images.size() )
    {
        if ( play ) view()->play(play);
        return;
    }

    mrv::media orig = reel->images[v-1];
    if ( orig )
    {
        CMedia* img = orig->image();
        if ( img )
        {
            img->close_audio();
        }
    }

    Fl_Tree_Item* item = root()->child(v-1);
    int ok = deselect( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Old item was not found in tree.") );
        return;
    }

    value( v );

    if ( v < 0 || v >= (int)reel->images.size() )
        return;

    mrv::media m = reel->images[v];


    item = root()->child(v);
    ok = select( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("New item was not found in tree.") );
        return;
    }

    add_menu( main()->uiReelWindow->uiMenuBar );

    view()->foreground( m );


    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    send_image( v );

    CMedia* img = nullptr;
    if ( m ) img = m->image();

    if ( reel->edl && img )
    {
        int64_t pos = m->position();
        DBGM3( "seek to " << pos );
        seek( pos );
    }
    else
    {
        seek( view()->frame() );
    }

    if ( play ) view()->play(play);
}



/**
 * Go to next image in browser's list
 *
 */
void ImageBrowser::next_image_limited()
{
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    DBGM3( "reel name " << reel->name );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int v = value();

    ++v;
    if ( size_t(v) >= reel->images.size() )
    {
        if ( play ) view()->play(play);
        return;
    }

    mrv::media orig = reel->images[v-1];
    if ( orig )
    {
        CMedia* img = orig->image();
        if ( img )
        {
            img->close_audio();
        }
    }

    Fl_Tree_Item* item = root()->child(v-1);
    int ok = deselect( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Old item was not found in tree.") );
        return;
    }

    value( v );

    if ( v < 0 || v >= (int)reel->images.size() )
        return;

    mrv::media m = reel->images[v];

    item = root()->child(v);
    ok = select( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("New item was not found in tree.") );
        return;
    }

    view()->foreground( m );


    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    send_image( v );

    if ( reel->edl )
    {
        first = m->position();
        last = first + m->image()->duration() - 1;
        uiMain->uiTimeline->display_minimum( first );
        uiMain->uiStartFrame->value( first );
        uiMain->uiTimeline->display_maximum( last );
        uiMain->uiEndFrame->value( last );
    }

    seek( first );


    if ( play ) view()->play(play);
}


/**
 * Go to previous image in browser's list
 *
 */
void ImageBrowser::previous_image()
{
    mrv::Reel reel = current_reel();

    DBGM3( "reel name " << reel->name );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int v = value();
    --v;
    if ( v < 0 )
    {
        if ( play ) view()->play(play);
        return;
    }

    mrv::media orig = reel->images[v];
    if ( orig )
    {
        CMedia* img = orig->image();
        if ( img )
        {
            img->close_audio();
        }
    }



    value( v );

    if ( v < 0 || (v+1 >= (int)reel->images.size()) )
        return;

    Fl_Tree_Item* item = root()->child( v+1 );
    int ok = deselect( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Item was not found in tree.") );
        return;
    }

    item = root()->child( v );
    ok = select( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Item was not found in tree.") );
        return;
    }

    add_menu( main()->uiReelWindow->uiMenuBar );

    mrv::media m = reel->images[v];
    view()->foreground( m );


    send_image( v );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    CMedia* img = NULL;
    if ( m ) img = m->image();
    if ( reel->edl && img )
    {
        int64_t pos = m->position();
        DBGM3( "seek to " << pos );
        seek( pos );
    }
    else
    {
        seek( view()->frame() );
    }

    if ( play ) view()->play(play);
}


/**
 * Go to previous image in browser's list
 *
 */
void ImageBrowser::previous_image_limited()
{
    mrv::Reel reel = current_reel();

    DBGM3( "reel name " << reel->name );

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int v = value();
    --v;
    if ( v < 0 )
    {
        if ( play ) view()->play(play);
        return;
    }

    mrv::media orig = reel->images[v];
    if ( orig )
    {
        CMedia* img = orig->image();
        if ( img )
        {
            img->close_audio();
        }
    }



    value( v );

    if ( v < 0 || (v+1 >= (int)reel->images.size()) )
        return;
    Fl_Tree_Item* item = root()->child( v+1 );
    int ok = deselect( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Item was not found in tree.") );
        return;
    }

    item = root()->child( v );
    ok = select( item, 0 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("Item was not found in tree.") );
        return;
    }

    mrv::media m = reel->images[v];
    view()->foreground( m );


    send_image( v );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    if ( reel->edl )
    {
        first = m->position();
        last = first + m->image()->duration() - 1;
        uiMain->uiTimeline->display_minimum( first );
        uiMain->uiStartFrame->value( first );
        uiMain->uiTimeline->display_maximum( last );
        uiMain->uiEndFrame->value( last );
    }

    seek( first );

    if ( play ) view()->play(play);
}

    void ImageBrowser::add_menu( Fl_Menu_* menu )
    {

        menu->clear();
        menu->add( _("File/Open/Movie or Sequence"), kOpenImage.hotkey(),
                   (Fl_Callback*)open_cb, this);
        menu->add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
                   (Fl_Callback*)open_single_cb, this);
        int idx = menu->add( _("Select/Single Image for Dragging"),
                             kSelectSingleImage.hotkey(),
                             (Fl_Callback*)select_single_cb, this,
                             FL_MENU_RADIO|FL_MENU_VALUE );
        Fl_Menu_Item* item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( selectmode() == FL_TREE_SELECT_MULTI )
        {
            item->clear();
        }
        else
        {
            item->set();
        }
        idx = menu->add( _("Select/Multiple Images for Modifying"),
                         kSelectMultiImage.hotkey(),
                         (Fl_Callback*)select_multi_cb, this,
                         FL_MENU_RADIO);
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( selectmode() == FL_TREE_SELECT_MULTI )
        {
            item->set();
        }
        else
        {
            item->clear();
        }

        if ( Preferences::use_ocio )
        {
            menu->add( _("OCIO/Input Color Space"),
                       kOCIOInputColorSpace.hotkey(),
                       (Fl_Callback*)attach_ocio_ics_cb, this);
        }
        else
        {
            // CTL / ICC
            menu->add( _("CTL/Attach Input Device Transform"),
                       kIDTScript.hotkey(),
                       (Fl_Callback*)attach_ctl_idt_script_cb,
                       this );
            menu->add( _("CTL/Attach Rendering Transform"),
                        kCTLScript.hotkey(),
                        (Fl_Callback*)attach_ctl_rrt_script_cb,
                        this, FL_MENU_DIVIDER);
        }


        bool has_version = false;

        PreferencesUI* prefs = main()->uiPrefs;
        std::string prefix = prefs->uiPrefsImageVersionPrefix->value();
        if ( prefix.empty() )
        {
            LOG_ERROR( _("Prefix cannot be an empty string.  Please type some unique characters to distinguish the version in the filename.") );
            return;
        }

        static int count = 0;

        Fl_Tree_Item* i = NULL;
        for ( i = first_selected_item(); i; i = next_selected_item(i) )
        {
            if ( ! i->widget() ) {
                continue;
            }

            mrv::Element* elem = (mrv::Element*) i->widget();
            mrv::media m = elem->media();
            if ( m ) {
                CMedia* img = m->image();
                const std::string& name = img->fileroot();
                if ( name.find( prefix ) != std::string::npos )
                {
                    has_version = true;
                    break;
                }
            }
        }

       if ( has_version )
        {
            menu->add( _("Version/First"), kFirstVersionImage.hotkey(),
                       (Fl_Callback*)first_image_version_cb, this,
                       FL_MENU_DIVIDER);
            menu->add( _("Version/Previous"),
                       kPreviousVersionImage.hotkey(),
                       (Fl_Callback*)previous_image_version_cb, this );
            menu->add( _("Version/Next"), kNextVersionImage.hotkey(),
                       (Fl_Callback*)next_image_version_cb, this,
                       FL_MENU_DIVIDER );
            menu->add( _("Version/Last"), kLastVersionImage.hotkey(),
                       (Fl_Callback*)last_image_version_cb, this );
        }
        menu->menu_end();

        menu->redraw();
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
    DBG;

    CMedia::Playback play = (CMedia::Playback) view()->playback();
    if ( play != CMedia::kStopped )
        view()->stop();

    int ok = 0;
    int button = Fl::event_button();

    if ( button == FL_LEFT_MOUSE )
    {
    DBG;
        int clicks = Fl::event_clicks();
        lastX = x;
        lastY = y;
        DBG;

        dragging = callback_item();

        if ( selectmode() == FL_TREE_SELECT_SINGLE_DRAGGABLE )
        {
            DBG;
            if ( (! dragging) || (! dragging->widget()) ) return 0;

            DBG;
            if ( dragging == old_dragging && clicks > 0 )
            {
                DBG;
                Fl::event_clicks(0);
                redraw();
                uiMain->uiImageInfo->uiMain->show();
                view()->update_image_info();
                if ( play != CMedia::kStopped )
                    view()->play( play );
                return 1;
            }

            old_dragging = dragging;

            int ok = Fl_Tree::deselect_all( NULL, 0 );
            if ( ok < 0 )
            {
                LOG_ERROR( "Could not deselect all" );
            }

        }

        if (!dragging) return 0;

        ok = Fl_Tree::select( dragging, 0 );
        if ( ok < 0 )
        {
            LOG_ERROR( "Could not select " << dragging->label() );
        }


        DBGM1( "DRAGGING LEFT MOUSE BUTTON " << dragging->label() );

        mrv::Reel reel = current_reel();
        if ( !reel ) return 0;
        mrv::Element* e = (mrv::Element*) dragging->widget();
        assert0( e != NULL );
        mrv::media m = e->media();
        assert0( m );
        view()->foreground( m );


        CMedia* img = NULL;
        if ( m ) img = m->image();
        if ( reel->edl && img )
        {
            int64_t pos = m->position() - img->first_frame() + img->frame();
            DBGM3( "seek to " << pos );
            seek( pos );
        }
        else
        {
            seek( view()->frame() );
        }

        if ( m == view()->background() )
        {
            uiMain->uiBButton->value(1);
        }
        else
        {
            uiMain->uiBButton->value(0);
        }

        match_tree_order();

        int64_t first, last;
        adjust_timeline( first, last );
        set_timeline( first, last );

        if ( play != CMedia::kStopped )
            view()->play( play );
        return 1;
    }
    else if ( button == FL_RIGHT_MOUSE )
    {

        Fl_Menu_Button menu( Fl::event_x(), Fl::event_y(),0,0);

        mrv::Reel reel = current_reel();
        if ( !reel ) return 0;

        CMedia* img = NULL;
        bool valid = false;

        add_menu( &menu );

        match_tree_order();

        int64_t first, last;
        adjust_timeline( first, last );

        int sel = value();

        if ( sel >= 0 )
        {
            change_image(sel);

            mrv::media m = reel->images[sel];
            img = m->image();

            menu.add( _("File/Save/Movie or Sequence As"),
                      kSaveSequence.hotkey(),
                      (Fl_Callback*)save_sequence_cb, view() );
            menu.add( _("File/Save/Reel As"), kSaveReel.hotkey(),
                      (Fl_Callback*)save_reel_cb, view() );
            menu.add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                      (Fl_Callback*)save_cb, view() );
            menu.add( _("File/Save/GL Snapshots As"), kSaveSnapshot.hotkey(),
                      (Fl_Callback*)save_snap_cb, view() );
            menu.add( _("File/Save/Session As"),
                      kSaveSession.hotkey(),
                      (Fl_Callback*)save_session_as_cb, view() );

            valid = ( /* dynamic_cast< slateImage* >( img ) == NULL && */
                      dynamic_cast< smpteImage* >( img ) == NULL &&
                      dynamic_cast< clonedImage* >( img ) == NULL );


            const stubImage* stub = dynamic_cast< const stubImage* >( img );
            if ( stub )
            {
                menu.add( _("Image/Clone"), 0,
                          (Fl_Callback*)clone_image_cb, this);
                menu.add( _("Image/Clone All Channels"), 0,
                          (Fl_Callback*)clone_all_cb,
                          this, FL_MENU_DIVIDER);
            }
            else
            {
                if ( valid )
                {
                    menu.add( _("Image/Clone"), 0,
                              (Fl_Callback*)clone_image_cb, this,
                              FL_MENU_DIVIDER);
                }
                menu.add( _("Image/Media Info"), 0,
                          (Fl_Callback*)media_info_cb, this,
                          FL_MENU_DIVIDER );
            }

            menu.add( _("Image/Set as Background"), 0,
                      (Fl_Callback*)set_as_background_cb,
                      (void*) view() );

            if ( valid )
            {
                valid = ( dynamic_cast< stubImage*  >( img ) == NULL );
            }
        }

        menu.add( _("Create/Black Gap"), 0,
                  (Fl_Callback*)black_cb, this);
        menu.add( _("Create/Color Bars/SMPTE NTSC"), 0,
                  (Fl_Callback*)ntsc_color_bars_cb, this);
        menu.add( _("Create/Color Bars/SMPTE NTSC HDTV"), 0,
                  (Fl_Callback*)ntsc_hdtv_color_bars_cb, this);
        menu.add( _("Create/Color Bars/PAL"), 0,
                  (Fl_Callback*)pal_color_bars_cb, this);
        menu.add( _("Create/Color Bars/PAL HDTV"), 0,
                  (Fl_Callback*)pal_hdtv_color_bars_cb, this);
        menu.add( _("Create/Gamma Chart/1.4"), 0,
                  (Fl_Callback*)gamma_chart_14_cb, this);
        menu.add( _("Create/Gamma Chart/1.8"), 0,
                  (Fl_Callback*)gamma_chart_18_cb, this);
        menu.add( _("Create/Gamma Chart/2.2"), 0,
                  (Fl_Callback*)gamma_chart_22_cb, this);
        menu.add( _("Create/Gamma Chart/2.4"), 0,
                  (Fl_Callback*)gamma_chart_24_cb, this);
        menu.add( _("Create/Gradient/Linear"), 0,
                  (Fl_Callback*)linear_gradient_cb, this);
        menu.add( _("Create/Gradient/Luminance"), 0,
                  (Fl_Callback*)luminance_gradient_cb, this);
        menu.add( _("Create/Checkered"), 0,
                  (Fl_Callback*)checkered_cb, this);

        // if (valid)
        //     menu.add( _("Create/Slate"), 0, (Fl_Callback*)slate_cb, this);

        menu.popup();
        return 1;
    }

    return ( ok != 0 );
}

void static_handle_dnd( mrv::ImageBrowser* b )
{
    b->handle_dnd();
    Fl::remove_idle( (Fl_Idle_Handler)static_handle_dnd, b );
}


/**
 * Handle a Drag and Drop operation on this widget (or image view)
 *
 */
void ImageBrowser::handle_dnd()
{
    DBGM1( "handle_dnd" );
    std::string filenames = _dnd_text;

    stringArray files;
#if defined(_WIN32) || defined(_WIN64)
    mrv::split_string( files, filenames, "\n" );
#else
    mrv::split_string( files, filenames, "\n" );
#endif

    std::sort( files.begin(), files.end() );

    mrv::Options opts;
    int64_t frameStart = AV_NOPTS_VALUE;
    int64_t frameEnd   = AV_NOPTS_VALUE;
    stringArray::iterator i = files.begin();
    stringArray::iterator e = files.end();
    std::string oldroot, oldview, oldext;

    DBGM1( "Parse string " << filenames );

    for ( ; i != e; ++i )
    {
#ifdef LINUX
        std::string file = hex_to_char_filename( *i );
#else
        std::string file = *i;
#endif

        if ( file.substr(0, 7) == "file://" )
            file = file.substr( 7, file.size() );

        if ( file.empty() ) continue;

        if ( mrv::is_directory( file.c_str() ) )
        {
            parse_directory( file, opts );
            continue;
        }
        else
        {
            retname = file;

            std::string root, frame, view, ext;
            bool ok = mrv::split_sequence( root, frame, view, ext, file );

            DBGM1( "Parse file " << file );
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

    DBGM1( "Load opts.files" );
    load( opts.files );

    if ( opts.files.size() > 1 )
    {
        set_edl();
    }

    last_image();

    uiMain->uiView->take_focus();
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

    if ( dragging ) redraw();

    if (y < 0) lastY = 0;
    else       lastY = y;
    redraw();
    return 1;
}

void ImageBrowser::exchange( int oldsel, int sel )
{
    if ( oldsel == sel )
    {
        return;
    }
    if ( sel < 0 || oldsel < 0  )
    {
        LOG_ERROR( _("Negative indices to exchange") );
        redraw();
        return;
    }

    char buf[1024];
    sprintf( buf, _("ExchangeImage %d %d"), sel, oldsel );
    view()->send_network( buf );

    CMedia::Playback play = view()->playback();

    if ( play != CMedia::kStopped )
        view()->stop();

    root()->swap_children( sel, oldsel );

    mrv::Reel r = current_reel();
    mrv::media m = r->images[sel];
    view()->foreground( m );

    match_tree_order();

    mrv::Timeline* t = timeline();
    if ( !t ) return;

    CMedia* img = m->image();
    int64_t f = img->frame();

    //
    // Adjust timeline position
    //
    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

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
        DBGM3( "set frame to " << f );
        frame( f );
    }

    if ( play ) view()->play(play);

    redraw();
}

Fl_Tree_Item* ImageBrowser::media_to_item( const mrv::media fg )
{
    if ( !fg ) {
        LOG_ERROR( "Empty media passed to media_to_item" );
        return NULL;
    }

    Fl_Tree_Item* i = NULL;
    for ( i = first(); i; i = next(i) )
    {
        if ( ! i->widget() ) {
            continue;
        }

        mrv::Element* elem = (mrv::Element*) i->widget();
        mrv::media m = elem->media();
        if ( m && m == fg ) {
            return i;
        }
    }

    return NULL;
}

void ImageBrowser::match_tree_order()
{
    mrv::Reel r = current_reel();
    r->images.clear();

    value( -1 );

    mrv::media fg = view()->foreground();

    int idx = -1;
    Fl_Tree_Item* i;
    for ( i = first(); i; i = next(i) )
    {
        mrv::Element* elem = (mrv::Element*) i->widget();
        if ( !elem ) continue;
        mrv::media m = elem->media();
        r->images.push_back( m );
        if ( m == fg && m ) {
            idx = int( r->images.size() - 1 );
        }
    }

    change_image(idx);

#ifdef DEBUG_IMAGES_ORDER

    std::cerr << "\n\n";
    for ( unsigned j = 0; j < r->images.size(); ++j )
    {
        const mrv::media& m = r->images[j];
        std::cerr << m->name() << " "
                  << m->position() << "-"
                  << m->position()+m->duration()-1;
        if ( j == value() ) std::cerr << " <----";
        std::cerr << "\n";
    }
    std::cerr << "\n\n";

#endif

}

int ImageBrowser::mouseRelease( int x, int y )
{
    int ok = 0;
    if (!dragging )
    {
        redraw();
    }

    dragging = NULL;

    int old = value();

    match_tree_order();

    int sel = value();

    if ( sel == old ) {
        return ok;
    }


    char buf[1024];
    sprintf( buf, _("ExchangeImage %d %d"), sel, old );
    view()->send_network( buf );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    return 1;
}

/**
 * Event handler for the image browser window
 *
 * @param event Fl::event enumeration
 *
 * @return 1 if handled, 0 if not
 */
int ImageBrowser::handle( int event )
{
    if ( event == FL_KEYBOARD )
    {
        if ( kFitScreen.match( Fl::event_key() ) )
        {
            Fl_Tree::display( Fl_Tree::first_selected_item() );
            return 1;
        }
        else if ( Fl::event_key() == FL_Escape && dragging != NULL )
        {
            dragging = NULL;
            redraw();
            return 1;
        }
        else if ( Fl::event_key() == FL_Delete )
        {
            remove_current();
            redraw();
            return 1;
        }

        int ok = view()->handle( event );
        if ( ok ) return ok;
    }

    int ret = Fl_Tree::handle( event );
    switch( event )
    {
    case FL_DND_ENTER:
    case FL_DND_LEAVE:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
        return 1;
    case FL_PASTE:
        {
            std::string text;
            if ( Fl::event_text() ) text = Fl::event_text();
            dnd_text( text );
            LOG_INFO( "DND: " << text );
            Fl::add_idle( (Fl_Idle_Handler)static_handle_dnd, this );
            return 1;
        }
    case FL_UNFOCUS:
    case FL_FOCUS:
        return 1;
    case FL_PUSH:
        return mousePush( Fl::event_x(), Fl::event_y() );
    case FL_DRAG:
        return mouseDrag( Fl::event_x(), Fl::event_y() );
    case FL_RELEASE:
        return mouseRelease( Fl::event_x(), Fl::event_y() );
    }
    return ret;
}


/**
 * Switch to a new frame on one image if in EDL mode or
 * in all images of reel if not.
 *
 * @param tframe new frame in timeline
 */
void ImageBrowser::seek( const int64_t tframe )
{
    int64_t f = tframe;  // needed as we may change it and tframe is const

    CMedia::Playback play = view()->playback();
    if ( play ) view()->stop();

    TRACE( "BROWSER seek to frame " << f
            << " view frame " << view()->frame()
            << " view->playback=" << play );


    char buf[64];
    sprintf( buf, "seek %" PRId64, f );
    view()->send_network(buf);


    mrv::media fg = view()->foreground();
    mrv::media bg = view()->background();

    view()->frame( tframe );
    if ( view()->A_image() && view()->B_image() ) return;


    mrv::Timeline* t = timeline();

    mrv::Reel reel = current_reel();
    mrv::Reel bgreel = reel_at( view()->bg_reel() );
    if ( reel && reel->edl )
    {
        TRACE( "BROWSER seek to frame " << f );

        // Check if we need to change to a new sequence based on frame
        mrv::media m = reel->media_at( tframe );
        if (! m ) return;

        if ( fg && fg != m )
        {
            CMedia* img = fg->image();

            TRACE( "BROWSER old image " << img->name() << " gframe= " << f
                    << " lframe= " << img->frame() << " in= " << img->in_frame()
                    << " out= " << img->out_frame() << " stopped? "
                    << img->stopped() );
            img->stop();
            img->close_audio();
        }
        if ( bg && bg != m )
        {
            CMedia* img = bg->image();
            img->close_audio();
        }


        CMedia* img = m->image();
        if ( ! img ) return;

        TRACE( "BROWSER new image " << img->name() << " gframe= " << f
                << " lframe= " << img->frame() << " in= " << img->in_frame()
                << " out= " << img->out_frame() << " stopped? "
                << img->stopped() );

        if ( f < t->display_minimum() )
        {
            f = int64_t(t->display_maximum() - t->display_minimum()) - f + 1;
        }
        else if ( f > t->display_maximum() )
        {
            f = int64_t(t->display_minimum() - t->display_maximum()) + f - 1;
        }

        mrv::media fg = view()->foreground();
        int64_t lf = reel->global_to_local( f );

        if ( m != fg && fg )
        {
            size_t i = reel->index( f );
            img = reel->image_at( f );
            if ( !img ) return;


            TRACE( "BROWSER pre seek " << img->name() << " gframe= " << f
                    << " lframe= " << img->frame() << " in= " << img->in_frame()
                    << " out= " << img->out_frame() << " stopped? "
                    << img->stopped() );

            img->seek( lf );

            if ( i < reel->images.size() )
                change_image((int)i);

            TRACE( "BROWSER post seek " << img->name() << " gframe= " << f
                    << " lframe= " << lf << " in= " << img->in_frame()
                    << " out= " << img->out_frame() << " stopped? "
                    << img->stopped() );

            CMedia* old = fg->image();
            if (old->has_video()) old->clear_cache();
        }
        else
        {
            TRACE2( "BROWSER same image " << img->name() << " gframe= " << f
                    << " lframe= " << lf << " in= " << img->in_frame()
                    << " out= " << img->out_frame() << " stopped? "
                    << img->stopped() );

            img->seek( lf );
        }

        mrv::Reel reel = reel_at( view()->bg_reel() );
        if ( reel )
        {
            mrv::media bg = view()->background();
            if ( bg )
            {
                bg = reel->media_at( tframe );

                if ( bg )
                {
                    int64_t lf = reel->global_to_local( f );

                    img = bg->image();
                    img->seek( lf );
                }
            }
        }

    }
    else
    {
        mrv::media fg = view()->foreground();
        if (!fg) return;

        CMedia* img = fg->image();
        img->seek( f );

        mrv::media bg = view()->background();
        if ( bg )
        {
            img = bg->image();
            img->seek( f );
        }
    }

    // Update current frame and timeline
    mrv::Timeline* timeline = uiMain->uiTimeline;
    if ( timeline->visible() )
    {
        timeline->value( double(f) );
        uiMain->uiFrame->value(f );  // so it is displayed properly
    }

    if ( uiMain->uiEDLWindow )
    {
        mrv::Timeline* t = uiMain->uiEDLWindow->uiTimeline;
        if (t && t->visible() )
        {
            t->value( double(f) );
        }
    }

    if ( play ) view()->play( play );


    redraw();
}


/**
 * Switch to a new frame, later changing timeline value and uiFrame.
 * This function does not check frame is in range in timeline.
 *
 * @param f new frame in timeline units
 */
void ImageBrowser::frame( const int64_t f )
{
    if ( ! uiMain->uiView ) return;

    uiMain->uiView->frame( f );

}

void ImageBrowser::clear_edl()
{
    CMedia::Playback play = view()->playback();
    if ( play ) view()->stop();

    mrv::Reel reel = current_reel();

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
    if ( !img ) return;

    int64_t f = img->frame();


    frame( f );

    int64_t first, last;
    adjust_timeline( first, last );
    set_timeline( first, last );

    char buf[64];
    sprintf( buf, "EDL 0" );
    view()->send_network( buf );

    if ( play ) view()->play(play);
}

void ImageBrowser::set_edl()
{
    CMedia::Playback play = view()->playback();
    if ( play ) view()->stop();

    mrv::Reel reel = current_reel();

    reel->edl = true;
    mrv::Timeline* t = timeline();
    if ( t )
    {
        t->edl( true );
        t->redraw();
    }

    uiMain->uiReelWindow->uiEDLButton->value(1);

    match_tree_order();
    int64_t first, last;
    adjust_timeline( first, last );

    set_timeline( first, last );

    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    if ( !img ) return;

    int64_t f = reel->local_to_global( img->frame(), img );
    frame( f );

    char buf[64];
    sprintf( buf, "EDL 1" );
    view()->send_network( buf );

    if ( play ) view()->play(play);

}

void ImageBrowser::toggle_edl()
{
    mrv::Reel reel = current_reel();

    if ( reel->edl ) clear_edl();
    else set_edl();
}






void ImageBrowser::adjust_timeline(int64_t& first, int64_t& last)
{
    int64_t f = view()->frame();

    mrv::Reel reel = current_reel();
    if ( reel->images.empty() ) {
        first = 1;
        last = 50;
        frame( 1 );
        return;
    }

    if ( reel->edl )
    {
        MediaList::iterator i = reel->images.begin();
        MediaList::iterator e = reel->images.end();
        // bool update = true;
        // if ( t->edl() )
        // {
        //     if ( t->undo_minimum() != AV_NOPTS_VALUE &&
        //          t->undo_maximum() != AV_NOPTS_VALUE )
        //     {
        //         update = false;
        //     }
        // }


        // if (! update ) return;

        if ( i != e )
        {
            MediaList::iterator j;
            (*i)->position( 1 );

            for ( j = i, ++i; i != e; j = i, ++i )
            {
                int64_t frame = (*j)->position() + (*j)->duration();
                DBGM1( (*i)->image()->name() << " moved to frame " << frame );
                if ( (*i)->position() > frame ) continue;
                (*i)->position( frame );
            }
        }

        mrv::EDLGroup* eg = edl_group();
        if ( eg )
        {
            eg->redraw();
        }

        mrv::media m = reel->images.front();
        if (! m ) return;

        CMedia* img = m->image();

        f = m->position() + img->frame() - img->first_frame();

        first = 1;

        m = reel->images.back();
        last  = m->position() + m->duration() - 1;
    }
    else
    {
        mrv::media m = current_image();
        if ( ! m ) return;
        CMedia* img = m->image();

        first = img->first_frame();
        if ( Preferences::switching_images ) f = img->frame();
        last  = img->last_frame();

        if (f > last ) f = last;
        if (f < first ) f = first;

        frame( f );
    }




}


bool ImageBrowser::add_to_tree( mrv::media m )
{
    std::string path = media_to_pathname( m );
    Fl_Tree_Item* item = Fl_Tree::insert( root(), path.c_str(),
                                          root()->children() );
    if ( !item ) return false;

    Element* nw = new_item( m );
    if ( !nw ) return false;
    item->widget( nw );
    return true;
}

void ImageBrowser::draw()
{
    Fl_Tree_Item* i = NULL;
    for ( i = first(); i; i = next(i) )
    {
        if ( ! i->widget() ) continue;


        if ( view()->playback() == CMedia::kStopped )
        {
            mrv::Element* elem = (mrv::Element*) i->widget();
            if ( elem->visible_r() ) elem->make_thumbnail();
        }
    }

    Fl_Tree::selection_color( Fl_Color(0xffff0000) );

    // Let tree draw itself
    Fl_Tree::draw();

    // Dragging item that has a widget()?
    //    Assume widget() is a group, draw it where the mouse is..
    //
    if ( selectmode() == FL_TREE_SELECT_SINGLE_DRAGGABLE &&
         dragging && dragging->widget() )
    {
        // Draw dragging line
        if (Fl::pushed() == this)
        { // item clicked is the one we're drawing?
            Fl_Tree_Item *item = root()->find_clicked(prefs(), 1); // item we're on, vertically
            if (item &&                   // we're over a valid item?
                item != get_item_focus()) { // item doesn't have keyboard focus?
                // Are we dropping above or below the target item?
                const int h = Fl::event_y() - item->y(); // mouse relative to item's top/left
                const int mid = item->h() / 2;  // middle of item relative to item's top/left
                const bool is_above = h < mid; // is mouse above middle of item?
                fl_color(FL_RED);
                int tgt = item->y() + (is_above ? 0 : item->h());
                fl_line_style( FL_SOLID, 4 );
                fl_line(item->x(), tgt, item->x() + item->w(), tgt);
            }
        }
        mrv::Element* elem = (mrv::Element*) dragging->widget();
        fl_push_clip( _tix, _tiy, _tiw, _tih );
        elem->DrawAt( Fl::event_x(), Fl::event_y() );
        fl_pop_clip();

    }
}


} // namespace mrv
