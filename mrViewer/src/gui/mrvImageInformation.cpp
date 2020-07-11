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
/**
 * @file   mrvImageInformation.cpp
 * @author gga
 * @date   Wed Jul 11 18:47:58 2007
 *
 * @brief  Class used to display information about an image
 *
 *
 */

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "core/mrvI8N.h"
#include <iostream>
using namespace std;

#include <algorithm>

#include <boost/regex.hpp>

#include <ImfBoxAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfDeepImageStateAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfEnvmapAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfKeyCodeAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfOpaqueAttribute.h>
#include <ImfPreviewImageAttribute.h>
#include <ImfRationalAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfStringVectorAttribute.h>
#include <ImfTimeCodeAttribute.h>
#include <ImfTileDescriptionAttribute.h>
#include <ImfVecAttribute.h>

#include <core/mrvRectangle.h>
#include <FL/Fl_Int_Input.H>
#include <FL/names.h>

#include "core/oiioImage.h"
#include "core/mrvString.h"
#include "core/aviImage.h"
#include "core/exrImage.h"
#include "core/R3dImage.h"
#include "core/brawImage.h"
#include "mrvImageInformation.h"
#include "mrvFileRequester.h"
#include "mrvPreferences.h"
#include "mrvMath.h"
#include "mrvMedia.h"
#include "mrvImageView.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvImageBrowser.h"
#include "mrViewer.h"
#include "CMedia.h"
#include "mrvIO.h"

#include "mrvI8N.h"
#include "gui/MyPack.h"


namespace {
const char* kModule = "info";
}

namespace mrv
{

    using namespace R3DSDK;

struct CtlLMTData
{
    Fl_Widget* widget;
    size_t idx;
};

typedef std::vector< CtlLMTData* > LMTData;

static const Fl_Color kTitleColors[] = {
    0x608080ff,
    0x808060ff,
    0x606080ff,
    0x608060ff,
    0x806080ff,
};


static LMTData widget_data;

static const unsigned int kSizeOfTitleColors = ( sizeof(kTitleColors) /
        sizeof(Fl_Color) );

static const Fl_Color kRowColors[] = {
    0x808080ff,
    0xa0a0a0ff,
};

static const unsigned int kSizeOfRowColors = ( sizeof(kRowColors) /
        sizeof(Fl_Color) );

static const int kMiddle = 220;

static void change_stereo_image( Fl_Button* w, mrv::ImageInformation* info )
{
    static CMedia* last = NULL;
    CMedia* img = info->get_image();
    if ( img->is_stereo() && img != last && img->right_eye() )
    {
        last = img;
        img = img->right_eye();
        info->set_image( img );
        w->label( _("Right View") );
    }
    else
    {
        if ( last )
        {
            info->set_image( last );
            last = NULL;
            w->label( _("Left View") );
        }
    }
    info->filled = false;
    info->refresh();
}

mrv::Choice *uiType=(mrv::Choice *)0;

Fl_Input *uiKey=(Fl_Input *)0;

Fl_Input *uiValue=(Fl_Input *)0;

mrv::Choice *uiKeyRemove=(mrv::Choice *)0;

static void cb_OK(Fl_Button*, Fl_Window* v) {
    v->damage( FL_DAMAGE_ALL );  // like fltk2.0's exec return true
    v->hide();
}

static void cb_Cancel(Fl_Button*, Fl_Window* v) {
    v->hide();
}


static void cb_uiType(mrv::Choice* o, void*) {
    std::string type = o->child( o->value() )->label();
    if ( type == _("String") )
        uiValue->value( _("Text") );
    else if ( type == _("Integer") )
        uiValue->value( N_("15") );
    else if ( type == _("Float") || type == _("Double") )
        uiValue->value( _("2.2") );
    else if ( type == N_("Timecode") )
        uiValue->value( _("00:00:00:00") );
    else if ( type == _("Rational") )
        uiValue->value( _("24000/1001") );
    else if ( type == _("Vector2 Integer") )
        uiValue->value( N_("2 5") );
    else if ( type == _("Vector2 Float") || type == _("Vector2 Double") )
        uiValue->value( _("2.2 5.1") );
    else if ( type == _("Vector3 Integer") )
        uiValue->value( N_("2 5 1") );
    else if ( type == _("Vector3 Float") || type == _("Vector3 Double") )
        uiValue->value( _("2.2 5.1 1.4") );
    else if ( type == _("Box2 Integer") )
        uiValue->value( N_("2 5  10 20") );
    else if ( type == _("Box2 Float")  )
        uiValue->value( _("0.2 5.1  10.5 20.2") );
    else if ( type == _("Chromaticities") )
        uiValue->value( _("0.64 0.33  0.3 0.6  0.15 0.06  0.3127 0.3290") );
    else if ( type == _("M33 Float") || type == _("M33 Double")  )
        uiValue->value( _("1.0 0.0 0.0  0.0 1.0 0.0  0.0 0.0 1.0") );
    else if ( type == _("M44 Float") || type == _("M44 Double") )
        uiValue->value( _("1.0 0.0 0.0 0.0  0.0 1.0 0.0 0.0  "
                         "0.0 0.0 1.0 0.0  0.0 0.0 0.0 1.0") );
    else if ( type == _("KeyCode") )
        uiValue->value( _("** multivalue **") );
}

static Fl_Double_Window* make_attribute_add_window() {
    Fl_Double_Window* w;
    {   Fl_Double_Window* o = new Fl_Double_Window(405, 200);
        w = o;
        o->label( _("Add Attribute") );
        o->begin();
        Fl_Group* g = new Fl_Group( 0, 0, 405, 200 );
        g->box( FL_UP_BOX );
        {   mrv::Choice* o = uiType = new mrv::Choice( 10, 30, 390, 25, _("Type") );
            o->align(FL_ALIGN_TOP);
            o->add( _("String") );
            o->add( _("Integer") );
            o->add( _("Float") );
            o->add( _("Double") );
            o->add( _("Rational") );
            o->add( _("M33 Float") );
            o->add( _("M44 Float") );
            o->add( _("M33 Double") );
            o->add( _("M44 Double") );
            o->add( N_("Timecode") );
            o->add( _("Box2 Integer") );
            o->add( _("Box2 Float") );
            o->add( _("Vector2 Integer") );
            o->add( _("Vector2 Float") );
            o->add( _("Vector2 Double") );
            o->add( _("Vector3 Integer") );
            o->add( _("Vector3 Float") );
            o->add( _("Vector3 Double") );
            o->add( _("Chromaticities") );
            o->add( _("KeyCode") );
            o->callback( (Fl_Callback*) cb_uiType, (void*)w );
            o->value( 9 );
        }
        {   Fl_Input* o = uiKey = new Fl_Input(10, 75, 390, 25, _("Keyword"));
            o->value( N_("timecode") );
            o->align(FL_ALIGN_TOP);
        }
        {   Fl_Input* o = uiValue = new Fl_Input(10, 120, 390, 25, _("Value"));
            o->value( N_("00:00:00:00") );
            o->align(FL_ALIGN_TOP);
        }
        {   Fl_Button* o = new Fl_Button(115, 150, 86, 41, _("OK"));
            o->callback((Fl_Callback*)cb_OK, (void*)(w));
        }
        {   Fl_Button* o = new Fl_Button(224, 150, 93, 41, _("Cancel"));
            o->callback((Fl_Callback*)cb_Cancel, (void*)(w));
        }
        g->end();
        o->end();
        o->set_modal();
        o->resizable(o);
    }
    return  w;
}

static Fl_Window* make_remove_window( CMedia::Attributes& attrs ) {
    Fl_Window* w;
    {   Fl_Window* o = new Fl_Window(405, 120);
        w = o;
        o->label( _( "Remove Attribute" ) );
        o->begin();
        {
            Fl_Group* g = new Fl_Group(0, 0, 405, 120);
            g->box( FL_FLAT_BOX );
        {   mrv::Choice* o =
                uiKeyRemove = new mrv::Choice(10, 35, 390, 25, _("Keyword"));
            o->align(FL_ALIGN_TOP);
            CMedia::Attributes::const_iterator i = attrs.begin();
            CMedia::Attributes::const_iterator e = attrs.end();
            for ( ; i != e; ++i )
            {
                o->add( i->first.c_str() );
            }
        }
        {   Fl_Button* o = new Fl_Button(115, 70, 86, 41, _("OK"));
            o->callback((Fl_Callback*)cb_OK, (void*)(w));
        }
        {   Fl_Button* o = new Fl_Button(224, 70, 93, 41, _("Cancel"));
            o->callback((Fl_Callback*)cb_Cancel, (void*)(w));
        }
        g->end();
        }
        o->end();
        o->set_modal();
        o->resizable(o);
    }
    return  w;
}

static void add_attribute( CMedia::Attributes& attrs,
                           CMedia* img )
{
    std::string type = _("String");
    if ( uiType ) type = uiType->child( uiType->value() )->label();
    std::string key = uiKey->value();
    std::string value = uiValue->value();
    if ( attrs.find( key ) != attrs.end() )
    {
        char buf[128];
        sprintf( buf, _("'%s' attribute already exists"), key.c_str() );
        mrvALERT( buf );
        return;
    }
    else if ( type == N_("Timecode") )
    {
        if ( attrs.find( N_("Video timecode") ) != attrs.end() )
        {
            mrvALERT( _("Video timecode attribute already exists") );
            return;
        }

        Imf::TimeCode t = CMedia::str2timecode( value );
        if ( key.find("timecode") != std::string::npos )
            img->process_timecode( t );
        img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
        Imf::TimeCodeAttribute attr( t );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("String") )
    {
        Imf::StringAttribute attr( value );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Integer") )
    {
        int v = atoi( value.c_str() );
        Imf::IntAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Float") )
    {
        float v = (float)atof( value.c_str() );
        Imf::FloatAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Double") )
    {
        double v = atof( value.c_str() );
        Imf::DoubleAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Rational") )
    {
        stringArray comp;
        split( comp, value, '/' );
        if ( comp.size() == 2 )
        {
            int n = atoi( comp[0].c_str() );
            unsigned d = (unsigned)atoi( comp[1].c_str() );
            Imf::Rational v( n, d );
            Imf::RationalAttribute attr( v );
            attrs.insert( std::make_pair(key, attr.copy() ) );
            return;
        }
    }
    else if ( type == _("Vector2 Integer") )
    {
        int x,y;
        int n = sscanf( value.c_str(), "%d %d", &x, &y );
        if ( n != 2 )
        {
            mrvALERT( _("Could not find two integers for vector") );
            return;
        }
        Imf::V2iAttribute attr( Imath::V2i( x, y ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Vector2 Float") )
    {
        float x,y;
        int n = sscanf( value.c_str(), "%g %g", &x, &y );
        if ( n != 2 )
        {
            mrvALERT( _("Could not find two floats for vector") );
            return;
        }
        Imf::V2fAttribute attr( Imath::V2f( x, y ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Vector2 Double") )
    {
        double x,y;
        int n = sscanf( value.c_str(), "%lg %lg", &x, &y );
        if ( n != 2 )
        {
            mrvALERT( _("Could not find two doubles for vector") );
            return;
        }
        Imf::V2dAttribute attr( Imath::V2d( x, y ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Vector3 Integer") )
    {
        int x,y,z;
        int n = sscanf( value.c_str(), "%d %d %d", &x, &y, &z );
        if ( n != 3 )
        {
            mrvALERT( _("Could not find three integers for vector") );
            return;
        }
        Imf::V3iAttribute attr( Imath::V3i( x, y, z ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Vector3 Float") )
    {
        float x,y,z;
        int n = sscanf( value.c_str(), "%g %g %g", &x, &y, &z );
        if ( n != 3 )
        {
            mrvALERT( _("Could not find three floats for vector") );
            return;
        }
        Imf::V3fAttribute attr( Imath::V3f( x, y, z ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Vector3 Double") )
    {
        double x, y, z;
        int n = sscanf( value.c_str(), "%lg %lg %lg", &x, &y, &z );
        if ( n != 3 )
        {
            mrvALERT( _("Could not find three doubles for vector") );
            return;
        }
        Imf::V3dAttribute attr( Imath::V3d( x, y, z ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Box2 Integer") )
    {
        int x, y, w, h;
        int n = sscanf( value.c_str(), "%d %d  %d %d", &x, &y, &w, &h );
        if ( n != 4 )
        {
            mrvALERT( _("Could not find four integers for box") );
            return;
        }
        Imf::Box2iAttribute attr( Imath::Box2i( Imath::V2i( x, y ),
                                                Imath::V2i( w, h ) ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Box2 Float") )
    {
        float x, y, w, h;
        int n = sscanf( value.c_str(), "%g %g  %g %g", &x, &y, &w, &h );
        if ( n != 4 )
        {
            mrvALERT( _("Could not find four floats for box") );
            return;
        }
        Imf::Box2fAttribute attr( Imath::Box2f( Imath::V2f( x, y ),
                                                Imath::V2f( w, h ) ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("M33 Float") )
    {
        float m00, m01, m02, m10, m11, m12, m20, m21, m22;
        int n = sscanf( value.c_str(), "%g %g %g  %g %g %g  %g %g %g",
                        &m00, &m01, &m02,
                        &m10, &m11, &m12,
                        &m20, &m21, &m22);
        if ( n != 9 )
        {
            mrvALERT( _("Could not find nine floats for matrix 3x3") );
            return;
        }
        Imf::M33fAttribute attr( Imath::M33f( m00, m01, m02,
                                              m10, m11, m12,
                                              m20, m21, m22 ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("Chromaticities") )
    {
        float rx, ry, gx, gy, bx, by, wx, wy;
        int n = sscanf( value.c_str(), "%g %g  %g %g  %g %g  %g %g",
                        &rx, &ry, &gx, &gy, &bx, &by, &wx, &wy );
        if ( n != 8 )
        {
            mrvALERT( _("Could not find eight floats for chromaticities") );
            return;
        }
        Imf::Chromaticities val( Imath::V2f( rx, ry ),
                                 Imath::V2f( gx, gy ),
                                 Imath::V2f( bx, by ),
                                 Imath::V2f( wx, wy ) );
        Imf::ChromaticitiesAttribute attr( val );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("M33 Double") )
    {
        double m00, m01, m02, m10, m11, m12, m20, m21, m22;
        int n = sscanf( value.c_str(), "%lg %lg %lg  %lg %lg %lg  %lg %lg %lg",
                        &m00, &m01, &m02,
                        &m10, &m11, &m12,
                        &m20, &m21, &m22);
        if ( n != 9 )
        {
            mrvALERT( _("Could not find nine doubles for matrix 3x3") );
            return;
        }
        Imf::M33dAttribute attr( Imath::M33d( m00, m01, m02,
                                              m10, m11, m12,
                                              m20, m21, m22 ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("M44 Float") )
    {
        float m00, m01, m02, m03,
              m10, m11, m12, m13,
              m20, m21, m22, m23,
              m30, m31, m32, m33;
        int n = sscanf( value.c_str(),
                        "%g %g %g %g  "
                        "%g %g %g %g  "
                        "%g %g %g %g  "
                        "%g %g %g %g",
                        &m00, &m01, &m02, &m03,
                        &m10, &m11, &m12, &m13,
                        &m20, &m21, &m22, &m23,
                        &m30, &m31, &m32, &m33 );
        if ( n != 16 )
        {
            mrvALERT( _("Could not find sixteen floats for matrix 4x4") );
            return;
        }
        Imf::M44fAttribute attr( Imath::M44f( m00, m01, m02, m03,
                                              m10, m11, m12, m13,
                                              m20, m21, m22, m23,
                                              m30, m31, m32, m33 ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("M44 Double") )
    {
        double m00, m01, m02, m03,
               m10, m11, m12, m13,
               m20, m21, m22, m23,
               m30, m31, m32, m33;
        int n = sscanf( value.c_str(),
                        "%lg %lg %lg %lg  "
                        "%lg %lg %lg %lg  "
                        "%lg %lg %lg %lg  "
                        "%lg %lg %lg %lg",
                        &m00, &m01, &m02, &m03,
                        &m10, &m11, &m12, &m13,
                        &m20, &m21, &m22, &m23,
                        &m30, &m31, &m32, &m33 );
        if ( n != 16 )
        {
            mrvALERT( _("Could not find sixteen doubles for matrix 4x4") );
            return;
        }
        Imf::M44dAttribute attr( Imath::M44d( m00, m01, m02, m03,
                                              m10, m11, m12, m13,
                                              m20, m21, m22, m23,
                                              m30, m31, m32, m33 ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
        return;
    }
    else if ( type == _("KeyCode") )
    {
        Imf::KeyCode k;
        Imf::KeyCodeAttribute attr( k );
        attrs.insert( std::make_pair( key, attr.copy() ) );
        return;
    }
    else
    {
        mrvALERT( _("Unknown data type for keyword '") << key << _("' type '")
                  << type << "'" );
    }
}


static
void toggle_modify_attribute( const std::string& key, ImageInformation* info )
{
    if ( info->m_attributes == NULL ) {
        LOG_ERROR( _("attributes not found") );
        return;
    }

    mrv::CollapsibleGroup* g =
    dynamic_cast< mrv::CollapsibleGroup* >( info->m_attributes );

    if ( g == NULL ) {
        LOG_ERROR( _("CollapsibleGroup not found") );
        return;
    }

    //if ( g->children() < 2 ) return;
    MyPack* p = dynamic_cast< MyPack* >( g->child(1) ); // pack
    if ( p == NULL ) {
        LOG_ERROR( _("MyPack not found") );
        return;
    }

    // Index is 3 for two scrollbars and a root node
    mrv::Table* t = dynamic_cast< mrv::Table* >( p->child(0) );
    if ( t == NULL )
    {
        LOG_ERROR( _("Not a table") );
        return;
    }

    for ( int r = 0; r < t->rows(); ++r )
    {
        int i = 2 * r;
        if ( i >= t->children() ) break;
        Fl_Group* sg = dynamic_cast< Fl_Group* >( t->child(i) );
        if ( !sg ) {
            LOG_ERROR( _("Not group child in table row ") << i );
            break;
        }
        Fl_Widget* w = sg->child(0);
        if ( ! w->label() ) continue;
        if ( key.rfind( _("All") ) != std::string::npos ||
                key == w->label() ||
                key + ".filmMfcCode" == w->label() ||
                key + ".filmType" == w->label() ||
                key + ".prefix" == w->label() ||
                key + ".count" == w->label() ||
                key + ".perfOffset" == w->label() ||
                key + ".perfsPerFrame" == w->label() ||
                key + ".perfsPerCount" == w->label() )
        {
            w = (Fl_Widget*)t->child(i+1);
            if ( w->active() )
            {
                w->deactivate();
            }
            else
            {
                w->activate();
            }
        }
    }
}


static
void toggle_modify_attribute_cb( Fl_Menu_Button* widget,
                                 ImageInformation* info )
{
    std::string key = widget->text();
    toggle_modify_attribute( key, info );
}

static void add_attribute_cb( Fl_Box* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;

    Fl_Group::current(0);
    Fl_Double_Window* w = make_attribute_add_window();
    w->show();
    while ( w->visible() )
        Fl::check();

    if ( ! (w->damage() & FL_DAMAGE_ALL) )
        return;

    std::string key = uiKey->value();
    std::string value = uiValue->value();

    CMedia::Attributes& attrs = img->attributes();
    add_attribute( attrs, img );
    info->filled = false;
    info->refresh();
    ViewerUI* ui = info->main();
    ui->uiView->redraw();
    delete w;
}


static void remove_attribute_cb( Fl_Box* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;

    CMedia::Attributes& attrs = img->attributes();

    Fl_Group::current(0);
    Fl_Window* w = make_remove_window( attrs );
    w->set_modal();
    w->show();
    while ( w->visible() )
        Fl::check();


    if ( ! ( w->damage() & FL_DAMAGE_ALL ) ) return;

    char picked[1024];
    int ok = uiKeyRemove->item_pathname( picked, sizeof(picked)-1 );
    if ( ok < 0 )
    {
        LOG_ERROR( _("item_pathname returned ") << ok );
        return;
    }

    std::string key = "/";
    if ( strlen(picked) > 0 ) key = picked;

    CMedia::Attributes::iterator i = attrs.find( key );
    if ( i == attrs.end() && key[0] == '/' ) {
        key = key.substr( 1, key.size() );
        i = attrs.find( key );
    }
    if ( i == attrs.end() )
    {
        char buf[128];
        sprintf( buf, _("No attribute named '%s'"), key.c_str() );
        mrvALERT( buf );
        return;
    }
    if ( key.find( N_("timecode") ) != std::string::npos )
    {
        img->timecode( 0 );
        img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
    }
    attrs.erase( i );
    info->filled = false;
    info->refresh();
}


ImageView* ImageInformation::view() const
{
    return uiMain->uiView;
}

int idx = -1;
std::string old_match;
int num_matches = 0;
int match_goal = 1;

static bool regex_match( float i, const std::string& regex, std::string text )
{
  boost::regex expr{ regex };
  if ( boost::regex_search( text, expr ) ) {
    ++num_matches;
    if ( match_goal == num_matches )
      {
        idx = int(i - 0.5f);
        return true;
      }
  }
  return false;
}


int search_table( mrv::Table* t, float& row, const std::string& match )
{
  int rows = t->children();
  for ( int i = 0; i < rows; ++i )
    {
      if ( ! t->parent()->visible() ) continue;

      row += 0.5;
      Fl_Widget* w = t->child(i);
    tryagain:
      if ( dynamic_cast< Fl_Input* >( w ) != NULL )
        {
          Fl_Input* input = (Fl_Input*) w;
          if ( regex_match( row, match, input->value() ) )
            break;
        }
      else if ( dynamic_cast< Fl_Output* >( w ) != NULL )
        {
          Fl_Output* output = (Fl_Output*) w;
          if ( regex_match( row, match, output->value() ) )
            break;
        }
      else if ( dynamic_cast< Fl_Group* >( w ) != NULL )
        {
          Fl_Group* g = (Fl_Group*) w;
          w = (Fl_Widget*) g->child(0);
          goto tryagain;
        }
      else
        {
          if ( !w->label() ) continue;
          if ( regex_match( row, match, w->label() ) )
            break;
        }
    }

  return idx;
}

static void search_cb( Fl_Widget* o, mrv::ImageInformation* info )
{
  std::string match = info->m_entry->value();
  num_matches = 0;

  if ( match == old_match )
    {
      ++match_goal;
    }
  else
    {
      match_goal = 1;
    }

  old_match = match;
  idx = -1;

  int H = info->line_height() + 1;
  int H2 = 56 - info->line_height();
  int H3 = 12 + info->line_height();

  MyPack* p = (MyPack*) info->m_image->child(1);
  mrv::Table* t = (mrv::Table*) p->child(0);

  float row = 0;
  int pos = info->line_height();
  if ( p->visible() ) pos += 32;

  int idx = search_table( t, row, match );
  if ( idx >= 0 ) {
    info->m_scroll->scroll_to( 0, pos + idx*H );
    return;
  }

  p = (MyPack*) info->m_video->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->m_scroll->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  p = (MyPack*) info->m_audio->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->m_scroll->scroll_to( 0, pos + idx*H );
        return;
      }
    }


  p = (MyPack*) info->m_subtitle->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->m_scroll->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  p = (MyPack*) info->m_attributes->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->m_scroll->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  match_goal = 0;

}


ImageInformation::ImageInformation( int x, int y, int w, int h,
                                    const char* l ) :
Fl_Group( x, y, w, h, l ),
img( NULL ),
menu( new Fl_Menu_Button( 0, 0, 0, 0, _("Attributes Menu") ) )
{
    menu->type( Fl_Menu_Button::POPUP3 );

    int sw = Fl::scrollbar_size();                // scrollbar width

    mrv::Recti r( x + Fl::box_dx(box()), y + Fl::box_dy(box()),
                  w - Fl::box_dw(box()), h - Fl::box_dh(box()));

    begin();

    Fl_Group* g = new Fl_Group( r.x(), r.y(), r.w(), 30 );
    {
      m_search = new Fl_Box( r.x(), r.y(), 80, 30, "Search" );
      m_search->box( FL_FLAT_BOX );

      m_entry  = new Fl_Input( r.x()+80, r.y(), r.w()-80, 30 );
      m_entry->textcolor( FL_BLACK );
      m_entry->callback( (Fl_Callback*) search_cb, this );
      // m_entry->when( FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED );
      m_entry->when( FL_WHEN_CHANGED | FL_WHEN_ENTER_KEY |
                     FL_WHEN_NOT_CHANGED );
    }
    g->end();

    m_scroll = new Fl_Scroll( r.x(), r.y()+30, r.w(), 800 );
    m_scroll->begin();

    m_all = new mrvPack( r.x(), r.y()+30, r.w()-sw, 800 );
    m_all->begin();

    m_button = new Fl_Button( r.x(), r.y()+30, r.w(), 40, _("Left View") );
    m_button->callback( (Fl_Callback*)change_stereo_image, this );
    m_button->hide();

    // CollapsibleGrop recalcs, we don't care its xyh sizes
    m_image = new mrv::CollapsibleGroup( 0, r.y()+70, r.w(),
                                         800, _("Main")  );

    m_video = new mrv::CollapsibleGroup( r.x(), r.y()+870,
                                         r.w(), 400, _("Video") );

    m_audio = new mrv::CollapsibleGroup( r.x(), r.y()+1270,
                                         r.w(), 400, _("Audio") );


    m_subtitle = new mrv::CollapsibleGroup( r.x(), r.y()+1670,
                                            r.w(), 400, _("Subtitle") );

    m_attributes  = new mrv::CollapsibleGroup( r.x(), r.y()+2070,
                                               r.w(), 400, _("Metadata")  );

    m_all->end();

    m_scroll->end();


    resizable( 0 );
    end();

    m_entry->take_focus();

    hide_tabs();

}


int ImageInformation::handle( int event )
{
    if ( ! filled || !img ) return 0;


    if ( event == FL_MOUSEWHEEL )
    {
        Fl::e_dy = Fl::event_dy() * 8;
    }
    else if ( event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE && img )
    {

        menu->add( _("Add Attribute"), 0,
                   (Fl_Callback*)add_attribute_cb,
                   this);
        {
            CMedia::Attributes& attrs = img->attributes();
            if ( !attrs.empty() )
            {
                CMedia::Attributes::iterator i = attrs.begin();
                CMedia::Attributes::iterator e = attrs.end();

                menu->add( _("Toggle Modify/All"), 0,
                           (Fl_Callback*)toggle_modify_attribute_cb,
                           this, FL_MENU_DIVIDER );
                for ( ; i != e; ++i )
                {
                    char buf[256];
                    sprintf( buf,  _("Toggle Modify/%s"),
                             i->first.c_str() );
                    menu->add( buf, 0,
                               (Fl_Callback*)toggle_modify_attribute_cb,
                               this );
                }

                menu->add( _("Remove Attribute"), 0,
                          (Fl_Callback*)remove_attribute_cb,
                          this);
            }
        }

        menu->menu_end();
        menu->popup();
        menu->clear();
        return 1;
    }
    else if ( event == FL_KEYBOARD )
    {
        int ok = view()->handle( event );
        if (ok) return ok;
    }

    // for ( int i = 0; i < children(); ++i )
    // {
    //     Fl_Widget* e = child(i);
    //     std::cerr << e << " label="
    //               << ( e->label() ? e->label() : "none" ) << std::endl;
    // }

    return Fl_Group::handle( event );
}


struct aspectName_t
{
    double     ratio;
    const char* name;
};

static const aspectName_t kAspectRatioNames[] =
{
    { 640.0/480.0, _("Video") },
    { 680.0/550.0, _("PAL Video") },
    { 720.0/576.0, _("PAL Video") },
    { 768.0/576.0, _("PAL Video") },
    { 720.0/486.0, _("NTSC Video") },
    { 720.0/540.0, _("NTSC Video") },
    { 1.5,  _("NTSC Video") },
    { 1.37, _("35mm Academy") },
    { 1.56, _("Widescreen (HDTV + STV)") },
    { 1.66, _("35mm European Widescreen") },
    { 1.75, _("Early 35mm") },
    { 1.777778, _("HDTV / Widescreen 16:9") },
    { 1.85, _("35mm Flat") },
    { 2.2,  _("70mm") },
    { 2.35, _("35mm Anamorphic") },
    { 2.39, _("35mm Panavision") },
    { 2.55, _("Cinemascope") },
    { 2.76, _("MGM Camera 65") },
};

void ImageInformation::enum_cb( mrv::PopupMenu* m, ImageInformation* v )
{
    m->label( m->child( m->value() )->label() );
}

static void timecode_cb( Fl_Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    CMedia::Attributes& attrs = img->attributes();
    CMedia::Attributes::iterator i = attrs.begin();
    CMedia::Attributes::iterator e = attrs.end();
    for ( ; i != e; ++i )
    {
        if ( i->first == N_("timecode") ||
             i->first == N_("Video timecode") ||
             i->first == N_("Timecode") )
        {
            const Imf::TimeCode& t = CMedia::str2timecode( w->value() );
            img->process_timecode( t );
            Imf::TimeCodeAttribute attr( t );
            i->second = attr.copy();
        }
    }

    img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
    ViewerUI* ui = info->main();
    ui->uiView->redraw();
}

// Update int slider from int input
static void update_int_slider( Fl_Int_Input* w )
{
    Fl_Group* g = w->parent();
    Fl_Slider* s = (Fl_Slider*)g->child(1);
    s->value( atoi( w->value() ) );
}

// Update float slider from float input
static void update_float_slider( Fl_Float_Input* w )
{
    Fl_Group* g = w->parent();
    Fl_Slider* s = (Fl_Slider*)g->child(1);
    s->value( atof( w->value() ) );
}

void ImageInformation::float_slider_cb( Fl_Slider* s, void* data )
{
    Fl_Float_Input* n = (Fl_Float_Input*) data;
    char buf[64];
    sprintf( buf, "%g", s->value() );
    n->value( buf );
    n->do_callback();
}

void ImageInformation::int_slider_cb( Fl_Slider* s, void* data )
{
    Fl_Int_Input* n = (Fl_Int_Input*) data;
    char buf[64];
    sprintf( buf, "%g", s->value() );
    n->value( buf );
    n->do_callback();
}

static bool modify_string( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    Imf::StringAttribute attr( w->value() );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2i( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    int x, y;
    int num = sscanf( w->value(), "%d %d", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two integers for vector ") << i->first );
        return false;
    }

    Imath::V2i val( x, y );
    Imf::V2iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2f( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    float x, y;
    int num = sscanf( w->value(), "%g %g", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two floats for vector ") << i->first );
        return false;
    }

    Imath::V2f val( x, y );
    Imf::V2fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2d( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    double x, y;
    int num = sscanf( w->value(), "%lg %lg", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two doubles for vector ") << i->first );
        return false;
    }

    Imath::V2d val( x, y );
    Imf::V2dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3i( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    int x, y, z;
    int num = sscanf( w->value(), "%d %d %d", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three integers for vector ") << i->first );
        return false;
    }

    Imath::V3i val( x, y, z );
    Imf::V3iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3f( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    float x, y, z;
    int num = sscanf( w->value(), "%g %g %g", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three floats for vector ") << i->first );
        return false;
    }

    Imath::V3f val( x, y, z );
    Imf::V3fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3d( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    double x, y, z;
    int num = sscanf( w->value(), "%lg %lg %lg", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three doubles for vector ") << i->first );
        return false;
    }

    Imath::V3d val( x, y, z );
    Imf::V3dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_chromaticities( Fl_Input* w,
                                   CMedia::Attributes::iterator& i)
{
    float rx, ry, gx, gy, bx, by, wx, wy;
    int num = sscanf( w->value(), "%g %g  %g %g  %g %g  %g %g",
                      &rx, &ry, &gx, &gy, &bx, &by, &wx, &wy );
    if ( num != 8 ) {
        mrvALERT( _("Could not find eight floats for chromaticities ")
                  << i->first );
        return false;
    }

    Imf::Chromaticities val( Imath::V2f( rx, ry ),
                             Imath::V2f( gx, gy ),
                             Imath::V2f( bx, by ),
                             Imath::V2f( wx, wy ) );
    Imf::ChromaticitiesAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33f( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    float m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->value(),
                      "%g %g %g  %g %g %g  %g %g %g",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) {
        mrvALERT( _("Could not find nine floats for matrix 3x3 ")
                  << i->first );
        return false;
    }

    Imath::M33f val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33d( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    double m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->value(),
                      "%lg %lg %lg  %lg %lg %lg  %lg %lg %lg",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) {
        mrvALERT( _("Could not find nine doubles for matrix 3x3 ")
                  << i->first );
        return false;
    }

    Imath::M33d val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44f( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    float m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->value(),
                      "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                      &m00, &m01, &m02, &m03,
                      &m10, &m11, &m12, &m13,
                      &m20, &m21, &m22, &m23,
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) {
        mrvALERT( _("Could not find sixteen floats for matrix 4x4 ")
                  << i->first );
        return false;
    }

    Imath::M44f val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44d( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    double m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->value(),
                      "%lg %lg %lg %lg  %lg %lg %lg %lg  "
                      "%lg %lg %lg %lg  %lg %lg %lg %lg",
                      &m00, &m01, &m02, &m03,
                      &m10, &m11, &m12, &m13,
                      &m20, &m21, &m22, &m23,
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) {
        mrvALERT( _("Could not find sixteen doubles for matrix 4x4 ")
                  << i->first );
        return false;
    }

    Imath::M44d val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}


static bool modify_box2i( Fl_Input* widget, CMedia::Attributes::iterator& i)
{
    int x, y, w, h;
    int num = sscanf( widget->value(),
                      "%d %d  %d %d", &x, &y, &w, &h );
    if ( num != 4 ) {
        mrvALERT( _("Could not find four integers for box ")
                  << i->first );
        return false;
    }

    Imath::Box2i val( Imath::V2i( x, y ), Imath::V2i( w, h ) );
    Imf::Box2iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}
static bool modify_box2f( Fl_Input* widget, CMedia::Attributes::iterator& i)
{
    float x, y, w, h;
    int num = sscanf( widget->value(),
                      "%g %g  %g %g", &x, &y, &w, &h );
    if ( num != 4 ) {
        mrvALERT( _("Could not find four floats for box ")
                  << i->first );
        return false;
    }

    Imath::Box2f val( Imath::V2f( x, y ), Imath::V2f( w, h ) );
    Imf::Box2fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_rational( Fl_Input* widget,
                             CMedia::Attributes::iterator& i)
{
    int n, d;
    int num = sscanf( widget->value(),
                      "%d / %d", &n, &d );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two integers for rational ")
                  << i->first );
        return false;
    }

    Imf::Rational val( n, d );
    Imf::RationalAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_value( Fl_Input* w, CMedia::Attributes::iterator& i)
{
    if ( dynamic_cast< Imf::StringAttribute* >( i->second ) != NULL )
        return modify_string( w, i );
    else if ( dynamic_cast< Imf::M44dAttribute* >( i->second ) != NULL )
        return modify_m44d( w, i );
    else if ( dynamic_cast< Imf::M44fAttribute* >( i->second ) != NULL )
        return modify_m44f( w, i );
    else if ( dynamic_cast< Imf::M33dAttribute* >( i->second ) != NULL )
        return modify_m33d( w, i );
    else if ( dynamic_cast< Imf::M33fAttribute* >( i->second ) != NULL )
        return modify_m33f( w, i );
    else if ( dynamic_cast< Imf::ChromaticitiesAttribute* >( i->second ) )
        return modify_chromaticities( w, i );
    else if ( dynamic_cast< Imf::Box2iAttribute* >( i->second ) != NULL )
        return modify_box2i( w, i );
    else if ( dynamic_cast< Imf::Box2fAttribute* >( i->second ) != NULL )
        return modify_box2f( w, i );
    else if ( dynamic_cast< Imf::V3dAttribute* >( i->second ) != NULL )
        return modify_v3d( w, i );
    else if ( dynamic_cast< Imf::V3fAttribute* >( i->second ) != NULL )
        return modify_v3f( w, i );
    else if ( dynamic_cast< Imf::V3iAttribute* >( i->second ) != NULL )
        return modify_v3i( w, i );
    else if ( dynamic_cast< Imf::V2dAttribute* >( i->second ) != NULL )
        return modify_v2d( w, i );
    else if ( dynamic_cast< Imf::V2fAttribute* >( i->second ) != NULL )
        return modify_v2f( w, i );
    else if ( dynamic_cast< Imf::V2iAttribute* >( i->second ) != NULL )
        return modify_v2i( w, i );
    else if ( dynamic_cast< Imf::RationalAttribute* >( i->second ) != NULL )
        return modify_rational( w, i );
    else
        LOG_ERROR( _("Unknown attribute to convert from string") );
    return false;
}

static bool modify_keycode( Fl_Int_Input* w,
                            CMedia::Attributes::iterator& i,
                            const std::string& subattr )
{
    Imf::KeyCodeAttribute* attr(
        dynamic_cast<Imf::KeyCodeAttribute*>(i->second ) );
    if ( !attr ) return false;

    Imf::KeyCode t = attr->value();
    try
    {
        if ( subattr == "filmMfcCode" )
        {
            t.setFilmMfcCode( atoi( w->value() ) );
        }
        else if ( subattr == "filmType" )
        {
            t.setFilmType( atoi( w->value() ) );
        }
        else if ( subattr == "prefix" )
        {
            t.setPrefix( atoi( w->value() ) );
        }
        else if ( subattr == "count" )
        {
            t.setCount( atoi( w->value() ) );
        }
        else if ( subattr == "perfOffset" )
        {
            t.setPerfOffset( atoi( w->value() ) );
        }
        else if ( subattr == "perfsPerFrame" )
        {
            t.setPerfsPerFrame( atoi( w->value() ) );
        }
        else if ( subattr == "perfsPerCount" )
        {
            t.setPerfsPerCount( atoi( w->value() ) );
        }
        else
        {
            mrvALERT( _("Unknown KeyCode subattr") );
            return false;
        }
        Imf::KeyCodeAttribute nattr( t );
        delete i->second;
        i->second = nattr.copy();
        update_int_slider( w );
    } catch ( const std::exception& e )
    {
        mrvALERT( e.what() );
    }

    return true;
}

static bool modify_int( Fl_Int_Input* w, CMedia::Attributes::iterator& i)
{
    Imf::IntAttribute attr( atoi( w->value() ) );
    delete i->second;
    i->second = attr.copy();
    update_int_slider( w );
    return true;
}

static bool modify_float( Fl_Float_Input* w, CMedia::Attributes::iterator& i)
{
    Imf::FloatAttribute attr( (float) atof( w->value() ) );
    delete i->second;
    i->second = attr.copy();
    update_float_slider( w );
    return true;
}

static void change_float_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    Fl_Group* g = (Fl_Group*)w->parent()->parent();
    Fl_Widget* widget = g->child(0);
    if ( !widget->label() ) return;

    std::string key = widget->label();
    CMedia::Attributes& attributes = img->attributes();
    CMedia::Attributes::iterator i = attributes.find( key );
    if ( i != attributes.end() )
    {
        modify_float( w, i );
        return;
    }
}

static void change_string_cb( Fl_Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) {
        LOG_ERROR( "Image is invalid" );
        return;
    }


    mrv::Table* t = dynamic_cast< mrv::Table* >( w->parent()->parent() );
    if ( t == NULL )
    {
        return;
    }

    Fl_Widget* box;
    bool found = false;
    for ( int r = 0; r < t->rows(); ++r )
    {
        int i = 2 * r;
        if ( i >= t->children() ) break;
        Fl_Group* sg = dynamic_cast< Fl_Group* >( t->child(i) );
        if ( !sg ) {
            break;
        }
        box = (Fl_Widget*)sg->child(0);
        Fl_Widget* widget = t->child(i+1);
        if ( widget == w ) {
            found = true;
            break;
        }
    }

    if (!found )
    {
        LOG_ERROR( _("Could not find attribute \"") << w->label() << "\"");
        return;
    }


    if ( !box->label() ) {
        LOG_ERROR( _("Widget has no label") );
        return;
    }

    std::string key = box->label();
    CMedia::Attributes& attributes = img->attributes();
    CMedia::Attributes::iterator i = attributes.find( key );
    if ( i != attributes.end() )
    {
        bool ok = modify_value( w, i );
        if (!ok) {
            info->filled = false;
            info->refresh();
            toggle_modify_attribute( key, info );
        }
        return;
    }
}

static void change_int_cb( Fl_Int_Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    Fl_Group* g = (Fl_Group*)w->parent()->parent();
    Fl_Widget* widget = g->child(0);
    if ( !widget->label() ) return;

    std::string key = widget->label();
    CMedia::Attributes& attributes = img->attributes();
    CMedia::Attributes::iterator i = attributes.find( key );
    if ( i != attributes.end() )
    {
        modify_int( w, i );
        return;
    }
}

static void change_keycode_cb( Fl_Int_Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    Fl_Group* g = (Fl_Group*)w->parent()->parent();
    Fl_Widget* widget = g->child(0);
    if ( !widget->label() ) return;

    std::string key = widget->label();
    std::string subattr;
    size_t p = key.rfind( '.' );
    if ( p != std::string::npos )
    {
        subattr = key.substr( p+1, key.size() );
        key  = key.substr( 0, p );
    }
    CMedia::Attributes& attributes = img->attributes();
    CMedia::Attributes::iterator i = attributes.find( key );
    if ( i != attributes.end() )
    {
        modify_keycode( w, i, subattr );
        return;
    }
}

static void change_scale_cb( Fl_Button* w, ImageInformation* info )
{
    w->value(1);

    R3dImage* img = dynamic_cast<R3dImage*>( info->get_image() );
    if ( !img ) return;

    int v = 4;

    std::string l = w->label();
    if ( l == "1:1" ) v = 0;
    else if ( l == "1:2" ) v = 1;
    else if ( l == "1:4" ) v = 2;
    else if ( l == "1:8" ) v = 3;
    else if ( l == "1:16" ) v = 4;

    img->scale( v );
    img->refetch( img->frame() );
    img->image_damage( mrv::CMedia::kDamageAll );
    info->refresh();

    ImageView* view = info->main()->uiView;

    view->fit_image();

    view->redraw();
}

static void change_scale_braw_cb( Fl_Button* w, ImageInformation* info )
{
    w->value(1);

    brawImage* img = dynamic_cast<brawImage*>( info->get_image() );
    if ( !img ) return;

    int v = 3;

    std::string l = w->label();
    if ( l == "1:1" ) v = 0;
    else if ( l == "1:2" ) v = 1;
    else if ( l == "1:4" ) v = 2;
    else if ( l == "1:8" ) v = 3;

    img->scale( v );
    img->refetch( img->frame() );
    img->image_damage( mrv::CMedia::kDamageAll );
    info->refresh();

    ImageView* view = info->main()->uiView;

    view->fit_image();

    view->redraw();
}

static void change_iso_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast<R3dImage*>( info->get_image() );
    if ( !img ) return;

    int v = w->value();
    char buf[256];
    sprintf( buf, "%zu", R3DSDK::ImageProcessingLimits::ISOList[v] );
    w->copy_label( buf );
    w->redraw();
    img->iso_index( v );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_ipp2_mode_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast<R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->pipeline( (R3DSDK::ImagePipeline) w->value() );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_color_space_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast<R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->color_space( w->value() );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_gamma_curve_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast<R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->gamma_curve( w->value() );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_colorspace( mrv::PopupMenu* w, ImageInformation* info )
{
    aviImage* img = dynamic_cast<aviImage*>( info->get_image() );
    if ( !img ) return;

    int v = w->value();
    w->label( _(kColorSpaces[v]) );
    img->colorspace_index( (int)v );
    img->image_damage( mrv::CMedia::kDamageAll );
}

static void change_mipmap_cb( Fl_Int_Input* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelX( atoi( w->value() ) );
        img->levelY( atoi( w->value() ) );
        img->clear_cache();
        update_int_slider( w );
        image_type_ptr canvas;
        bool ok = img->fetch( canvas, img->frame() );
        if (ok)
        {
            img->hires( canvas );
            img->cache( canvas );
            img->refresh();
            view->fit_image();
            view->redraw();
        }
        else
        {
            LOG_ERROR( _("Fetch failed for mipmap " ) << img->levelX() );
        }
        return;
    }
    else
    {
        oiioImage* img = dynamic_cast<oiioImage*>( info->get_image() );
        if ( img )
        {
            mrv::ImageView* view = info->main()->uiView;
            img->clear_cache();
            img->level( atoi( w->value() ) );
            update_int_slider( w );
            image_type_ptr canvas;
            bool ok = img->fetch( canvas, img->frame() );
            if (ok)
            {
                img->hires( canvas );
                img->cache( canvas );
                img->refresh();
                view->fit_image();
                view->redraw();
            }
            else
            {
                LOG_ERROR( _("Fetch failed for mipmap " ) << img->level() );
            }
            return;
        }
    }
}

static void change_x_ripmap_cb( Fl_Int_Input* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelX( atoi( w->value() ) );
        update_int_slider( w );
        img->clear_cache();
        image_type_ptr canvas;
        bool ok = img->fetch( canvas, view->frame() );
        if (ok)
        {
            img->hires( canvas );
            img->cache( canvas );
            img->refresh();
            view->fit_image();
            view->redraw();
        }
        else
        {
            LOG_ERROR( _("Fetch failed for ripmap X " ) << img->levelX() );
        }
        return;
    }
}

static void change_y_ripmap_cb( Fl_Int_Input* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelY( atoi( w->value() ) );
        img->clear_cache();
        update_int_slider( w );
        image_type_ptr canvas;
        bool ok = img->fetch( canvas, view->frame() );
        if (ok)
        {
            img->hires( canvas );
            img->cache( canvas );
            img->refresh();
            view->fit_image();
            view->redraw();
        }
        else
        {
            LOG_ERROR( _("Fetch failed for ripmap Y " ) << img->levelY() );
        }
    }
}

static void change_first_frame_cb( Fl_Int_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        int64_t v = atoi( w->value() );
        // if ( v < img->start_frame() )
        //     v = img->start_frame();
        // if ( v > img->last_frame() )
        //     v = img->last_frame();
        char buf[64];
        sprintf( buf, "%" PRId64, v );
        w->value( buf );

        img->first_frame( v );
        update_int_slider( w );
        mrv::ImageView* view = info->main()->uiView;
        int64_t first, last;
        view->browser()->adjust_timeline( first, last );
        view->browser()->set_timeline( first, last );
        view->redraw();
    }
}

static void eye_separation_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {

        img->eye_separation( (float) atof( w->value() ) );
        update_float_slider( w );

        info->main()->uiView->redraw();
    }
}

static void change_fps_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        float f = (float) atof( w->value() );
        img->fps( f );
        update_float_slider( w );
    }
}

static void change_last_frame_cb( Fl_Int_Input* w,
                                  ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        int64_t v = atoi( w->value() );
        // if ( v < img->first_frame() )
        //     v = img->first_frame();
        // if ( v > img->end_frame() )
        //     v = img->end_frame();
        char buf[64];
        sprintf( buf, "%" PRId64, v );
        w->value( buf );

        img->last_frame( v );
        mrv::ImageView* view = info->main()->uiView;
        int64_t first, last;
        view->browser()->adjust_timeline( first, last );
        view->browser()->set_timeline( first, last );
        view->redraw();
        update_int_slider( w );
    }
}

static void change_scale_x_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();

    img->scale_x( atof( w->value() ) );
    update_float_slider( w );

    info->main()->uiView->redraw();
}

static void change_scale_y_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();

    img->scale_y( atof( w->value() ) );
    update_float_slider( w );

    info->main()->uiView->redraw();
}

static void change_x_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();

    img->x( atof( w->value() ) );
    update_float_slider( w );

    info->main()->uiView->redraw();
}

static void change_y_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();

    img->y( atof( w->value() ) );
    update_float_slider( w );

    info->main()->uiView->redraw();
}

static void change_pixel_ratio_cb( Fl_Float_Input* w,
                                   ImageInformation* info )
{
    CMedia* img = info->get_image();

    img->pixel_ratio( atof( w->value() ) );
    update_float_slider( w );

    info->main()->uiView->redraw();
}

static void r3d_camera_cb( Fl_Button* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->load_camera_settings();
    img->image_damage( mrv::CMedia::kDamageAll );
    info->refresh();
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_sidecar_cb( Fl_Button* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage* >( info->get_image() );
    if ( !img ) return;

    img->load_rmd_sidecar();
    img->image_damage( mrv::CMedia::kDamageAll );
    info->refresh();
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}


static void change_gain_blue_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->GainBlue( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_gain_green_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->GainGreen( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_gain_red_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->GainRed( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_exposure_compensation_cb( Fl_Float_Input* w,
                                             ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->ExposureCompensation( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_exposure_adjust_cb( Fl_Float_Input* w,
                                       ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->ExposureAdjust( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_brightness_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Brightness( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_contrast_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Contrast( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_flut_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Flut( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_kelvin_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Kelvin( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_tint_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Tint( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_saturation_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Saturation( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_shadow_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Shadow( (float) atof( w->value() ) );
    update_float_slider( w );

    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}


static void change_blend_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    unsigned idx = w->value();
    img->hdr_mode( (R3DSDK::HdrMode) idx );
    w->copy_label( w->child( idx )->label() );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_trackno_cb( Fl_Int_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->trackNo( atoi( w->value() ) );
    img->image_damage( mrv::CMedia::kDamageAll );
    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_bias_cb( Fl_Float_Input* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    img->Bias( (float) atof( w->value() ) );
    update_float_slider( w );


    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_sharpness_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;


    int v = w->value();
    w->copy_label( w->child(v)->label() );
    img->Sharpness( (R3DSDK::ImageOLPFCompensation) v );
    img->refetch();

    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_denoise_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    int v = w->value();
    w->copy_label( w->child(v)->label() );
    img->Denoise( (R3DSDK::ImageDenoise) v );
    img->refetch();

    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}

static void change_detail_cb( mrv::PopupMenu* w, ImageInformation* info )
{
    R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
    if ( !img ) return;

    int v = w->value();
    w->copy_label( w->child(v)->label() );
    img->Detail( (R3DSDK::ImageDetail) v );
    img->refetch();

    mrv::ImageView* view = info->main()->uiView;
    view->redraw();
}



static void change_gamma_cb( Fl_Float_Input* w, ImageInformation* info )
{
    CMedia* img = info->get_image();

    float g = (float) atof( w->value() );
    img->gamma( g );
    update_float_slider( w );


    mrv::ImageView* view = info->main()->uiView;
    view->gamma( g );
    view->redraw();
}

double ImageInformation::to_memory( long double value,
                                    const char*& extension )
{
    if ( value >= 1099511627776 )
    {
        value /= 1099511627776;
        extension = N_("Tb");
    }
    else if ( value >= 1073741824 )
    {
        value /= 1073741824;
        extension = N_("Gb");
    }
    else if ( value >= 1048576 )
    {
        value /= 1048576;
        extension = N_("Mb");
    }
    else if ( value >= 1024 )
    {
        value /= 1024;
        extension = N_("Kb");
    }
    else
    {
        extension = N_("bytes");
    }
    return value;
}

void ImageInformation::set_image( CMedia* i )
{
    DBG3;
    filled = false;
    img = i;
    refresh();
}

void ImageInformation::clear_callback_data()
{

    LMTData::iterator i = widget_data.begin();
    LMTData::iterator e = widget_data.end();

    for ( ; i != e; ++i)
    {
        delete *i;
    }

    widget_data.clear();
}

void ImageInformation::hide_tabs()
{
    DBG2;
    tooltip( _("Load an image or movie file") );
    m_all->hide();

    m_image->hide();
    m_video->hide();
    m_audio->hide();
    m_subtitle->hide();
    m_attributes->hide();

    m_curr = NULL;
    DBG3;
}

void ImageInformation::process_attributes( mrv::CMedia::Attributes::const_iterator& i )
{
    {
        Imf::TimeCodeAttribute* attr =
            dynamic_cast< Imf::TimeCodeAttribute*>( i->second );
        if (attr)
        {
            ++group;
            mrv::Timecode::Display d = mrv::Timecode::kTimecodeNonDrop;
            if ( attr->value().dropFrame() )
                d = mrv::Timecode::kTimecodeDropFrame;
            char buf[64];
            int n = mrv::Timecode::format( buf, d, img->frame(),
                                           img->timecode(),
                                           img->play_fps(), true );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*)timecode_cb );
            return;
        }
    }
    {
        Imf::RationalAttribute* attr =
            dynamic_cast< Imf::RationalAttribute* >( i->second );
        if ( attr )
        {
            const Imf::Rational& r = attr->value();
            char buf[64];
            sprintf( buf, "%d / %d", r.n, r.d );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*)change_string_cb );
            return;
        }
    }
    {
        Imf::DoubleAttribute* attr =
            dynamic_cast< Imf::DoubleAttribute* >( i->second );
        if ( attr )
        {
            add_float( i->first.c_str(), NULL,
                       (float)attr->value(), true, false,
                       (Fl_Callback*)change_float_cb );
            return;
        }
    }
    {
        Imf::FloatAttribute* attr =
            dynamic_cast< Imf::FloatAttribute* >( i->second );
        if ( attr )
        {
            add_float( i->first.c_str(), NULL,
                       attr->value(), true, false,
                       (Fl_Callback*)change_float_cb );
            return;
        }
    }
    {
        Imf::IntAttribute* attr =
            dynamic_cast< Imf::IntAttribute* >( i->second );
        if ( attr )
        {
            add_int( i->first.c_str(), NULL,
                     attr->value(), true, false,
                     (Fl_Callback*)change_int_cb, 0, 10,
                     FL_WHEN_CHANGED );
            return;
        }
    }
    {
        Imf::V2iAttribute* attr =
            dynamic_cast< Imf::V2iAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2i& v = attr->value();
            sprintf( buf, "%d %d", v.x, v.y );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V2dAttribute* attr =
            dynamic_cast< Imf::V2dAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2d& v = attr->value();
            sprintf( buf, "%g %g", v.x, v.y );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V2fAttribute* attr =
            dynamic_cast< Imf::V2fAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2f& v = attr->value();
            sprintf( buf, "%g %g", v.x, v.y );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3iAttribute* attr =
            dynamic_cast< Imf::V3iAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3i& v = attr->value();
            sprintf( buf, "%d %d %d", v.x, v.y, v.z );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3dAttribute* attr =
            dynamic_cast< Imf::V3dAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3d& v = attr->value();
            sprintf( buf, "%g %g %g", v.x, v.y, v.z );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3fAttribute* attr =
            dynamic_cast< Imf::V3fAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3f& v = attr->value();
            sprintf( buf, "%g %g %g", v.x, v.y, v.z );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::Box2iAttribute* attr =
            dynamic_cast< Imf::Box2iAttribute* >( i->second );
        if ( attr )
        {
            char buf[128];
            const Imath::Box2i& v = attr->value();
            sprintf( buf, "%d %d %d %d", v.min.x, v.min.y, v.max.x, v.max.x );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::Box2fAttribute* attr =
            dynamic_cast< Imf::Box2fAttribute* >( i->second );
        if ( attr )
        {
            char buf[128];
            const Imath::Box2f& v = attr->value();
            sprintf( buf, "%g %g %g %g", v.min.x, v.min.y, v.max.x, v.max.x );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::ChromaticitiesAttribute* attr =
            dynamic_cast< Imf::ChromaticitiesAttribute* >( i->second );
        if ( attr )
        {
            char buf[128];
            const Imf::Chromaticities& v = attr->value();
            sprintf( buf, "%g %g  %g %g  %g %g  %g %g",
                     v.red.x, v.red.y, v.green.x, v.green.y,
                     v.blue.x, v.blue.y, v.white.x, v.white.y );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M33dAttribute* attr =
            dynamic_cast< Imf::M33dAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M33d& v = attr->value();
            sprintf( buf, "%g %g %g  %g %g %g  %g %g %g",
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M33fAttribute* attr =
            dynamic_cast< Imf::M33fAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M33f& v = attr->value();
            sprintf( buf, "%g %g %g  %g %g %g  %g %g %g",
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M44dAttribute* attr =
            dynamic_cast< Imf::M44dAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M44d& v = attr->value();
            sprintf( buf, "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M44fAttribute* attr =
            dynamic_cast< Imf::M44fAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M44f& v = attr->value();
            sprintf( buf, "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            add_text( i->first.c_str(), NULL,
                      buf, true, false, (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::StringVectorAttribute* attr =
            dynamic_cast< Imf::StringVectorAttribute* >( i->second );
        if ( attr )
        {
            Imf::StringVector::const_iterator it = attr->value().begin();
            Imf::StringVector::const_iterator ib = it;
            Imf::StringVector::const_iterator et = attr->value().end();
            char buf[256];
            for ( ; it != et; ++it )
            {
                sprintf( buf, "%s #%td", i->first.c_str(), (it - ib)+1 );
                add_text( buf, NULL,
                          *it, false, false,
                          (Fl_Callback*) change_string_cb );
            }
            return;
        }
    }
    {
        Imf::StringAttribute* attr =
            dynamic_cast< Imf::StringAttribute* >( i->second );
        if ( attr )
        {
            add_text( i->first.c_str(), NULL,
                      attr->value().c_str(), true, false,
                      (Fl_Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::OpaqueAttribute* attr =
            dynamic_cast< Imf::OpaqueAttribute* >( i->second );
        if ( attr )
        {
            add_text( i->first.c_str(), NULL,
                      "opaque", false, false );
            return;
        }
    }
    {
        Imf::KeyCodeAttribute* attr =
            dynamic_cast< Imf::KeyCodeAttribute* >( i->second );
        if ( attr )
        {
            const Imf::KeyCode& k = attr->value();
            std::string key = i->first;
            add_int( (key + ".filmMfcCode").c_str(), NULL,
                     k.filmMfcCode(), true, false,
                     (Fl_Callback*) change_keycode_cb, 0, 99 );
            add_int( (key + ".filmType").c_str(), NULL,
                     k.filmType(), true, false,
                     (Fl_Callback*) change_keycode_cb, 0, 99 );
            add_int( (key + ".prefix").c_str(), NULL,
                     k.prefix(), true, false,
                     (Fl_Callback*) change_keycode_cb, 0, 999999 );
            add_int( (key + ".count").c_str(), NULL,
                     k.count(), true, false,
                     (Fl_Callback*) change_keycode_cb, 0, 9999 );
            add_int( (key + ".perfOffset").c_str(), NULL,
                     k.perfOffset(), true, false,
                     (Fl_Callback*) change_keycode_cb, 0, 119 );
            add_int( (key + ".perfsPerFrame").c_str(), NULL,
                     k.perfsPerFrame(), true, false,
                     (Fl_Callback*) change_keycode_cb, 1, 15 );
            add_int( (key + ".perfsPerCount").c_str(), NULL,
                     k.perfsPerCount(), true, false,
                     (Fl_Callback*) change_keycode_cb, 20, 120 );
            return;
        }
    }
    {
        Imf::CompressionAttribute* attr =
            dynamic_cast< Imf::CompressionAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    {
        Imf::DeepImageStateAttribute* attr =
            dynamic_cast< Imf::DeepImageStateAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    {
        Imf::EnvmapAttribute* attr =
            dynamic_cast< Imf::EnvmapAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    {
        Imf::LineOrderAttribute* attr =
            dynamic_cast< Imf::LineOrderAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    {
        Imf::PreviewImageAttribute* attr =
            dynamic_cast< Imf::PreviewImageAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    {
        Imf::TileDescriptionAttribute* attr =
            dynamic_cast< Imf::TileDescriptionAttribute* >( i->second );
        if ( attr ) return;  // Nothing to do here
    }
    mrvALERT( _("Unknown attribute type for '") << i->first << _("' type '")
              << i->second->typeName() << "'");
}

void ImageInformation::fill_data()
{


    char buf[1024];
    m_curr = add_browser(m_image);


    add_text( _("Directory"), _("Directory where clip resides"), img->directory() );


    add_text( _("Filename"), _("Filename of the clip"), img->name().c_str() );

    ++group;





    int num_video_streams = (int) img->number_of_video_streams();
    int num_audio_streams = (int) img->number_of_audio_streams();
    int num_subtitle_streams = (int) img->number_of_subtitle_streams();

    if ( img->has_video() || img->has_audio() )
    {
        add_int( _("Video Streams"), _("Number of video streams in file"),
                 num_video_streams );
        add_int( _("Audio Streams"), _("Number of audio streams in file"),
                 num_audio_streams );
        add_int( _("Subtitle Streams"),
                 _("Number of subtitle streams in file"),
                 num_subtitle_streams );
    }
    else
    {
        add_bool( _("Sequence"), _("Clip is a sequence of frames"), img->is_sequence() );
    }

    //if ( img->first_frame() != img->last_frame() )
    {
        add_int( _("First Frame"), _("First frame of clip - User selected"),
                 (int)img->first_frame(), true, true,
                 (Fl_Callback*)change_first_frame_cb, 1, 50 );
        add_int( _("Last Frame"), _("Last frame of clip - User selected"),
                 (int)img->last_frame(), true, true,
                 (Fl_Callback*)change_last_frame_cb, 2, 55 );
    }



    add_int64( _("Frame Start"), _("Frame start of clip in file"), img->start_frame() );
    add_int64( _("Frame End"), _("Frame end of clip in file"), img->end_frame() );


    add_float( _("FPS"), _("Frames Per Second"), (float) img->fps(), true, true,
               (Fl_Callback*)change_fps_cb, 1.0f, 100.0f );



    ++group;

    add_int( _("Width"), _("Width of clip"), (unsigned)img->width(), false );
    add_int( _("Height"), _("Height of clip"), (unsigned)img->height(), false );

    R3dImage* r3d = dynamic_cast< R3dImage* >( img );
    if ( r3d )
    {
        add_int( _("Real Width"), _("Real width of clip"),
                 (unsigned)r3d->real_width(), false );
        add_int( _("Real Height"), _("Real height of clip"),
                 (unsigned)r3d->real_height(), false );
    }

    double aspect_ratio = 0;
    const mrv::Recti& daw = img->display_window();
    mrv::Recti dpw = img->display_window();
    if ( daw.h() >= dpw.h() )
    {
        aspect_ratio = ( (double) daw.w() / (double) daw.h() );
    }
    else if ( dpw.h() )
    {
        aspect_ratio = ( (double) dpw.w() / (double) dpw.h() );
    }
    else if ( img->height() > 0 )
        aspect_ratio = ( img->width() / (double) img->height() );


    const char* name = _("Unknown");
    int num = sizeof( kAspectRatioNames ) / sizeof(aspectName_t);
    for ( int i = 0; i < num; ++i )
    {
        static const double fuzz = 0.001;
        if ( mrv::is_equal( aspect_ratio, kAspectRatioNames[i].ratio, fuzz ) )          {
            name = _( kAspectRatioNames[i].name );
            break;
        }
    }



    sprintf( buf, N_("%g (%s)"), aspect_ratio, name );
    add_text( _("Aspect Ratio"), _("Aspect ratio of clip"), buf );
    add_float( _("Pixel Ratio"), _("Pixel ratio of clip"),
               float(img->pixel_ratio()), true, true,
               (Fl_Callback*)change_pixel_ratio_cb, 0.01f, 4.0f,
               FL_WHEN_CHANGED );

    add_float( _("X Position"), _("Image X Position in Canvas"),
               (float)img->x(), true, true,
               (Fl_Callback*)change_x_cb, 0.0f, 720.0f,
               FL_WHEN_CHANGED );

    add_float( _("Y Position"), _("Image Y Position in Canvas"),
               (float)img->y(), true, true,
               (Fl_Callback*)change_y_cb, 0.0f, 720.0f,
               FL_WHEN_CHANGED );

    add_float( _("X Scale"), _("Image X Scale in Canvas"),
               (float)img->scale_x(), true, true,
               (Fl_Callback*)change_scale_x_cb, 0.00001f, 2.0f,
               FL_WHEN_CHANGED );

    add_float( _("Y Scale"), _("Image Y Scale in Canvas"),
               (float)img->scale_y(), true, true,
               (Fl_Callback*)change_scale_y_cb, 0.00001f, 2.0f,
               FL_WHEN_CHANGED );

    ++group;

    const mrv::Recti& window = img->data_window();
    if ( dpw != window && dpw.w() != 0 )
    {
        add_rect( _("Data Window"), _("Data Window of Clip"), window );

        if ( window.y() == 0 )
        {
          dpw.y( (int)img->height() - dpw.b() );
        }
        else
        {
          int top = window.y() - ( dpw.h() - window.h() - window.y() );
          dpw.y( top );
        }
    }


    add_rect( _("Display Window"), _("Display Window of Clip"), dpw );


    if ( img->is_stereo() )
    {
        mrv::Recti dpw2 = img->display_window2();

        const mrv::Recti& window2 = img->data_window2();
        if ( window != window2 )
        {
            add_rect( _("Data Window 2"), _("Second Data Window of Clip"),
                      window2 );

            if ( window2.y() == 0 )
            {
              dpw2.y( (int)img->height() - dpw2.b() );
            }
            else
            {
              int top = window2.y() -
                        ( dpw2.h() - window2.h() - window2.y() );
              dpw2.y( top );
            }
        }

        if ( dpw != dpw2 )
        {
            add_rect( _("Display Window 2"), _("Second Display Window of Clip"),
                      dpw2 );
        }


        if ( img->right() )
            add_float( _("Eye Separation"), _("Stereo eye separation"),
                       img->eye_separation(), true, true,
                       (Fl_Callback*)eye_separation_cb, -20.0f, 20.0f );
    }


    ++group;

    const char* depth;
    switch( img->depth() )
    {
    case VideoFrame::kByte:
        depth = _("unsigned byte (8-bits per channel)");
        break;
    case VideoFrame::kShort:
        depth = _("unsigned short (16-bits per channel)");
        break;
    case VideoFrame::kInt:
        depth = _("unsigned int (32-bits per channel)");
        break;
    case VideoFrame::kHalf:
        depth = _("half float (16-bits per channel)");
        break;
    case VideoFrame::kFloat:
        depth = _("float (32-bits per channel)");
        break;
    default:
        depth = _("Unknown bit depth");
        break;
    }


    add_text( _("Depth"), _("Bit depth of clip"), depth );
    add_int( _("Image Channels"), _("Number of channels in clip"),
             img->number_of_channels(), false );

    exrImage* exr = dynamic_cast< exrImage* >( img );
    if ( exr )
    {
        int numparts = exr->numparts();
        add_int( _("Number of Parts"), _("Number of Parts"), numparts );
    }

    aviImage* avi = dynamic_cast< aviImage* >( img );
    if ( avi )
    {
        add_enum( _("Color Space"), _("YUV Color Space conversion.  This value is extracted from the movie file.  To lock it to always use the same color space, set the value in Preferences->Video->YUV Conversion.  That value shall take precedence upon loading of the movie file."),
                  avi->colorspace_index(), kColorSpaces,
                  12, true, (Fl_Callback*)change_colorspace );
        add_text( _("Color Range"), _("YUV Color Range"),
                  _(avi->color_range()) );
    }

    DBG3;

    ++group;

    const char* format = img->pixel_format_name();

    add_text( _("Render Pixel Format"), _("Render Pixel Format"), format );




    static const char* kRenderingIntent[] = {
        _("Undefined"),
        _("Saturation"),
        _("Perceptual"),
        _("Absolute"),
        _("Relative"),
    };

    DBG3;


    add_text( _("Rendering Intent"), _("ICC Rendering Intent"),
              kRenderingIntent[ (int) img->rendering_intent() ] );





    add_float( _("Gamma"), _("Display Gamma of Image"), img->gamma(), true,
               true, (Fl_Callback*)change_gamma_cb, 0.01f,	4.0f );


    DBG3;
    if ( img->has_chromaticities() )
    {
        const Imf::Chromaticities& c = img->chromaticities();
        sprintf( buf, _("R: %g %g    G: %g %g    B: %g %g"),
                 c.red.x, c.red.y, c.green.x, c.green.y,
                 c.blue.x, c.blue.y );
        add_text( _("CIExy Chromaticities"), _("CIExy Chromaticities"), buf );
        sprintf( buf, _("W: %g %g"),c.white.x, c.white.y );
        add_text( _("CIExy White Point"), _("CIExy White Point"), buf );
    }

    if ( Preferences::use_ocio )
    {
    DBG3;
        add_ocio_ics( _("Input Color Space"),
                      _("OCIO Input Color Space"),
                      img->ocio_input_color_space().c_str() );
    }
    else
    {

        add_ctl_idt( _("Input Device Transform"),
                     _("(IDT) Input Device Transform"),
                     img->idt_transform() );


        clear_callback_data();

        {


            unsigned count = (unsigned) img->number_of_lmts();
            for ( unsigned i = 0; i <= count; ++i )
            {
                sprintf( buf, _("LMTransform %u"), i+1 );
                add_ctl_lmt( buf, _("(LMT) Look Mod Transform"),
                             img->look_mod_transform(i), i );
            }
        }



        add_ctl( _("Render Transform"), _("(RT) Render Transform"),
                 img->rendering_transform() );


        add_icc( _("ICC Profile"), _("ICC Profile"), img->icc_profile() );
    }

    DBG3;
    ++group;



    add_text( _("Format"), _("Format"), img->format() );

    DBG3;
    if ( !img->has_video() )
    {
        add_text( _("Line Order"), _("Line order in file"),
                  img->line_order() );
    }



    if ( !img->has_video() )
    {
        ++group;

    DBG3;

        add_text( _("Compression"), _("Clip Compression"), img->compression() );


    }

    ++group;



    DBG3;
    const char* space_type = NULL;
    double memory_space = double( to_memory( (long double)img->memory(),
                                  space_type ) );
    sprintf( buf, N_("%.3f %s"), memory_space, space_type );
    add_text( _("Memory"), _("Memory without Compression"), buf );


    DBG3;
    if ( img->disk_space() >= 0 )
    {

        double disk_space = double( to_memory( (long double)img->disk_space(),
                                               space_type ) );

        double pct   = double( 100.0 * ( (long double) img->disk_space() /
                                         (long double) img->memory() ) );


    DBG3;
        sprintf( buf, N_("%.3f %s  (%.2f %% of memory size)"),
                 disk_space, space_type, pct );

        add_text( _("Disk space"), _("Disk space"), buf );



        if ( !img->has_video() )
        {
            double ratio = 100.0 - double(pct);
            sprintf( buf, _("%4.8g %%"), ratio );

            add_text( _("Compression Ratio"), _("Compression Ratio"), buf );
        }

    }


    DBG3;

    ++group;
    add_text( _("Creation Date"), _("Creation Date"), img->creation_date() );


    DBG3;


    DBG3;

    m_image->show();

    DBG3;

    tooltip( NULL );



    const CMedia::Attributes& cattrs = img->clip_attributes();
    const CMedia::Attributes& attrs = img->attributes();
    if ( ! attrs.empty() || !cattrs.empty() )
    {
        m_curr = add_browser( m_attributes );

        brawImage* braw = dynamic_cast< brawImage* >( img );
        if ( braw )
        {
            add_scale( _("Image Scale"), _("Proxy image scale"), braw->scale(),
                       4, (Fl_Callback*)change_scale_braw_cb );
        }

        R3dImage* r3d = dynamic_cast< R3dImage* >( img );

        if ( r3d )
        {
            add_scale( _("Image Scale"), _("Proxy image scale"), r3d->scale(),
                       5, (Fl_Callback*)change_scale_cb );

            stringArray options;
            std::string content;

            char version[10];
            sprintf( version, "%d", r3d->color_version() );
            add_text( _("Color Version"), _("RED Color Version"),
                      version );

            add_button( _("Settings from RMD"),
                        _("Load settings from RMD sidecar or\n"
                          "reset them to camera settings."),
                        (Fl_Callback*)change_sidecar_cb,
                        (Fl_Callback*)r3d_camera_cb );

            if ( r3d->color_version() >= 3 ||
                 r3d->color_space() == "REDWideGamutRGB" )
            {
                options.clear();
                for ( size_t i = 0;
                      i < R3DSDK::ImageProcessingLimits::ImagePipelineModeCount;
                      ++i )
                {
                    options.push_back( R3DSDK::ImageProcessingLimits::ImagePipelineModeLabels[i] );
                }

                content = options[ (unsigned) r3d->pipeline() ];

                add_enum( _("IPP2 Mode"), _("Mode for IPP2 Processing"),
                          content, options, true,
                          (Fl_Callback*)change_ipp2_mode_cb );
            }

            r3d->color_spaces( options );
            add_enum( _("Color Space"), _("RED Color space"),
                      r3d->color_space(), options, true,
                      (Fl_Callback*)change_color_space_cb );

            r3d->gamma_curves( options );
            add_enum( _("Gamma Curve"), _("RED gamma curve"),
                      r3d->gamma_curve(), options, true,
                      (Fl_Callback*)change_gamma_curve_cb );


            group++;
            options.clear();
            char buf[128];
            for ( size_t i = 0;
                  i < R3DSDK::ImageProcessingLimits::ISOCount; ++i )
            {
                sprintf( buf, "%zu",
                         R3DSDK::ImageProcessingLimits::ISOList[i] );
                options.push_back( buf );
            }
            sprintf( buf, "%zu",
                     R3DSDK::ImageProcessingLimits::ISOList[r3d->iso_index()] );
            content = buf;
            add_enum( _("ISO"), _("ISO of film stock."),
                      content, options, true, (Fl_Callback*)change_iso_cb );

            add_float( _("Kelvin"), _("Kelvin temperature."),
                       r3d->Kelvin(), true, true,
                       (Fl_Callback*)change_kelvin_cb,
                       ImageProcessingLimits::KelvinMin,
                       ImageProcessingLimits::KelvinMax,
                       FL_WHEN_RELEASE,
                       mrv::Slider::kLOG );

            add_float( _("Tint"), _("Tinting of clip."),
                       r3d->Tint(), true, true,
                       (Fl_Callback*)change_tint_cb,
                       ImageProcessingLimits::TintMin,
                       ImageProcessingLimits::TintMax );

            if ( r3d->color_version() >= 2 )
            {
                add_float( _("Exposure Adjust"),
                           _("Exposure adjustment of clip."),
                           r3d->ExposureAdjust(), true, true,
                           (Fl_Callback*)change_exposure_adjust_cb,
                           ImageProcessingLimits::ExposureMin,
                           ImageProcessingLimits::ExposureMax );

                add_float( _("Saturation"), _("Saturation of clip."),
                           r3d->Saturation(), true, true,
                           (Fl_Callback*)change_saturation_cb,
                           ImageProcessingLimits::SaturationMin,
                           ImageProcessingLimits::SaturationMax );

                add_float( _("Contrast"),
                           _("Overall contrast of clip."),
                           r3d->Contrast(), true, true,
                           (Fl_Callback*)change_contrast_cb,
                           ImageProcessingLimits::ContrastMin,
                           ImageProcessingLimits::ContrastMax );

                add_float( _("Flut"),
                           _("Adjust ISO without clipping the highlights\n"
                             "or crushing the shadows."),
                           r3d->Flut(), true, true,
                           (Fl_Callback*)change_flut_cb,
                           ImageProcessingLimits::FLUTMin,
                           ImageProcessingLimits::FLUTMax );
            }
            else
            {
                add_float( _("Exposure Compensation"),
                           _("Exposure compensation of clip."),
                           r3d->ExposureCompensation(), true, true,
                           (Fl_Callback*)change_exposure_compensation_cb,
                           ImageProcessingLimits::ExposureMin,
                           ImageProcessingLimits::ExposureMax );
            }

            if ( r3d->color_space() != "REDWideGamutRGB" )
            {
                group++;
                add_float( _("Brightness"),
                           _("Overall brightness of clip."),
                           r3d->Brightness(), true, true,
                           (Fl_Callback*)change_brightness_cb,
                           ImageProcessingLimits::BrightnessMin,
                           ImageProcessingLimits::BrightnessMax );

               add_float( _("Shadow"), _("Shadow levels of clip."),
                           r3d->Shadow(), true, true,
                           (Fl_Callback*)change_shadow_cb,
                           ImageProcessingLimits::ShadowMin,
                           ImageProcessingLimits::ShadowMax );

               group++;

               add_float( _("Red Gain"), _("Red Gain of clip."),
                           r3d->GainRed(), true, true,
                           (Fl_Callback*)change_gain_red_cb,
                          ImageProcessingLimits::GainsMin,
                          ImageProcessingLimits::GainsMax );
               add_float( _("Green Gain"), _("Green Gain of clip."),
                           r3d->GainGreen(), true, true,
                           (Fl_Callback*)change_gain_green_cb,
                          ImageProcessingLimits::GainsMin,
                          ImageProcessingLimits::GainsMax );
               add_float( _("Blue Gain"), _("Blue Gain of clip."),
                           r3d->GainBlue(), true, true,
                           (Fl_Callback*)change_gain_blue_cb,
                          ImageProcessingLimits::GainsMin,
                          ImageProcessingLimits::GainsMax );
           }

            if ( r3d->is_hdr() )
            {
                ++group;

                stringArray options;
                options.push_back( _("Use TrackNo") );
                options.push_back( _("Do Blend") );
                assert0( r3d->hdr_mode() >= 0 &&
                         r3d->hdr_mode() < options.size() );
                content = options[ r3d->hdr_mode() ];
                add_enum( _("HDR blend"), _("HDR blend type"),
                          content, options, true, (Fl_Callback*)change_blend_cb );

                add_int( _("Track #"), _("HDR Track No"),
                         (unsigned)r3d->trackNo(), true, true,
                         (Fl_Callback*)change_trackno_cb,
                         (unsigned)0, (unsigned)1 );

                add_float( _("Bias"), _("HDR Blend bias of clip."),
                           r3d->Bias(), true, true,
                           (Fl_Callback*)change_bias_cb, -1.0f, 1.f );
            }

            if ( r3d->scale() == 0 )
            {
                ++group;

                options.clear();
                options.push_back( "Off" );
                options.push_back( "Low" );
                options.push_back( "Medium" );
                options.push_back( "High" );
                assert0( r3d->Sharpness() >= 0 &&
                         r3d->Sharpness() < options.size() );
                content = options[ r3d->Sharpness() ];
                add_enum( _("Sharpness"),
                          _("Compensation for the OLPF (low-pass) filter.\n"
                            "Only applicable when decoding 1:1"),
                          content, options, true,
                          (Fl_Callback*)change_sharpness_cb );

                options.clear();
                options.push_back( "Off" );
                options.push_back( "Minimum" );
                options.push_back( "Milder" );
                options.push_back( "Mild" );
                options.push_back( "Medium" );
                options.push_back( "Strong" );
                options.push_back( "Maximum" );
                assert0( r3d->Denoise() >= 0 &&
                         r3d->Denoise() < options.size() );
               content = options[ r3d->Denoise() ];
                add_enum( _("Denoise"),
                          _("Amount of denoise filter to apply.\n"
                            "Only applicable when decoding 1:1."),
                          content, options, true,
                          (Fl_Callback*)change_denoise_cb );

                options.clear();
                options.push_back( "Low" );
                options.push_back( "Medium" );
                options.push_back( "High" );
                assert0( r3d->Detail() >= 0 && r3d->Detail() < options.size() );
                content = options[ r3d->Detail() ];
                add_enum( _("Detail"),
                          _("Amount of detail extraction.\n"
                            "Only applicable when decoding 1:1."),
                          content, options, true,
                          (Fl_Callback*)change_detail_cb );
            }

            ++group;

        }

        // First, parse clip attributes
        CMedia::Attributes::const_iterator i = cattrs.begin();
        CMedia::Attributes::const_iterator e = cattrs.end();
        for ( ; i != e; ++i )
        {
            if ( i->first.find( _("Video") ) != std::string::npos )
                group = 1;
            else if ( i->first.find( _("Audio") ) != std::string::npos )
                group = 2;
            else
                group = 3;
            process_attributes( i );
        }

        // Then, parse frame attributes
        i = attrs.begin();
        e = attrs.end();
        for ( ; i != e; ++i )
        {
            if ( i->first == _("Mipmap Levels") )
            {
                exrImage* exr = dynamic_cast< exrImage* >( img );
                if ( exr )
                {
                    add_int( _("Mipmap Level"), _("Mipmap Level"),
                             exr->levelX(), true, true,
                             (Fl_Callback*)change_mipmap_cb, 0, 20 );
                    exr->levelY( exr->levelX() );
                }
                oiioImage* oiio = dynamic_cast< oiioImage* >( img );
                if ( oiio )
                {
                    add_int( _("Mipmap Level"), _("Mipmap Level"),
                             oiio->level(), true, true,
                             (Fl_Callback*)change_mipmap_cb, 0,
                             (int)oiio->mipmap_levels()-1 );
                }
            }
            else if ( i->first == _("X Ripmap Levels") )
            {
                exrImage* exr = dynamic_cast< exrImage* >( img );
                if ( exr )
                {
                    add_int( _("X Ripmap Level"), _("X Ripmap Level"),
                             exr->levelX(), true, true,
                             (Fl_Callback*)change_x_ripmap_cb, 0, 20 );
                }
            }
            else if ( i->first == _("Y Ripmap Levels") )
            {
                exrImage* exr = dynamic_cast< exrImage* >( img );
                if ( exr )
                {
                    add_int( _("Y Ripmap Level"), _("Y Ripmap Level"),
                             exr->levelY(), true, true,
                             (Fl_Callback*)change_y_ripmap_cb, 0, 20 );
                }
            }
            else if ( i->first == N_("Input Color Space") )
            {
                continue;
            }
            else
            {
                if ( i->first.find( _("Video") ) != std::string::npos )
                    group = 1;
                else if ( i->first.find( _("Audio") ) != std::string::npos )
                    group = 2;
                else
                    group = 3;
                process_attributes( i );
            }


        }

        m_attributes->show();
    }


    if ( num_video_streams > 0 )
    {
        for ( int i = 0; i < num_video_streams; ++i )
        {

            char buf[256];
            sprintf( buf, _("Video Stream #%d"), i+1 );

            m_curr = add_browser( m_video );

            m_curr->copy_label( buf );



            const CMedia::video_info_t& s = img->video_info(i);

            add_bool( _("Known Codec"), _("mrViewer knows codec used"),
                      s.has_codec );
            add_text( _("Codec"), _("Codec Name"), s.codec_name );
            add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
            add_bool( _("B Frames"), _("Video has B frames"), s.has_b_frames );
            ++group;


            add_text( _("Pixel Format"), _("Pixel Format"), s.pixel_format );
            ++group;



            const char* name = "";

            if      ( is_equal( s.fps, 29.97 ) )     name = "(NTSC)";
            else if ( is_equal( s.fps, 30.0 ) )      name = "(60hz HDTV)";
            else if ( is_equal( s.fps, 25.0 ) )      name = "(PAL)";
            else if ( is_equal( s.fps, 24.0 ) )      name = "(Film)";
            else if ( is_equal( s.fps, 50.0 ) )      name = _("(PAL Fields)");
            else if ( is_equal( s.fps, 59.940059 ) ) name = _("(NTSC Fields)");


            sprintf( buf, "%g %s", s.fps, name );

            add_text( _("FPS"), _("Frames per Second"), buf );

            ++group;
            add_text( _("Language"), _("Language if known"), s.language );
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition );


            ++group;
            add_time( _("Start"), _("Start of Video"), s.start, s.fps );
            add_time( _("Duration"), _("Duration of Video"),
                      s.duration, s.fps );


            // m_curr->relayout();

            // m_curr->parent()->relayout();
        }
        m_video->show();
    }


    if ( num_audio_streams > 0 )
    {
        for ( int i = 0; i < num_audio_streams; ++i )
        {
            char buf[256];

            m_curr = add_browser( m_audio );
            sprintf( buf, _("Audio Stream #%d"), i+1 );
            m_curr->copy_label( buf );


            const CMedia::audio_info_t& s = img->audio_info(i);



            add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      s.has_codec );
            add_text( _("Codec"), _("Codec Name"), s.codec_name );
            add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
            ++group;


            const char* channels = "Stereo";
            if ( s.channels == 1 )      channels = "Mono";
            else if ( s.channels == 2 ) channels = "Stereo";
            else if ( s.channels == 6 ) channels = "5:1";
            else if ( s.channels == 8 ) channels = "7:1";
            else {
                sprintf( buf, N_("%d"), s.channels );
                channels = buf;
            }


            add_text( _("Format"), _("Format"), s.format );
            add_text( _("Channels"), _("Number of audio channels"), channels );
            sprintf( buf, _("%d Hz."), s.frequency );

            add_text( _("Frequency"), _("Frequency of audio"), buf );
            sprintf( buf, _("%d kb/s"), s.bitrate/1000 );

            add_text( _("Max. Bitrate"), _("Max. Bitrate"), buf );

            ++group;

            add_text( _("Language"), _("Language if known"), s.language );
            ++group;

            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition);
            ++group;


            add_time( _("Start"), _("Start of Audio"), s.start, img->fps() );
            add_time( _("Duration"), _("Duration of Audio"),
                      s.duration, img->fps() );


            // m_curr->relayout();
            // m_curr->parent()->relayout();
        }


        m_audio->show();

    }

    if ( num_subtitle_streams > 0 )
    {
        for ( int i = 0; i < num_subtitle_streams; ++i )
        {
            char buf[256];

            m_curr = add_browser( m_subtitle );
            sprintf( buf, _("Subtitle Stream #%d"), i+1 );
            m_curr->copy_label( buf );

            const CMedia::subtitle_info_t& s = img->subtitle_info(i);

            add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      s.has_codec );
            add_text( _("Codec"), _("Codec name"), s.codec_name );
            add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
            add_bool( _("Closed Captions"), _("Video has Closed Captions"),
                      s.closed_captions );
            ++group;

            sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
            add_text( _("Avg. Bitrate"), _("Avg. Bitrate"), buf );

            ++group;
            add_text( _("Language"), _("Language if known"), s.language );
            ++group;
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition );

            ++group;
            add_time( _("Start"), _("Start of Subtitle"), s.start, img->fps() );
            add_time( _("Duration"), _("Duration of Subtitle"),
                      s.duration, img->fps() );

            //    m_curr->relayout();
            //     m_curr->parent()->relayout();
        }

        m_subtitle->show();
    }

    // m_all->layout();


}

void ImageInformation::refresh()
{
    img->image_damage( img->image_damage() & ~CMedia::kDamageData );
    bool movie = ( dynamic_cast< aviImage* >( get_image() ) != NULL );
    if ( movie && filled && !img->right_eye() && !img->is_left_eye() )
    {
        return;
    }

    filled = false;

    hide_tabs();

    m_image->clear();
    m_video->clear();
    m_audio->clear();
    m_subtitle->clear();
    m_attributes->clear();

    DBG2;

    if ( img == NULL || !visible_r() ) {
        Fl_Group::current(0);
        return;
    }

    if ( img->is_stereo() && (img->right_eye() || !img->is_left_eye()) )
    {
        m_button->show();
        DBG3;
    }
    else
    {
        m_button->hide();
        DBG3;
    }

    fill_data();

    DBG2;

#define USE_ENDS
#ifndef USE_ENDS
    m_image->layout();
    m_video->layout();
    m_audio->layout();
    m_subtitle->layout();
    m_attributes->layout();

    m_all->layout();
#else
    m_image->end();
    m_video->end();
    m_audio->end();
    m_subtitle->end();
    m_attributes->end();

    m_all->end();
#endif

    DBG2;

#ifdef USE_ENDS
    DBG2;
    end();
#endif

    m_all->show();

    filled = true;

    Fl_Group::current(0);
    DBG3;
}

void
ImageInformation::resize( int x, int y, int w, int h )
{
    m_scroll->scroll_to( 0, 0 );  // needed to avoid m_all shifting downwards
    m_all->resize( x, y, w, h );
    Fl_Group::resize( x, y, w, h );
}

mrv::Table* ImageInformation::add_browser( mrv::CollapsibleGroup* g )
{
    if (!g) return NULL;

    X = 0;
    Y = g->y() + line_height();

    mrv::Table* table = new mrv::Table( 0, Y, w(), 20 /*, g->label() */ );
    table->column_separator(true);
    table->auto_resize( true );
    table->labeltype(FL_NO_LABEL);
    table->col_width(0, kMiddle );

    static const char* headers[] = { _("Attribute"), _("Value"), 0 };
    table->column_labels( headers );
    table->align(FL_ALIGN_CENTER);
    table->end();

    g->add( table );

    group = row = 0; // controls line colors

    return table;
}

int ImageInformation::line_height()
{
    return 24;
}

Fl_Color ImageInformation::get_title_color()
{
    return kTitleColors[ group % kSizeOfTitleColors ];
}

Fl_Color ImageInformation::get_widget_color()
{
    Fl_Color col = kRowColors[ row % kSizeOfRowColors ];
    ++row;
    return col;
}

void ImageInformation::icc_callback( Fl_Widget* t, ImageInformation* v )
{
    attach_icc_profile( v->get_image() );
    v->filled = false;
    v->refresh(); // @TODO: move this somewhere else
}

void ImageInformation::ctl_callback( Fl_Widget* t, ImageInformation* v )
{
    attach_ctl_script( v->get_image(), v->main() );
    v->filled = false;
}

void ImageInformation::ctl_idt_callback( Fl_Widget* t,
                                         ImageInformation* v )
{
    attach_ctl_idt_script( v->get_image(), v->main() );
    v->filled = false;
}

void ImageInformation::ctl_lmt_callback( Fl_Widget* t,
                                         CtlLMTData* c )
{
    ImageInformation* v = (ImageInformation*) c->widget;
    size_t idx = c->idx;

    attach_ctl_lmt_script( v->get_image(), idx, v->main() );
    v->filled = false;
}

void ImageInformation::compression_cb( mrv::PopupMenu* t, ImageInformation* v )
{
    unsigned   idx = t->value();
    CMedia* img = v->get_image();
    img->compression( idx );
    t->label( t->child(idx)->label() );
    v->filled = false;
}

void ImageInformation::add_icc( const char* name,
                                const char* tooltip,
                                const char* content,
                                const bool editable,
                                Fl_Callback* callback )
{
    if ( !content )
        content = _("None");

    if ( !editable )
        return add_text( name, tooltip, content );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;

    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Group* sg = new Fl_Group( X+kMiddle, Y, g->w()-kMiddle-X, hh );
        sg->end();

        Fl_Input* widget = new Fl_Input( X+kMiddle, Y, sg->w()-50, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->color( colB );
        widget->textcolor( FL_BLACK );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( callback )
            widget->callback( (Fl_Callback*)icc_callback, (void*)this );

        sg->add( widget );

        Fl_Button* pick = new Fl_Button( X + kMiddle + sg->w()-50, Y,
                                         50, hh, _("Load") );
        pick->callback( (Fl_Callback*)icc_callback, (void*)this );
        sg->add( pick );
        sg->resizable(widget);
        sg->end();

        m_curr->add( sg );
    }

    m_curr->layout();
}

void ImageInformation::add_button( const char* name,
                                   const char* tooltip,
                                   Fl_Callback* callback,
                                   Fl_Callback* callback2 )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    int hh = line_height();
    Y += hh;

    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    g = new Fl_Group( kMiddle, Y, w() - kMiddle, hh );
    {
        int w2 = w() - kMiddle;
        w2 /= 2;
        Fl_Button* widget = new Fl_Button( kMiddle, Y, w2, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("Load") );
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w2, Y, w2, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("Reset") );
        if ( callback )
            widget->callback( (Fl_Callback*)callback2, (void*)this );
        g->end();
    }
    m_curr->add( g );

    m_curr->layout();
}


void ImageInformation::add_scale( const char* name,
                                  const char* tooltip,
                                  int pressed,
                                  int num_scales,
                                  Fl_Callback* callback )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    int hh = line_height();
    Y += hh;

    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    g = new Fl_Group( kMiddle, Y, w() - kMiddle, hh );
    {
        int w5 = w() - kMiddle;
        w5 /= num_scales;
        Fl_Button* widget = new Fl_Button( kMiddle, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:1") );
        if ( pressed == 0 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:2") );
        if ( pressed == 1 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5*2, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:4") );
        if ( pressed == 2 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5*3, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:8") );
        if ( pressed == 3 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        if ( num_scales > 4 )
        {
            widget = new Fl_Button( kMiddle+w5*4, Y, w5, hh );
            widget->tooltip( tooltip );
            widget->copy_label( _("1:16") );
            if ( pressed == 4 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
        }
        g->end();
    }
    m_curr->add( g );

    m_curr->layout();
}


void ImageInformation::add_ctl( const char* name,
                                const char* tooltip,
                                const char* content,
                                const bool editable,
                                Fl_Callback* callback )
{
    if ( !editable )
        return add_text( name, tooltip, content );


    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );


    {
        Fl_Group* sg = new Fl_Group( kMiddle, Y, w()-kMiddle, hh );

        Fl_Input* widget = new Fl_Input( kMiddle, Y, sg->w()-50, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( callback )
            widget->callback( (Fl_Callback*)ctl_callback, (void*)this );

        Fl_Button* pick = new Fl_Button( kMiddle + sg->w()-50, Y, 50, hh,
                                         _("Pick") );
        pick->callback( (Fl_Callback*)ctl_callback, this );
        sg->resizable(widget);
        sg->end();

        m_curr->add( sg );
    }
    m_curr->layout();
}



void ImageInformation::add_ocio_ics( const char* name,
                                     const char* tooltip,
                                     const char* content,
                                     const bool editable,
                                     Fl_Callback* callback )
{
    if ( !editable )
        return add_text( name, tooltip, content );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Group* sg = new Fl_Group( kMiddle, Y, kMiddle, hh );
        sg->end();

        Fl_Input* widget = new Fl_Input( kMiddle, Y, sg->w()-50, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        widget->tooltip( tooltip ? tooltip : lbl->label() );

        sg->add( widget );

        Fl_Button* pick = new Fl_Button( kMiddle + sg->w()-50, Y, 50, hh,
                                         _("Pick") );
        pick->callback( (Fl_Callback*)attach_ocio_ics_cb, view() );
        sg->add( pick );

        m_curr->add( sg );
    }
    m_curr->layout();
}


void ImageInformation::add_ctl_idt( const char* name,
                                    const char* tooltip,
                                    const char* content,
                                    const bool editable,
                                    Fl_Callback* callback )
{
    if ( !editable )
        return add_text( name, tooltip, content );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Group* sg = new Fl_Group( X+kMiddle, Y, w()-kMiddle-X, hh );
        sg->end();

        Fl_Input* widget = new Fl_Input( X+kMiddle, Y, sg->w()-50, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_WHITE );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( callback )
            widget->callback( (Fl_Callback*)ctl_idt_callback, (void*)this );

        sg->add( widget );

        Fl_Button* pick = new Fl_Button( X+kMiddle + sg->w()-50, Y, 50, hh,
                                         _("Pick") );
        pick->callback( (Fl_Callback*)ctl_idt_callback, this );
        sg->add( pick );
        sg->resizable(widget);

        m_curr->add( sg );
    }
    m_curr->layout();
}



void ImageInformation::add_ctl_lmt( const char* name,
                                    const char* tooltip,
                                    const char* content,
                                    const size_t idx,
                                    const bool editable,
                                    Fl_Callback* callback )
{
    if ( !editable )
        return add_text( name, tooltip, content );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->add( widget );
    }
    m_curr->add( g );

    bool sop_sat_node = false;
    if ( content )
    {
        std::string n = content;
        n = n.substr( 4, 7 );
        if ( n == "SOPNode" || n == "SatNode" )
            sop_sat_node = true;
    }

    {
        Fl_Group* sg = new Fl_Group( X+kMiddle, Y, w()-kMiddle-X, hh );
        sg->end();

        Fl_Input* widget = new Fl_Input( X+kMiddle, Y, sg->w()-50
                                         -50*sop_sat_node, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_WHITE );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );

        CtlLMTData* c = new CtlLMTData;
        c->widget = this;
        c->idx    = idx;

        widget_data.push_back( c );

        if ( callback )
            widget->callback( callback, (void*)c );

        sg->add( widget );

        if ( sop_sat_node )
        {
            Fl_Button* modify = new Fl_Button( X+kMiddle + sg->w()-100, Y,
                                               50, hh, _("Values") );
            mrv::ImageView* view = main()->uiView;
            modify->callback( (Fl_Callback*)modify_sop_sat_cb, view );
            sg->add( modify );
        }


        Fl_Button* pick = new Fl_Button( X+kMiddle + sg->w()-50, Y, 50, hh,
                                         _("Pick") );
        pick->callback( (Fl_Callback*)ctl_lmt_callback, c );
        sg->add( pick );
        sg->resizable(widget);

        m_curr->add( sg );
    }
    m_curr->layout();
}


void ImageInformation::add_text( const char* name,
                                 const char* tooltip,
                                 const char* content,
                                 const bool editable,
                                 const bool active,
                                 Fl_Callback* callback )
{


    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    {
        Fl_Widget* widget = NULL;
        if ( !editable )
        {
            Fl_Output* o = new Fl_Output( kMiddle, Y, w()-kMiddle, hh );
            widget = o;
            o->value( content );
            o->textcolor( FL_BLACK );
        }
        else
        {
            Fl_Input* o = new Fl_Input( kMiddle, Y, w()-kMiddle, hh );
            widget = o;
            o->value( content );
            o->textcolor( FL_BLACK );
        }
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( !editable )
        {
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
            if (!active) widget->deactivate();
        }
        m_curr->add( widget );
    }
    m_curr->layout();
}


void ImageInformation::add_text( const char* name,
                                 const char* tooltip,
                                 const std::string& content,
                                 const bool editable,
                                 const bool active,
                                 Fl_Callback* callback )
{
    add_text( name, tooltip, content.c_str(), editable, active, callback );
}

void ImageInformation::add_int( const char* name, const char* tooltip,
                                const int content, const bool editable,
                                const bool active,
                                Fl_Callback* callback,
                                const int minV, const int maxV,
                                const int when )
{

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, w()-kMiddle, hh );
        p->end();
        p->box( FL_FLAT_BOX );
        // p->set_horizontal();
        p->begin();

        if ( !editable )
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "%d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 50, hh );
            sprintf( buf, "%d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            Fl_Slider* slider = new Fl_Slider( kMiddle+50, Y, p->w()-40, hh );
            // slider->type(Fl_Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );
            int maxS = maxV;

            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > maxS ) maxS = content+50;
            slider->maximum( maxS );

            slider->value( content );
            slider->step( 1.0 );
            // slider->slider_size(10);
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->when( when );
            slider->callback( (Fl_Callback*)int_slider_cb, widget );

            p->resizable(slider);
        }
        p->end();
        m_curr->add( p );
        if ( !active )
        {
            p->deactivate();
        }
    }
    m_curr->layout();
}

void ImageInformation::add_enum( const char* name,
                                 const char* tooltip,
                                 const size_t content,
                                 const char* const* options,
                                 const size_t num,
                                 const bool editable,
                                 Fl_Callback* callback
                               )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        mrv::PopupMenu* widget = new mrv::PopupMenu( kMiddle, Y,
                                                     w()-kMiddle, hh );
        widget->type( 0 );
        widget->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
        widget->color( colB );
        widget->labelcolor( FL_BLACK );
        widget->textcolor( FL_BLACK );
        for ( size_t i = 0; i < num; ++i )
        {
            widget->add( _( options[i] ) );
        }
        widget->value( unsigned(content) );
        widget->copy_label( _( options[content] ) );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        widget->menu_end();

        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        m_curr->add( widget );
    }
    m_curr->layout();
}


void ImageInformation::add_enum( const char* name,
                                 const char* tooltip,
                                 const std::string& content,
                                 stringArray& options,
                                 const bool editable,
                                 Fl_Callback* callback
                               )
{
    size_t index;
    stringArray::iterator it = std::find( options.begin(), options.end(),
                                          content );
    if ( it != options.end() )
    {
        index = std::distance( options.begin(), it );
    }
    else
    {
        index = options.size();
        options.push_back( content );
    }

    size_t num = options.size();
    const char** opts = new const char*[num];
    for ( size_t i = 0; i < num; ++i )
        opts[i] = options[i].c_str();

    add_enum( name, tooltip,index, opts, num, editable, callback );

    delete [] opts;
}




void ImageInformation::add_int( const char* name,
                                const char* tooltip,
                                const unsigned int content,
                                const bool editable,
                                const bool active,
                                Fl_Callback* callback,
                                const unsigned int minV,
                                const unsigned int maxV )
{
    assert0( m_curr != NULL );
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->copy_label( name );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        g->add( widget );
    }
    m_curr->add( g );

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, w()-kMiddle, hh );
        p->box( FL_FLAT_BOX );
        // p->set_horizontal();
        p->begin();

        if ( !editable )
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "%d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->textcolor( FL_BLACK );
            widget->color( colB );
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 60, hh );
            sprintf( buf, "%d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->textcolor( FL_BLACK );
            widget->color( colB );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            mrv::Slider* slider = new mrv::Slider( kMiddle+60, Y,
                                                   p->w()-60, hh );
            slider->type(mrv::Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );

            unsigned maxS = maxV;
            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > 10 && maxV <= 10 ) maxS = 100;
            else if ( content > maxS ) maxS = content+50;


            slider->maximum( maxS );
            slider->value( content );
            slider->step( 1.0 );
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->slider_size(10);
            slider->when( FL_WHEN_RELEASE );
            slider->callback( (Fl_Callback*)int_slider_cb, widget );

            p->resizable(slider);
        }
        p->end();
        m_curr->add( p );
    }

    m_curr->layout();
}


void ImageInformation::add_time( const char* name, const char* tooltip,
                                 const double content,
                                 const double fps, const bool editable )
{
    int64_t seconds = (int64_t) content;
    int ms = (int) ((content - (double) seconds) * 1000);

    int64_t frame = int64_t( content * fps );

    CMedia* img = get_image();
    if ( img->start_frame() == 1 && frame == 0 ) frame = 1;

    char buf[128];

    sprintf( buf, _( "Frame %" PRId64 " " ), frame );
    std::string text = buf;

    sprintf( buf, _("%" PRId64 " seconds %d ms."), seconds, ms );
    text += buf;

    if ( content > 60.0 )
    {
        int64_t hours, minutes;
        hours    = seconds / 3600;
        seconds -= hours * 3600;
        minutes  = seconds / 60;
        seconds -= minutes * 60;

        sprintf( buf,
                 _(" ( %02" PRId64 ":%02" PRId64 ":%02" PRId64 "  %d ms. )"),
                 hours, minutes, seconds, ms );
        text += buf;
    }

    add_text( name, tooltip, text, false );
}

void ImageInformation::add_int64( const char* name,
                                  const char* tooltip,
                                  const int64_t content )
{
    assert0( m_curr != NULL );
    char buf[128];
    sprintf( buf, N_("%" PRId64), content );
    add_text( name, tooltip, buf, false );
}

void ImageInformation::add_rect( const char* name, const char* tooltip,
                                 const mrv::Recti& content,
                                 const bool editable, Fl_Callback* callback )
{
    assert0( m_curr != NULL );
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->copy_label( name );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        g->add( widget );
    }
    m_curr->add( g );

    char buf[64];
    unsigned dw = (w() - kMiddle) / 6;
    Fl_Group* g2 = new Fl_Group( kMiddle, Y, w()-kMiddle, hh );
    g2->end();
    if ( tooltip ) g2->tooltip( tooltip );
    else g2->tooltip( lbl->label() );
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, dw, hh );
        sprintf( buf, "%d", content.l() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->color( colB );
        widget->textcolor( FL_BLACK );
        widget->box( FL_FLAT_BOX );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, img );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw, Y, dw, hh );
        sprintf( buf, "%d", content.t() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, img );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*2, Y, dw, hh );
        sprintf( buf, "%d", content.r()-1 );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, img );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*3, Y, dw, hh );
        sprintf( buf, "%d", content.b()-1 );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, img );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*4, Y, dw,
                                                 hh, "W:" );
        sprintf( buf, "%d", content.w() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->color( colB );
        widget->labelcolor( FL_LIGHT3 );
        widget->textcolor( FL_BLACK );
        widget->deactivate();
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle + dw*5, Y, dw,
                                                 hh, "H:" );
        sprintf( buf, "%d", content.h() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_LIGHT3 );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        widget->deactivate();
        g2->add( widget );
    }
    m_curr->add( g2 );
    m_curr->layout();
}

void ImageInformation::add_float( const char* name,
                                  const char* tooltip,
                                  const float content, const bool editable,
                                  const bool active,
                                  Fl_Callback* callback,
                                  const float minV, float maxV,
                                  const int when,
                                  const mrv::Slider::SliderType type )
{
    assert0( m_curr != NULL );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, w()-kMiddle, hh );
        p->box( FL_FLAT_BOX );
        // p->set_horizontal();
        p->begin();

        if ( !editable )
        {
            Fl_Float_Input* widget = new Fl_Float_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "%g", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Float_Input* widget = new Fl_Float_Input( kMiddle, Y, 60, hh );
            sprintf( buf, "%g", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            mrv::Slider* slider = new mrv::Slider( kMiddle+60, Y,
                                                   p->w()-60, hh );
            slider->ticks(mrv::Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );

            double maxS = maxV;
            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > 10 && maxV <= 10 ) maxS = 100;
            else if ( content > maxS ) maxS = content+50;

            slider->maximum( maxS );
            slider->value( content );
            // slider->slider_size(10);
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->slider_type( type );
            slider->when( when );
            slider->callback( (Fl_Callback*)float_slider_cb, widget );

            p->resizable(slider);
        }
        p->end();
        m_curr->add( p );
        if ( !active ) {
            p->deactivate();
        }
    }
    m_curr->layout();
}

void ImageInformation::add_bool( const char* name,
                                 const char* tooltip,
                                 const bool content,
                                 const bool editable,
                                 Fl_Callback* callback )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;

    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();

    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Input* widget = new Fl_Input( kMiddle, Y, w()-kMiddle, 20 );
        widget->value( content? _("Yes") : _("No") );
        widget->box( FL_FLAT_BOX );
        widget->align(FL_ALIGN_LEFT);
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, img );
        }
        m_curr->add( widget );
    }
    m_curr->layout();

}


} // namespace mrv
