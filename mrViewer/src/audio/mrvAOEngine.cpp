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
 * @file   mrvAOEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using libao.
 *
 *
 */

#ifdef AOENGINE

#include <cstdio>
#include <cstdlib>

#include "RtAudio.h"
#include "ao/ao.h"

#include <FL/fl_utf8.h>

#include "gui/mrvIO.h"
#include "audio/mrvAOEngine.h"

namespace
{

const char* kModule = "ao";

}


namespace mrv {

#define THROW(x) throw( exception(x) )

    std::atomic< unsigned int >    AOEngine::_instances( 0 );

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

    void AOEngine::refresh_devices()
    {
        // Create default device
        _devices.clear();
        _device_idx = 0;
        RtAudio audio;
        // Determine the number of devices available
        unsigned int devices = audio.getDeviceCount();
        // Scan through devices for various capabilities
        RtAudio::DeviceInfo info;
        for ( unsigned int i = 0; i < devices; ++i )
        {
            info = audio.getDeviceInfo( i );
            if ( info.probed == false || info.inputChannels ) continue;


            if ( info.isDefaultOutput )
            {
                Device d( "default", info.name );
                _devices.insert( _devices.begin(), d );
            }
            else
            {
                char buf[64];
                sprintf( buf, "%d", i );

                Device d( buf, info.name );
                _devices.push_back( d );
            }
        }

        if ( _devices.empty() )
        {
            Device def( "default", "Default Audio Device" );
            _devices.push_back( def );
        }
    }

bool AOEngine::initialize()
{
    if ( _instances == 0 )
    {

        const char* env = fl_getenv( "AO_PLUGIN_PATH" );
        if (! env || strlen(env) == 0 )
        {
            env = fl_getenv( "MRV_ROOT" );
            if ( env )
            {
                std::string path = env;
                path += "/lib/ao/plugins-4";

                char buf[1024];
                sprintf( buf, "AO_PLUGIN_PATH=%s", path.c_str() );
                LOG_INFO( buf );
                putenv( strdup( buf ) );
            }
        }

        refresh_devices();

        ao_initialize();
    }

    _audio_device = ao_default_driver_id();


    ++_instances;
    return true;
}


bool AOEngine::shutdown()
{

    close();

    --_instances;

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
    _volume = v;
}

AOEngine::AudioFormat AOEngine::default_format()
{
    return kS32LSB;
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
                     const AudioFormat format )
{
    try
    {
        flush();

        _enabled = false;
        ao_sample_format fmt;
        memset( &fmt, 0, sizeof(ao_sample_format) );
        switch( format )
        {
        case kU8:
                fmt.bits = 8; break;
        case kS16LSB:
                fmt.bits = 16; break;
        case kS32LSB:
                fmt.bits = 32; break;
        default:
                LOG_ERROR( "Unknown audio format.  Setting 32 bits" );
                fmt.bits = 32; break;
        }
        fmt.rate = freq;
        fmt.channels = channels;
        fmt.byte_format = AO_FMT_LITTLE;
        fmt.matrix = strdup( N_("L,C,R,CL,CR,SL,SR,BL,BC,BR,LFE") );


        ao_option* options = NULL;


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
            break;
        }
        case kS32LSB:
        {
            size_t samples = size / 4;
            int32_t* s32data = new int32_t[samples];
            memcpy( s32data, d, size );
            for ( size_t i = 0; i < samples; ++i )
            {
                s32data[i] *= _volume;
            }
            data = (int16_t*) s32data;
            break;
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
            break;
        }
        }
    }

    if ( !_enabled ) return true;
    if ( !_device) return false;

    int ok = ao_play( _device, (char*)data, size );

    if ( ok == 0 )
    {
        LOG_ERROR( _("Error playing sample. ") << ao_error_text(errno) );
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
            LOG_ERROR( _("Error closing ao device. ")
                       << ao_error_text(errno) );
        }
        _device = NULL;

        _enabled = false;
        return true;
    }
    return false;
}


} // namespace mrv

#endif // AOENGINE
