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
 * @file   mrvWMMEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using a basic interface and hiding the details.
 *
 *
 */

#include <cstdio>
#include <cstdlib>

#include "ao/ao.h"

#include "gui/mrvIO.h"
#include "audio/mrvAOEngine.h"

namespace
{

const char* kModule = "ao";

}


namespace mrv {

#define THROW(x) throw( exception(x) )

unsigned int     AOEngine::_instances = 0;

AOEngine::AOEngine() :
    AudioEngine(),
    _sample_size(0),
    _audio_device(0),
    _volume( 1.0f ),
    _format( NULL ),
    _device( NULL ),
    _options( NULL )
{
    initialize();
}

AOEngine::~AOEngine()
{
    close();
    shutdown();
}

bool AOEngine::initialize()
{
    if ( _instances == 0 )
    {
        // Create default device
        Device def( "default", "Default Audio Device" );
        _devices.clear();
        _devices.push_back( def );
        _device_idx = 0;

        ao_initialize();
    }

    _audio_device = ao_default_driver_id();

    ++_instances;
    return true;
}


bool AOEngine::shutdown()
{
    --_instances;

    close();

    if ( _instances == 0 )
    {
        ao_shutdown();
    }

    return true;
}


float AOEngine::volume() const
{

    return _volume;
}

void AOEngine::volume( float v )
{
    if (!_device) return;
    _volume = v;
}

AOEngine::AudioFormat AOEngine::default_format()
{
    return kS16LSB;
}

    std::string ao_error_text( int err )
    {
        switch( err )
        {
        case AO_ENODRIVER:
            return _("No driver for device id");
        case AO_ENOTLIVE:
            return _("Driver is not a live output device");
        case AO_EBADOPTION:
            return _("Valid option key has invalid value");
        case AO_EOPENDEVICE:
            return _("Cannot open device");
        case AO_EFAIL:
        default:
            return _("Unknown failure");
        }
    }

bool AOEngine::open( const unsigned channels,
                     const unsigned freq,
                     const AudioFormat format,
                     const unsigned bits )
{
    try
    {
        flush();

        _enabled = false;
        ao_sample_format fmt;
        memset( &fmt, 0, sizeof(ao_sample_format) );
        fmt.bits = 16;
        fmt.rate = freq;
        fmt.channels = channels;
        fmt.byte_format = AO_FMT_LITTLE;
        fmt.matrix = strdup( N_("L,C,R,CL,CR,SL,SR,BL,BC,BR,LFE") );


        ao_option* options = NULL;
        int err = ao_append_option( &options, N_("buffer_time"), N_("250") );
        if ( err == 0 )
        {
            LOG_ERROR( _("Memory failure allocating option") );
            return false;
        }

        _device = ao_open_live( _audio_device, &fmt, options );
        ao_free_options( options );

        if ( _device == NULL )
        {
            _enabled = false;
            LOG_ERROR( _("Error opening ao driver: ") << ao_error_text(errno) );
            return false;
        }


        // ao_plugin_device_init( _device );

        // Store these for future round
        _audio_format = format;
        _channels = channels;
        _old_device_idx = _device_idx;

        // All okay, enable device
        _enabled = true;

        return true;
    }
    catch( const AudioEngine::exception& e )
    {
        close();
        _enabled = false;
        throw(e);
    }
}


bool AOEngine::play( const char* d, const size_t size )
{

    if ( !_device) return false;
    if ( !_enabled ) return true;

    int16_t* data = (int16_t*)d;
    if ( _volume < 0.99f )
    {
        switch ( _audio_format )
        {
        case kU8:
        {
            data = (int16_t*)new uint8_t[size];
            memcpy( data, d, size );
            for ( size_t i = 0; i < size; ++i )
            {
                data[i] *= _volume;
            }
        }
        default:
        case kS16LSB:
        {
            size_t samples = size / 2;
            data = new int16_t[samples];
            memcpy( data, d, size );
            for ( size_t i = 0; i < samples; ++i )
            {
                data[i] *= _volume;
            }
        }
        }
    }

    int ok = ao_play( _device, (char*)data, size );

    if ( ok == 0 )
    {
        LOG_ERROR( _("Error playing sample") );
        close();
        return false;
    }


    return true;
}

void AOEngine::flush()
{
    if ( _device )
    {
    }
}


bool AOEngine::close()
{
    if ( _device )
    {
        int ok = ao_close( _device );
        if ( ok == 0 )
        {
            LOG_ERROR( _("Error closing ao device") );
        }

        _device = NULL;
        _enabled = false;
        return true;
    }
    return false;
}


} // namespace mrv
