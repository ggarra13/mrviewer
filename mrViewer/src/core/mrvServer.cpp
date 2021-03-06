/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
//#define BOOST_ASIO_ENABLE_BUFFER_DEBUGGING
//#define DEBUG_COMMANDS

#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>  // for PRId64

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <fstream>
#include <set>

#include <boost/locale.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio.hpp>

#include <FL/fl_utf8.h>

#include <ImfStandardAttributes.h>
#include <ImfVecAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfStringAttribute.h>

#include "core/mrvPlayback.h"
#include "mrvClient.h"
#include "mrvServer.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvEDLGroup.h"
#include "mrvEDLWindowUI.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvIO.h"
#include "gui/mrvReel.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvVectorscope.h"
#include "gui/mrvHistogram.h"
#include "gui/mrvTimeline.h"
#include "mrvColorAreaUI.h"
#include "mrvReelUI.h"
#include "mrvPreferencesUI.h"
#include "mrViewer.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

namespace {
const char* const kModule = "parser";
}


//----------------------------------------------------------------------

namespace mrv {

typedef CMedia::Mutex Mutex;

Parser::Parser( boost::asio::io_service& io_service, ViewerUI* v ) :
    connected( false ),
    socket_( io_service ),
    ui( v ),
    deadline_(io_service),
    non_empty_output_queue_(io_service)
{
}

Parser::~Parser()
{
}

void Parser::write( const std::string& s, const std::string& id )
{
    if ( !connected || !ui || !ui->uiView ) {
        return;
    }

    mrv::ImageView* v = view();

    Mutex& m = v->_clients_mtx;
    SCOPED_LOCK( m );

    ParserList::const_iterator i = v->_clients.begin();
    ParserList::const_iterator e = v->_clients.end();

    for ( ; i != e; ++i )
    {
        try
        {
            if ( !(*i)->socket_.is_open() ) continue;
            std::string p = boost::lexical_cast<std::string>( (*i)->socket_.remote_endpoint() );

            if ( p == id )
            {
                continue;
            }
            // LOG_INFO( "Resending " << s << " to " << p );
            (*i)->deliver( s );
        }
        catch( const std::exception& e )
        {
            LOG_CONN( "Parser::write " << e.what() );
        }
        catch( ... )
        {
            LOG_CONN( "Parser::write unhandled exception" );
        }
    }
}

mrv::ImageView* Parser::view() const
{
    if ( !ui ) return NULL;
    return ui->uiView;
}

mrv::ImageBrowser* Parser::browser() const
{
    if ( !ui || !ui->uiReelWindow ) return NULL;
    return ui->uiReelWindow->uiBrowser;
}

mrv::EDLGroup* Parser::edl_group() const
{
    if ( !ui || !ui->uiEDLWindow ) return NULL;
    return ui->uiEDLWindow->uiEDLGroup;
}

bool Parser::parse( const std::string& s )
{
    if ( !connected || !ui || !ui->uiView ) return false;

    mrv::ImageView* v = view();
    if ( !v ) return false;

    typedef CMedia::Mutex Mutex;

    Mutex& mtx = v->commands_mutex;
    SCOPED_LOCK( mtx );

    std::istringstream is( s );

    // Set locale globally to user locale
    // const char* env = getenv("LC_ALL");
    // if ( !env )
    //     std::locale::global( std::locale("") );
    // else
    //     std::locale::global( std::locale(env) );
    // is.imbue(std::locale());

    setlocale( LC_NUMERIC, "C" );


    mrv::Reel r;

    std::string cmd;
    is >> cmd;

    bool ok = false;

    if (! v ) return false;

    Mutex& cmtx = v->_clients_mtx;
    SCOPED_LOCK( cmtx );

    v->network_active( false );



    if ( Preferences::debug )
        LOG_INFO( "received: " << s );

    if ( cmd == N_("GLPathShape") )
    {
        Point xy;
        std::string points;
        GLPathShape* shape = new GLPathShape;
        std::getline( is, points );
        is.str( points );
        is.clear();
        is >> shape->r >> shape->g >> shape->b >> shape->a >> shape->pen_size
           >> shape->frame;
        while ( is >> xy.x >> xy.y )
        {
            shape->pts.push_back( xy );
        }
        v->add_shape( mrv::shape_type_ptr(shape) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("GLArrowShape") )
    {
        Point xy;
        std::string points;
        GLArrowShape* shape = new GLArrowShape;
        std::getline( is, points );
        is.str( points );
        is.clear();
        is >> shape->r >> shape->g >> shape->b >> shape->a >> shape->pen_size
           >> shape->frame;
        while ( is >> xy.x >> xy.y )
        {
            shape->pts.push_back( xy );
        }
        v->add_shape( mrv::shape_type_ptr(shape) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("GLCircleShape") )
    {
        Point xy;
        std::string points;
        GLCircleShape* shape = new GLCircleShape;
        std::getline( is, points );
        is.str( points );
        is.clear();
        is >> shape->r >> shape->g >> shape->b >> shape->a >> shape->pen_size
           >> shape->frame;
        is >> shape->center.x >> shape->center.y;
        is >> shape->radius;
        v->add_shape( mrv::shape_type_ptr(shape) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("GLErasePathShape") )
    {
        Point xy;
        std::string points;
        GLErasePathShape* shape = new GLErasePathShape;
        is.clear();
        std::getline( is, points );
        is.str( points );
        is.clear();
        is >> shape->pen_size >> shape->frame;
        while ( is >> xy.x >> xy.y )
        {
            shape->pts.push_back( xy );
        }
        v->add_shape( mrv::shape_type_ptr(shape) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("GLTextShape") )
    {
        Point xy;
        std::string font, text;
        static string old_text;
        static string old_font;
        static unsigned old_size = 0;
        static ImagePixel oldcolor;
        static int64_t old_frame = AV_NOPTS_VALUE;
        unsigned font_size;
        std::getline( is, font, '"' ); // skip first quote
        std::getline( is, font, '"' );

        std::getline( is, text, '^' ); // skip first quote
        std::getline( is, text, '^' );

        ImagePixel color;
        int64_t frame;

        is >> font_size >> color.r >> color.g >> color.b >> color.a
           >> frame;

        bool same = true;
        if ( font != old_font || text != old_text || font_size != old_size ||
             color.r != oldcolor.r || color.g != oldcolor.g ||
             color.b != oldcolor.b || color.a != oldcolor.a ||
             frame != old_frame )
        {
            same = false;
        }


        GLTextShape* shape;
        mrv::ImageView* view = ui->uiView;
        if ( same && !view->shapes().empty() )
        {
            shape = dynamic_cast< GLTextShape* >( view->shapes().back().get() );
            if ( shape == NULL ) {
                LOG_ERROR( "Not a GLTextShape as last shape" );
                v->network_active( true );
                return false;
            }
        }
        else
        {
            shape = new GLTextShape;
        }

        shape->text( text );

        unsigned i;
        unsigned num = Fl::set_fonts( "-*" );
        for ( i = 0; i < num; ++i )
        {
            int t;
            if ( font == Fl::get_font_name( (Fl_Font) i, &t ) ) break;
        }
        if ( i >= num ) i = 0;


        shape->font( (Fl_Font) i );
        is >> xy.x >> xy.y;
        shape->size( font_size );
        shape->r = color.r;
        shape->g = color.g;
        shape->b = color.b;
        shape->a = color.a;
        shape->frame = frame;
        shape->pts.clear();
        shape->pts.push_back( xy );

        if ( !same )
        {
            v->add_shape( mrv::shape_type_ptr(shape) );
            old_text = text;
            old_font = font;
            old_size = font_size;
            oldcolor.r = shape->r;
            oldcolor.g = shape->g;
            oldcolor.b = shape->b;
            oldcolor.a = shape->a;
            old_frame = frame;
        }

        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("GhostPrevious") )
    {
        short x;
        is >> x;
        v->ghost_previous( x );
        ok = true;
    }
    else if ( cmd == N_("GhostNext") )
    {
        short x;
        is >> x;
        v->ghost_next( x );
        ok = true;
    }
    else if ( cmd == N_("FPS") )
    {
        double fps;
        is >> fps;

        v->fps( fps );

        ok = true;
    }
    else if ( cmd == N_("EDL") )
    {
        int b;
        is >> b;
        if ( b )
            browser()->set_edl();
        else
            browser()->clear_edl();
        ok = true;
    }
    else if ( cmd == N_("Looping") )
    {
        int i;
        is >> i;

        v->looping( (CMedia::Looping)i );
        ok = true;
    }
    else if ( cmd == N_("Selection") )
    {
        double x, y, w, h;
        is >> x >> y >> w >> h;
        v->selection( mrv::Rectd( x, y, w, h ) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("UndoDraw") )
    {
        v->undo_draw();
        ok = true;
    }
    else if ( cmd == N_("RedoDraw") )
    {
        v->redo_draw();
        ok = true;
    }
    else if ( cmd == N_("Zoom") )
    {
        float z;
        is >> z;

        ImageView::Command c;
        c.type = ImageView::kZoomChange;
        c.data = new Imf::FloatAttribute( z );
        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("Rotation") )
    {
        double x, y;
        is >> x >> y;
        v->rot_x( x );
        v->rot_y( y );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Spin") )
    {
        double x, y;
        is >> x >> y;
        v->spin_x( x );
        v->spin_y( y );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Offset") )
    {
        double x, y;
        is >> x >> y;
        v->offset_x( x );
        v->offset_y( y );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("MovePicture") )
    {
        double x, y;
        is >> x >> y;
        mrv::media fg = v->foreground();
        if ( !fg ) {
            v->network_active( true );
            return false;
        }
        CMedia* img = fg->image();
        img->x( x );
        img->y( y );
        ok = true;
    }
    else if ( cmd == N_("ScalePicture") )
    {
        double x, y;
        is >> x >> y;
        mrv::media fg = v->foreground();
        if ( !fg ) {
            v->network_active( true );
            return false;
        }
        CMedia* img = fg->image();
        img->scale_x( x );
        img->scale_y( y );
        ok = true;
    }
    else if ( cmd == N_("UpdateLayers") )
    {
        v->update_layers();
        ok = true;
    }
    else if ( cmd == N_("Channel") )
    {
        unsigned short ch;
        std::string name;
        is >> ch >> name;

        ImageView::Command c;
        c.type = ImageView::kChangeChannel;
        c.data = new Imf::IntAttribute( ch );

        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("FieldDisplay") )
    {
        int field;
        is >> field;
        v->field( (mrv::ImageView::FieldDisplay) field );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Normalize") )
    {
        int b;
        is >> b;
        v->normalize( ( b != 0 ) );
        ui->uiNormalize->value( (b != 0 ) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("WipeVertical") )
    {
        float b;
        is >> b;
        v->wipe_direction( ImageView::kWipeVertical );
        v->wipe_amount( b );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("WipeHorizontal") )
    {
        float b;
        is >> b;
        v->wipe_direction( ImageView::kWipeHorizontal );
        v->wipe_amount( b );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("NoWipe") )
    {
        v->wipe_direction( ImageView::kNoWipe );
        v->wipe_amount( 0.0f );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("OCIO") )
    {
        bool t;
        is >> t;

        mrv::Preferences::use_ocio = t;
        v->main()->uiPrefs->uiPrefsUseOcio->value( t );

        const char* env = fl_getenv( "OCIO" );
        if ( env )
        {
            ImageView::Command c;
            c.type = ImageView::kLUT_CHANGE;
            v->commands.push_back( c );
        }
        ok = true;
    }
    else if ( cmd == N_("OCIOConfig") )
    {
        std::string s;
        is.clear();
        std::getline( is, s, '"' ); // skip first quote
        is.clear();
        std::getline( is, s, '"' );

        char buf[1024];
        sprintf( buf, "OCIO=%s", s.c_str() );
        putenv( av_strdup(buf) );

        ImageView::Command c;
        c.type = ImageView::kLUT_CHANGE;
        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("OCIOView") )
    {
        std::string d, s;
        is.clear();
        std::getline( is, d, '"' ); // skip first quote
        is.clear();
        std::getline( is, d, '"' );
        is.clear();
        std::getline( is, s, '"' ); // skip first quote
        is.clear();
        std::getline( is, s, '"' );

        ImageView::Command c;
        c.type = ImageView::kOCIOViewChange;
        Imf::StringVector vec;
        vec.push_back( d );
        vec.push_back( s );
        c.data = new Imf::StringVectorAttribute( vec );
        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("ICS") )
    {
        std::string s;
        is.clear();
        std::getline( is, s, '"' ); // skip first quote
        is.clear();
        std::getline( is, s, '"' );

        ImageView::Command c;
        c.type = ImageView::kICS;
        c.data = new Imf::StringAttribute( s );

        v->commands.push_back( c );


        ok = true;
    }
    else if ( cmd == N_("Gain") )
    {
        float f;
        is >> f;
        v->gain( f );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Gamma") )
    {
        float f;
        is >> f;
        v->gamma( f );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Mask") )
    {
        float b;
        is >> b;
        v->masking( b );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("SafeAreas") )
    {
        int b;
        is >> b;
        v->safe_areas( (b != 0) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("DisplayWindow") )
    {
        int b;
        is >> b;
        v->display_window( (b != 0) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("Volume") )
    {
        float b;
        is >> b;
        v->volume( b );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("DataWindow") )
    {
        int b;
        is >> b;
        v->data_window( (b != 0) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("UseLUT") )
    {
        int b;
        is >> b;
        ui->uiLUT->value( (b != 0) );
        v->use_lut( (b != 0) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("AudioVolume") )
    {
        float t;
        is >> t;

        v->volume( t );
        ok = true;
    }
    else if ( cmd == N_("ShowBG") )
    {
        int b;
        is >> b;
        v->show_background( ( b != 0 ) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("ShowPixelRatio") )
    {
        int b;
        is >> b;
        v->show_pixel_ratio( (b != 0) );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("TimelineMax") )
    {
        double x;
        is >> x;

        ImageView::Command c;
        c.type = ImageView::kTimelineMax;
        c.frame = (int64_t) x;
        ok = true;
    }
    else if ( cmd == N_("TimelineMaxDisplay") )
    {
        double x;
        is >> x;

        ImageView::Command c;
        c.type = ImageView::kTimelineMaxDisplay;
        c.frame = (int64_t) x;
        ok = true;
    }
    else if ( cmd == N_("TimelineMin") )
    {
        double x;
        is >> x;

        ImageView::Command c;
        c.type = ImageView::kTimelineMin;
        c.frame = (int64_t) x;

        ok = true;
    }
    else if ( cmd == N_("TimelineMinDisplay") )
    {
        double x;
        is >> x;
        ImageView::Command c;
        c.type = ImageView::kTimelineMinDisplay;
        c.frame = (int64_t) x;
        ok = true;
    }
    else if ( cmd == N_("VRCubic") )
    {
        bool t;
        is >> t;
        if ( ui->uiStereo )
            ui->uiStereo->uiVR360Cube->value( t );
        if (t) v->vr( ImageView::kVRCubeMap );
        else v->vr( ImageView::kNoVR );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("VRSpherical") )
    {
        bool t;
        is >> t;
        if ( ui->uiStereo )
            ui->uiStereo->uiVR360Sphere->value( t );
        if (t) v->vr( ImageView::kVRSphericalMap );
        else v->vr( ImageView::kNoVR );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("VRangle") )
    {
        float t;
        is >> t;
        v->vr_angle( t );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("FullScreen") )
    {
        int on;
        is >> on;
        ImageView::Command c;
        c.type = ImageView::kFULLSCREEN;
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("PresentationMode" ) )
    {
        int on;
        is >> on;
        ImageView::Command c;
        c.type = ImageView::kPRESENTATION;
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("ShiftAudio") )
    {
        int reel;
        is >> reel;

        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );

        int64_t offset;
        is >> offset;

        edl_group()->shift_audio( reel, imgname, offset );

        ok = true;
    }
    else if ( cmd == N_("ShiftMediaStart") )
    {
        int reel;
        is >> reel;

        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );

        int64_t diff;
        is >> diff;

        edl_group()->shift_media_start( reel, imgname, diff );

        ok = true;
    }
    else if ( cmd == N_("ShiftMediaEnd") )
    {
        int reel;
        is >> reel;

        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );

        int64_t diff;
        is >> diff;

        edl_group()->shift_media_end( reel, imgname, diff );

        ok = true;
    }
    else if ( cmd == N_("CurrentReel") )
    {
        std::string name;
        is.clear();
        std::getline( is, name, '"' ); // skip first quote
        is.clear();
        std::getline( is, name, '"' ); // read up to second quote
        is.clear();

        ImageView::Command c;
        c.type = ImageView::kCreateReel;
        c.data = new Imf::StringAttribute( name );

        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("ReplaceImage") )
    {
        int idx;
        is >> idx;

        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );

        stringArray files;
        files.push_back( imgname );
        browser()->load( files );

        mrv::media fg = v->foreground();
        browser()->replace( idx, fg );
        v->redraw();
        ok = true;
    }
    else if ( cmd == N_("RemoveImage") )
    {
        int idx;
        is >> idx;

        ImageView::Command c;
        c.type = ImageView::kRemoveImage;
        c.data = new Imf::IntAttribute( idx );

        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("AudioStream") )
    {
        unsigned idx;
        is >> idx;

        v->audio_stream( idx );

        ok = true;
    }
    else if ( cmd == N_("CloneImage") )
    {
        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );

        if ( r )
        {
            size_t j;
            size_t e = r->images.size();

            for ( j = 0; j != e; ++j )
            {
                mrv::media fg = r->images[j];
                CMedia* img = fg->image();
                std::string file = img->directory() + '/' + img->name();
                if ( file == imgname )
                    break;
            }

            if ( j != e )
            {
                browser()->value( int(j) );
                browser()->clone_current();
                browser()->redraw();

                ok = true;
            }
        }
    }
    else if ( cmd == N_("InsertImage") )
    {
        int idx;
        is >> idx;

        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );
        is.clear();

        ImageView::Command c;
        c.type = ImageView::kInsertImage;
        c.data = new Imf::IntAttribute( idx );
        c.linfo = new LoadInfo( imgname );
        v->commands.push_back( c );
        ok = true;
    }
    else
        if ( cmd == N_("ChangeImage") )
    {
        int idx;
        is >> idx;

        ImageView::Command c;
        c.type = ImageView::kChangeImage;
        c.data = new Imf::IntAttribute( idx );
        c.linfo = NULL;

        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("ClearCache") )
    {

        ImageView::Command c;
        c.type = ImageView::kCacheClear;


        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("CurrentImage") )
    {
        int idx = 0;
        std::string imgname;
        is >> idx;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );
        is.clear();

        int64_t first, last;
        is >> first;
        is >> last;

        LOG_WARNING( "Change to image #" << idx << " "  << imgname );
        ImageView::Command c;
        c.type = ImageView::kChangeImage;
        c.data = new Imf::IntAttribute( idx );
        c.linfo = new LoadInfo(imgname, first, last);

        v->commands.push_back( c );

        ok = true;

    }
    else if ( cmd == N_("ExchangeImage") )
    {
        int oldsel, sel;
        is >> oldsel;
        is >> sel;

        ImageView::Command c;
        c.type = ImageView::kExchangeImage;
        Imath::V2i vec( oldsel, sel );
        c.data = new Imf::V2iAttribute( vec );

        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("FGReel") )
    {
        int idx;
        is >> idx;

        ImageView::Command c;
        c.type = ImageView::kFGReel;
        c.data = new Imf::IntAttribute( idx );

        v->commands.push_back( c );

        r = browser()->reel_at( idx );

        ok = true;
    }
    else if ( cmd == N_("BGReel") )
    {
        int idx;
        is >> idx;

        ImageView::Command c;
        c.type = ImageView::kBGReel;
        c.data = new Imf::IntAttribute( idx );

        v->commands.push_back( c );

        r = browser()->reel_at( idx );

        ok = true;
    }
    else if ( cmd == N_("CurrentBGImage") )
    {
        std::string imgname;
        is.clear();
        std::getline( is, imgname, '"' ); // skip first quote
        is.clear();
        std::getline( is, imgname, '"' );
        is.clear();


        int64_t first, last;
        is >> first;
        is >> last;

        r = browser()->current_reel();

        if ( r )
        {
            mrv::MediaList::iterator j = r->images.begin();
            mrv::MediaList::iterator e = r->images.end();
            int idx = 0;
            ImageView::Command c;
            c.type = ImageView::kBGImage;


            for ( ; j != e; ++j, ++idx )
            {
                if ( !(*j) ) continue;
                CMedia* img = (*j)->image();
                std::string file = img->directory() + '/' + img->name();
                if ( file == imgname )
                {
                    c.data = new Imf::IntAttribute( idx );

                    img->first_frame( first );
                    img->last_frame( last );
                    ok = true;
                    break;
                }
            }

            if (!ok )
            {
                c.data = new Imf::IntAttribute( -1 );
            }

            v->commands.push_back( c );
            ok = true;
        }
        v->redraw();
    }
    else if ( cmd == N_("TextureFiltering") )
    {
        int f;
        is >> f;
        v->texture_filtering( (ImageView::TextureFiltering) f );
        ok = true;
    }
    else if ( cmd == N_("sync_image") )
    {
        std::string cmd;
        size_t num = browser()->number_of_reels();
        for (size_t i = 0; i < num; ++i )
        {
            r = browser()->reel_at( unsigned(i) );
            if (!r) continue;

            cmd = N_("CurrentReel \"");
            cmd += r->name;
            cmd += "\"";
            deliver( cmd );

            mrv::MediaList::iterator j = r->images.begin();
            mrv::MediaList::iterator e = r->images.end();
            for ( ; j != e; ++j )
            {
                if ( !(*j) ) continue;

                CMedia* img = (*j)->image();

                cmd = N_("CurrentImage \"");
                cmd += img->directory();
                cmd += "/";
                cmd += img->name();
                cmd += "\" ";

                char buf[1024];
                int64_t start = img->first_frame();
                int64_t end   = img->last_frame();

                sprintf( buf, N_("%" PRId64 " %" PRId64 ),
                         start, end );
                cmd += buf;

                deliver( cmd );

                //
                // Handle color management (OCIO)
                //
                const std::string& s = img->ocio_input_color_space();
                if ( ! s.empty() )
                {
                    sprintf( buf, N_("ICS \"%s\""), s.c_str() );
                    deliver( buf );
                }

                //
                // Handle shape drawings
                //
                const mrv::GLShapeList& shapes = img->shapes();
                if ( shapes.empty() ) continue;


                cmd = N_("CurrentImage \"");
                cmd += img->fileroot();

                sprintf( buf, "\" %" PRId64 " %" PRId64, img->first_frame(),
                         img->last_frame() );
                cmd += buf;
                deliver( cmd );

                mrv::GLShapeList::const_iterator k = shapes.begin();
                mrv::GLShapeList::const_iterator e = shapes.end();
                for ( ; k != e; ++k )
                {
                    std::string s = (*k)->send();
                    deliver( s );
                }

            }
        }

        char buf[1024];
        int64_t frame = v->frame();
        sprintf( buf, N_("seek %" PRId64 ), frame );
        deliver( buf );

        if ( num == 0 ) {
            v->network_active( true );
            return true;
        }


        r = browser()->current_reel();
        if (r)
        {
            cmd = N_("CurrentReel \"");
            cmd += r->name;
            cmd += "\"";
            deliver( cmd );

            if ( r->edl )
            {
                cmd = N_("EDL 1");
                deliver( cmd );
            }
        }

        if ( browser()->value() >= 0 )
        {
            sprintf( buf, N_("ChangeImage %d"), browser()->value() );
            deliver( buf );

            int64_t frame = v->frame();
            sprintf( buf, N_("seek %" PRId64 ), frame );
            deliver( buf );
        }

        {
            mrv::media bg = v->background();
            if ( bg )
            {
                cmd = N_("CurrentBGImage \"");

                CMedia* img = bg->image();
                cmd += img->fileroot();

                sprintf( buf, "\" %" PRId64 " %" PRId64, img->first_frame(),
                         img->last_frame() );
                cmd += buf;
                deliver( cmd );

            }

            mrv::media fg = v->foreground();
            if ( !fg ) {
                v->network_active( true );
                return false;
            }

            CMedia* img = fg->image();

            ImageView::VRType t = v->vr();
            if ( t == ImageView::kVRSphericalMap )
                sprintf(buf, N_("VRSpherical 1"));
            else if ( t == ImageView::kVRCubeMap )
                sprintf(buf, N_("VRCubic 1"));
            else
            {
                sprintf(buf, N_("VRSpherical 0"));
                deliver( buf );
                sprintf(buf, N_("VRCubic 0"));
                deliver( buf );
            }
            deliver( buf );

            sprintf(buf, N_("VRangle %g"), v->vr_angle() );
            deliver( buf );

            sprintf(buf, N_("Spin %g %g"), v->spin_x(), v->spin_y() );
            deliver( buf );

            sprintf( buf, N_("Looping %d"), (int)img->looping() );
            deliver( buf );

            CMedia::StereoOutput out = img->stereo_output();
            sprintf( buf, "StereoOutput %d", out );
            deliver( buf );

            CMedia::StereoInput in = img->stereo_input();
            sprintf( buf, "StereoInput %d", in );
            deliver( buf );

        }

        int idx = v->ghost_previous();
        sprintf( buf, "GhostPrevious %d", idx );
        deliver( buf );

        idx = v->ghost_next();
        sprintf( buf, "GhostNext %d", idx );
        deliver( buf );



        sprintf(buf, N_("Zoom %g"), v->zoom() );
        deliver( buf );

        sprintf(buf, N_("Offset %g %g"), v->offset_x(), v->offset_y() );
        deliver( buf );

        sprintf(buf, N_("Rotation %g %g"), v->rot_x(), v->rot_y() );
        deliver( buf );

        sprintf(buf, N_("Spin %g %g"), v->spin_x(), v->spin_y() );
        deliver( buf );

        sprintf(buf, N_("OCIO %d"), (int)mrv::Preferences::use_ocio );
        deliver( buf );

        const char* const config = v->main()->uiPrefs->uiPrefsOCIOConfig->value();
        sprintf(buf, N_("OCIOConfig \"%s\""), config );
        deliver( buf );

        const std::string& display = mrv::Preferences::OCIO_Display;
        const std::string& view = mrv::Preferences::OCIO_View;
        sprintf(buf, N_("OCIOView \"%s\" \"%s\""),
                display.c_str(), view.c_str() );
        deliver( buf );

        sprintf(buf, N_("UseLUT %d"), (int)v->use_lut() );
        deliver( buf );

        sprintf(buf, N_("SafeAreas %d"), (int)v->safe_areas() );
        deliver( buf );

        sprintf(buf, N_("Volume %g"), v->volume() );
        deliver( buf );

        sprintf(buf, N_("Gain %g"), v->gain() );
        deliver( buf );

        sprintf(buf, N_("Gamma %g"), v->gamma() );
        deliver( buf );

        sprintf(buf, N_("ShowPixelRatio %d"),
                (int)v->show_pixel_ratio() );
        deliver( buf );

        sprintf(buf, N_("Normalize %d"), (int)v->normalize() );
        deliver( buf );

        sprintf(buf, N_("Mask %g"), v->masking() );
        deliver( buf );

        sprintf( buf, N_("FPS %g"), v->fps() );
        deliver( buf );



        sprintf( buf, N_("UpdateLayers") );
        deliver( buf );

        const char* lbl = v->get_layer_label(v->channel());
        sprintf(buf, N_("Channel %d %s"), v->channel(),
                lbl ? lbl : "(null)"  );
        deliver( buf );


        const mrv::Rectd& s = v->selection();
        if ( s.w() != 0 )
        {
            sprintf( buf, N_("Selection %g %g %g %g"), s.x(), s.y(),
                     s.w(), s.h() );
            deliver( buf );
        }

        browser()->redraw();
        v->redraw();

        ok = true;
    }
    else if ( cmd == N_("stop") )
    {
        int64_t f;
        is >> f;

        {
            ImageView::Command c;
            c.type = ImageView::kStopVideo;

            c.frame = f;
            v->commands.push_back( c );
        }


        ok = true;
    }
    else if ( cmd == N_("playfwd") )
    {
        ImageView::Command c;
        c.type = ImageView::kPlayForwards;

        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("playback") )
    {
        ImageView::Command c;
        c.type = ImageView::kPlayBackwards;

        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("seek") )
    {
        int64_t f;
        is >> f;

        ImageView::Command c;
        c.type = ImageView::kSeek;

        c.frame = f;
        v->commands.push_back( c );

        ok = true;
    }
    else if ( cmd == N_("MediaInfoWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kMEDIA_INFO_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kMEDIA_INFO_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("ColorInfoWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kCOLOR_AREA_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kCOLOR_AREA_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("ColorControlWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kCOLOR_CONTROL_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kCOLOR_CONTROL_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("GL3dView") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::k3D_VIEW_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::k3D_VIEW_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("StereoOptions") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kSTEREO_OPTIONS_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kSTEREO_OPTIONS_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("StereoOutput") )
    {
        int x;
        is >> x;
        v->stereo_output( (CMedia::StereoOutput) x );
        ok = true;
    }
    else if ( cmd == N_("StereoInput") )
    {
        int x;
        is >> x;
        v->stereo_input( (CMedia::StereoInput) x );
        ok = true;
    }
    else if ( cmd == N_("PaintTools") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kPAINT_TOOLS_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kPAINT_TOOLS_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("HistogramWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kHISTOGRAM_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kHISTOGRAM_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("VectorscopeWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kVECTORSCOPE_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kVECTORSCOPE_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }
    else if ( cmd == N_("WaveformWindow") )
    {
        int x;
        is >> x;
        ImageView::Command c;

        if ( x )
        {
            c.type = ImageView::kWAVEFORM_WINDOW_SHOW;
        }
        else
        {
            c.type = ImageView::kWAVEFORM_WINDOW_HIDE;
        }
        v->commands.push_back( c );
        ok = true;
    }

    if (!ok) LOG_ERROR( "Parsing failed for " << cmd << " " << s );

    v->network_active( true );


#ifdef OSX
    const char* loc = setlocale( LC_MESSAGES, NULL );
    setlocale( LC_NUMERIC, loc );
#else
    setlocale( LC_NUMERIC, NULL );
#endif

    return ok;
}


//------------------------ --------------------------

//----------------------------------------------------------------------

namespace {
const char* const kModule = "server";
}

tcp_session::tcp_session(boost::asio::io_service& io_service,
                         ViewerUI* const v) :
    Parser(io_service, v)
{

    // The non_empty_output_queue_ deadline_timer is set to pos_infin
    // whenever the output queue is empty. This ensures that the output
    // actor stays asleep until a message is put into the queue.
    non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
}

tcp_session::~tcp_session()
{
    stop();
}

tcp::socket& tcp_session::socket()
{
    return socket_;
}

// Called by the server object to initiate the four actors.
void tcp_session::start()
{
    connected = true;

    mrv::ImageView* v = view();
    if (!v) return;

    {
        Mutex& m = v->_clients_mtx;
        SCOPED_LOCK( m );

        v->_clients.push_back( this );
    }

    start_read();

    //	std::cerr << "start1: " << socket_.native_handle() << std::endl;
    // input_deadline_.async_wait(
    //                            boost::bind(&tcp_session::check_deadline,
    //                                        shared_from_this(),
    //                                        &input_deadline_));

    await_output();

    // std::cerr << "start2: " << socket_.native_handle() << std::endl;
    // output_deadline_.async_wait(
    //                             boost::bind(&tcp_session::check_deadline,
    //                                         shared_from_this(),
    //                                         &output_deadline_));

}

bool tcp_session::stopped()
{
    return !socket_.is_open();
}

void tcp_session::deliver( const std::string& msg )
{
    SCOPED_LOCK( mtx );

#ifdef DEBUG_COMMANDS
    if ( view()->show_pixel_ratio() )
        if ( msg.find( "Offset" ) == std::string::npos )
            LOG_INFO( "SEND CMD: " << msg );
#endif

    output_queue_.push_back(msg + "\n");

    // Signal that the output queue contains messages. Modifying the expiry
    // will wake the output actor, if it is waiting on the timer.
    non_empty_output_queue_.expires_from_now(boost::posix_time::seconds(0));
}


void tcp_session::stop()
{
    connected = false;

    deadline_.cancel();
    socket_.close();
    non_empty_output_queue_.cancel();

    if ( !ui ) return;

    mrv::ImageView* v = ui->uiView;
    if ( ui && v )
    {
        Mutex& m = v->_clients_mtx;
        SCOPED_LOCK( m );

        ParserList& p = v->_clients;
        ParserList::iterator i = p.begin();

        for ( ; i != p.end(); )
        {
            if ( *i == this )
            {
                LOG_CONN( _("Removed client ") << this );
                i = p.erase( i );
                break;
            }
            else
            {
                ++i;
            }
        }

        LOG_CONN( _("Number of connections now: ") << p.size() );
        ui = NULL;
    }
}


void tcp_session::start_read()
{
    // Set a deadline for the read operation.

    // input_deadline_.expires_from_now(boost::posix_time::seconds(30));
    // input_deadline_.expires_at(boost::posix_time::pos_infin);

    // Start an asynchronous operation to read a newline-delimited message.
    boost::asio::async_read_until(
        socket(), input_buffer_, '\n',
        boost::bind( &tcp_session::handle_read, shared_from_this(),
                     boost::asio::placeholders::error ) );
}

void tcp_session::handle_read(const boost::system::error_code& ec)
{
    if (stopped())
        return;

    if (!ec)
    {
        // Extract the newline-delimited message from the buffer.
        std::istream is(&input_buffer_);
        is.exceptions( std::ifstream::failbit | std::ifstream::badbit |
                       std::ifstream::eofbit );

        std::string id = boost::lexical_cast<std::string>(socket().remote_endpoint() );

        try {
            std::string msg;
            is.clear();
            if ( std::getline(is, msg) )
            {
                if ( msg == N_("OK") || msg.empty() )
                {
                }
                else if ( msg == N_("Not OK") )
                {
                    LOG_CONN( N_("Not OK") );
                }
                else if ( parse( msg ) )
                {
                    // send message to all clients
                    // We need to do this to update multiple clients.
                    // Note that the original client that sent the
                    // message will be skipped as it is IDed.
                    write( msg, id );
                }
                else
                {
                    write( N_("Not OK"), "" );
                }
            }
        }
        catch ( const std::exception& e )
        {
            LOG_ERROR( "Failure in getline " << e.what() );
        }



        start_read();
    }
    else
    {
        stop();
    }
}

void tcp_session::await_output()
{

    if (stopped())
        return;


    try {
        if (output_queue_.empty())
        {
            // There are no messages that are ready to be sent. The actor goes to
            // sleep by waiting on the non_empty_output_queue_ timer. When a new
            // message is added, the timer will be modified and the actor will
            // wake.

            non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
            non_empty_output_queue_.async_wait(
                boost::bind(&tcp_session::await_output,
                            shared_from_this())
            );
        }
        else
        {
            start_write();
        }
    }
    catch( const std::exception& e )
    {
        LOG_ERROR( "await_output exception.  " << e.what() );
    }
}

void tcp_session::start_write()
{
    SCOPED_LOCK( mtx );

    // Start an asynchronous operation to send a message.
    boost::asio::async_write(socket(),
                             boost::asio::buffer(output_queue_.front()),
                             boost::bind(&tcp_session::handle_write,
                                         shared_from_this(),
                                         boost::asio::placeholders::error));
}

void tcp_session::handle_write(const boost::system::error_code& ec)
{
    if (stopped())
        return;

    if (!ec)
    {
        output_queue_.pop_front();

        await_output();
    }
    else
    {
        stop();
    }
}

void tcp_session::check_deadline(deadline_timer* deadline)
{
    if (stopped())
        return;

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline->expires_at() <= deadline_timer::traits_type::now())
    {
        // The deadline has passed. Stop the session. The other actors will
        // terminate as soon as possible.
        stop();
    }
    else
    {
        // Put the actor back to sleep.
        deadline->async_wait(
            boost::bind(&tcp_session::check_deadline,
                        shared_from_this(), deadline));
    }
}


//----------------------------------------------------------------------

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by two "actors" that persist for the lifetime of the
// session object, one for input and one for output:
//
//  +----------------+                     +----------------+
//  |                |                     |                |
//  | check_deadline |<---+                | check_deadline |<---+
//  |                |    | async_wait()   |                |    | async_wait()
//  +----------------+    |  on input      +----------------+    |  on output
//              |         |  deadline                  |         |  deadline
//              +---------+                            +---------+
//
// If either deadline actor determines that the corresponding deadline has
// expired, the socket is closed and any outstanding operations are cancelled.
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character:
//
//  +------------+
//  |            |
//  | start_read |<---+
//  |            |    |
//  +------------+    |
//          |         |
//  async_- |    +-------------+
//   read_- |    |             |
//  until() +--->| handle_read |
//               |             |
//               +-------------+
//
// The deadline for receiving a complete message is 30 seconds. If a non-empty
// message is received, it is delivered to all subscribers. If a heartbeat (a
// message that consists of a single newline character) is received, a heartbeat
// is enqueued for the client, provided there are no other messages waiting to
// be sent.
//
// The output actor is responsible for sending messages to the client:
//
//  +--------------+
//  |              |<---------------------+
//  | await_output |                      |
//  |              |<---+                 |
//  +--------------+    |                 |
//      |      |        | async_wait()    |
//      |      +--------+                 |
//      V                                 |
//  +-------------+               +--------------+
//  |             | async_write() |              |
//  | start_write |-------------->| handle_write |
//  |             |               |              |
//  +-------------+               +--------------+
//
// The output actor first waits for an output message to be enqueued. It does
// this by using a deadline_timer as an asynchronous condition variable. The
// deadline_timer will be signalled whenever the output queue is non-empty.
//
// Once a message is available, it is sent to the client. The deadline for
// sending a complete message is 30 seconds. After the message is successfully
// sent, the output actor again waits for the output queue to become non-empty.
//


server::server(boost::asio::io_service& io_service,
               const tcp::endpoint& endpoint,
               ViewerUI* v)
    : io_service_(io_service),
      acceptor_(io_service),
      ui_( v )
{

    acceptor_.open(endpoint.protocol());

    boost::asio::socket_base::reuse_address option(true);
    acceptor_.set_option(option);
    acceptor_.bind(endpoint);
    acceptor_.listen();

    start_accept();
}

server::~server()
{
    io_service_.stop();
}

void server::start_accept()
{
    // tcp_session_ptr new_session(new tcp_session(io_service_, ui_));
    tcp_session_ptr new_session(
        boost::make_shared<tcp_session>(
            boost::ref(
                io_service_
            ),
            boost::ref(ui_)
        )
    );

    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session, _1));
}

void server::handle_accept(tcp_session_ptr session,
                           const boost::system::error_code& ec)
{
    if (!ec)
    {
        session->start();
    }

    start_accept();
}


void server::create(ViewerUI* ui)
{
    unsigned short port = (unsigned short) ui->uiConnection->uiServerPort->value();
    ServerData* data = new ServerData;
    data->port = port;
    data->ui = ui;

    ui->uiConnection->uiCreate->label( _("Disconnect") );
    ui->uiConnection->uiClientGroup->deactivate();

    boost::thread t( boost::bind( mrv::server_thread,
                                  data ) );
    t.detach();
}

void server::remove( ViewerUI* ui )
{
    if ( !ui || !ui->uiView ) return;

    using namespace mrv;

    ImageView* v = ui->uiView;

    Mutex& m = v->_clients_mtx;
    SCOPED_LOCK( m );

    ParserList::iterator i = v->_clients.begin();
    ParserList::iterator e = v->_clients.end();

    for ( ; i != e; ++i )
    {
        (*i)->stop();
    }

    if ( ui->uiConnection )
    {
        ui->uiConnection->uiCreate->label( _("Create") );
        ui->uiConnection->uiClientGroup->activate();
    }

    v->_clients.clear();
    v->_server.reset();
}


//----------------------------------------------------------------------

void server_thread( const ServerData* s )
{
    try
    {

        boost::asio::io_service io_service;

        tcp::endpoint listen_endpoint(tcp::v4(), s->port);


        s->ui->uiView->_server = boost::make_shared< server >( boost::ref(io_service),
                                                               listen_endpoint,
                                                               s->ui);


        LOG_CONN( _("Created server at port ") << s->port );

        io_service.run();
        LOG_CONN( _("Closed server at port ") << s->port );

        delete s;

    }
    catch (const std::exception& e)
    {
        LOG_ERROR( "Exception: " << e.what() );
    }
}

} // namespace mrv
