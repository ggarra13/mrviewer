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
 * @file   mrvRtAudioEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using RtAudio for portability
 *
 *
 */


#include <cstdio>
#include <cstdlib>

#include <iostream>
using namespace std;

#include "audio/mrvRtAudioEngine.h"
#include "gui/mrvPreferences.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"

#ifdef _WIN32
#undef fprintf
#endif

namespace mrv {

    static const char* kModule = "rtaudio";


#define THROW(x) throw( AudioEngine::exception(x) )

    unsigned int RtAudioEngine::_instances = 0;

unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
const unsigned int callbackReturnValue = 1;

// Two-channel sawtooth wave generator.
    int myCallback( void *outputBuffer, void * /*inputBuffer*/,
                    unsigned int nBufferFrames,
                    double /*streamTime*/, RtAudioStreamStatus status,
                    void *userData )
    {
        unsigned int i, j;
        double *buffer = (double *) outputBuffer;
        double *lastValues = (double *) userData;
        if ( status )
            LOG_ERROR( "Stream underflow detected!" );
        // Write interleaved audio data.
        for ( i=0; i<nBufferFrames; i++ ) {
            for ( j=0; j<2; j++ ) {
                *buffer++ = lastValues[j];
                lastValues[j] += 0.005 * (j+1+(j*0.1));
                if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
            }
        }

        frameCounter += nBufferFrames;
        if ( checkCount && ( frameCounter >= nFrames ) )
            return callbackReturnValue;

        return 0;
    }

RtAudioEngine::RtAudioEngine() :
    AudioEngine(),
    audio( RtAudio::MACOSX_CORE )
{
    initialize();
}

RtAudioEngine::~RtAudioEngine()
{
    shutdown();
}

bool RtAudioEngine::initialize()
{

    if ( _devices.empty() )
    {
        // Create default device
        Device def( "default", _("Default Audio Device") );
        _devices.push_back( def );


        // Determine the number of devices available
        unsigned int devices = audio.getDeviceCount();
        // Scan through devices for various capabilities
        RtAudio::DeviceInfo info;
        for ( unsigned int i = 0; i < devices; ++i )
        {
            info = audio.getDeviceInfo( i );
            if ( info.probed == false || info.isDefaultInput ) continue;

            char buf[64];
            sprintf( buf, "%d", i );

            Device d( buf, info.name );
            _devices.push_back( d );
        }

    }
    ++_instances;
    return true;
}


bool RtAudioEngine::shutdown()
{
    --_instances;

    close();

    if ( _instances == 0 )
    {
    }
    return true;
}


float RtAudioEngine::volume() const
{
    return _volume;
}

void RtAudioEngine::volume( float v )
{
    _volume = v;
}


bool RtAudioEngine::open( const unsigned channels,
                          const unsigned freq,
                          const AudioFormat format,
                          const unsigned bits )
{

    try
    {
        close();

        RtAudioFormat fmt = RTAUDIO_FLOAT32;
        switch( format )
        {
        case kU8:
            fmt = RTAUDIO_SINT8;
            break;
        case kS16LSB:
            fmt = RTAUDIO_SINT16;
            break;
        case kS32LSB:
            fmt = RTAUDIO_SINT32;
            break;
        default:
        case kFloatLSB:
            fmt = RTAUDIO_FLOAT32;
            break;
        }

        RtAudio::StreamParameters parameters;
        parameters.deviceId = 1; //audio.getDefaultOutputDevice();
        parameters.nChannels = channels;
        unsigned int sampleRate = freq;
        unsigned int bufferFrames = 512; // 256 sample frames

        RtAudio::StreamOptions options;
        options.flags = RTAUDIO_HOG_DEVICE |
                        RTAUDIO_NONINTERLEAVED | RTAUDIO_MINIMIZE_LATENCY |
                        RTAUDIO_SCHEDULE_REALTIME;

        double *data = (double*) calloc( channels, sizeof( double ) );

        audio.showWarnings( true );

        int                  status;
        try {
            audio.openStream( &parameters, NULL, fmt,
                              sampleRate, &bufferFrames, &myCallback,
                              (void*)&data, &options );
            audio.startStream();
        }
        catch ( RtAudioError& e ) {
            LOG_ERROR( e.getMessage() );
            return false;
        }


        _audio_format = format;

        _channels = channels;


        // All okay, enable device
        _enabled = true;
        _old_device_idx = parameters.deviceId;


        /* We're ready to rock and roll. :-) */
        return true;
    }
    catch( const std::exception& e )
    {
        LOG_ERROR( e.what() );
        close();
        struct timespec req = {
            7, 0
        };
        nanosleep( &req, NULL );
        _enabled = false;
    }
    return _enabled;
}


bool RtAudioEngine::play( const char* orig, const size_t size )
{
    if ( !_enabled )   return true;

    if ( audio.isStreamRunning() ) return true;

    audio.startStream();

    return true;
}


void RtAudioEngine::flush()
{
    if ( audio.isStreamRunning() ) audio.abortStream();
}


bool RtAudioEngine::close()
{
    _enabled = false;
    flush();
    if ( audio.isStreamOpen() ) audio.closeStream();

    return true;
}

} // namespace mrv
