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
 * @file   mrvPortAudioEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using PortAudio engine
 *
 *
 */


#ifdef PORTAUDIO

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <iostream>


#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "audio/mrvPortAudioEngine.h"

#ifdef _WIN32
#include <mmreg.h>   // for manufacturer and product IDs
#endif

namespace
{
const char* kModule = "paudio";
}


namespace mrv {

#define THROW(x) throw( exception(x) )

unsigned int     PortAudioEngine::_instances = 0;

/* Microsoft speaker definitions */
#define WAVE_SPEAKER_FRONT_LEFT             0x1
#define WAVE_SPEAKER_FRONT_RIGHT            0x2
#define WAVE_SPEAKER_FRONT_CENTER           0x4
#define WAVE_SPEAKER_LOW_FREQUENCY          0x8
#define WAVE_SPEAKER_BACK_LEFT              0x10
#define WAVE_SPEAKER_BACK_RIGHT             0x20
#define WAVE_SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define WAVE_SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define WAVE_SPEAKER_BACK_CENTER            0x100
#define WAVE_SPEAKER_SIDE_LEFT              0x200
#define WAVE_SPEAKER_SIDE_RIGHT             0x400
#define WAVE_SPEAKER_TOP_CENTER             0x800
#define WAVE_SPEAKER_TOP_FRONT_LEFT         0x1000
#define WAVE_SPEAKER_TOP_FRONT_CENTER       0x2000
#define WAVE_SPEAKER_TOP_FRONT_RIGHT        0x4000
#define WAVE_SPEAKER_TOP_BACK_LEFT          0x8000
#define WAVE_SPEAKER_TOP_BACK_CENTER        0x10000
#define WAVE_SPEAKER_TOP_BACK_RIGHT         0x20000
#define WAVE_SPEAKER_RESERVED               0x80000000

static const int channel_mask[] = {
    WAVE_SPEAKER_FRONT_CENTER,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT     | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_CENTER  | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_LEFT  | WAVE_SPEAKER_BACK_RIGHT | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_LEFT  | WAVE_SPEAKER_BACK_CENTER  | WAVE_SPEAKER_BACK_RIGHT | WAVE_SPEAKER_LOW_FREQUENCY
};


static void MMerror(const char *function, int err)
{
    LOG_ERROR( function << " - " << err << " " << Pa_GetErrorText(err) );
}

#define PA_ERROR(x) MMerror( __FUNCTION__, x );


PortAudioEngine::PortAudioEngine() :
    AudioEngine(),
    _sample_size(0),
    stream( NULL ),
    _samples_per_block( 48000 ),  // 1 second of 48khz audio
    _audio_playback( false ),
    bytesPerBlock( 0 )
{
    initialize();
}

PortAudioEngine::~PortAudioEngine()
{
    close();
    shutdown();
}

void PortAudioEngine::refresh_devices()
{
    _devices.clear();

    Device def( "default", _("Default Audio Device") );
    _devices.push_back( def );

    unsigned int num = Pa_GetDeviceCount();

    char name[256];
    char desc[1024];

    for (unsigned i = 0; i < num; ++i )
    {
        const PaDeviceInfo* woc;
	woc = Pa_GetDeviceInfo( i );

        std::string channels;
        switch( woc->maxOutputChannels )
        {
        case 1:
            channels = "Mono";
        case 2:
            channels = "Stereo";
            break;
        case 6:
            channels = "5:1";
            break;
        case 8:
            channels = "7:1";
            break;
        default:
            char buf[128];
            sprintf( buf, "%d Channels", woc->maxOutputChannels );
            channels = buf;
            break;
        }

        _channels = 0;

        sprintf( desc, "%s (%s)",
                 woc->name, channels.c_str() );
        
        Device dev( name, desc );
        _devices.push_back( dev );
    }

}

bool PortAudioEngine::initialize()
{
    if ( _instances == 0 )
    {
      int err = Pa_Initialize();
      if ( err != paNoError )
	{
            PA_ERROR( err );
            return false;
	}
      outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
      if (outputParameters.device == paNoDevice) {
          PA_ERROR( err );
          return false;
      }
    }

    ++_instances;
    return true;
}


bool PortAudioEngine::shutdown()
{
    --_instances;

    close();

    if ( ! _instances )
      {
	Pa_Terminate();
      }

    return true;
}


float PortAudioEngine::volume() const
{
    if ( outputParameters.device == paNoDevice ) return 1.0f;
    return _volume;
}


void PortAudioEngine::volume( float v )
{
    if ( outputParameters.device == paNoDevice ) return;

    _volume = v;
}


bool PortAudioEngine::open( const unsigned channels,
                       const unsigned freq,
                       const AudioFormat format,
                       const unsigned bits )
{
    try
    {
        close();

	outputParameters.channelCount = channels;
	outputParameters.suggestedLatency = 0.050;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	unsigned bits = 32;
	switch( format )
        {
	case kFloatLSB:
	case kFloatMSB:
	  outputParameters.sampleFormat = paFloat32;
	  break;
	case kS32LSB:
	case kS32MSB:
	  outputParameters.sampleFormat = paInt32;
	  break;
	case kS16LSB:
	case kS16MSB:
	  bits = 16;
	  outputParameters.sampleFormat = paInt16;
	  break;
	case kU8:
	  bits = 8;
	  outputParameters.sampleFormat = paUInt8;
	  break;
        }
	
        size_t bytes = 1 * 1024;
	int err = Pa_OpenStream(
				&stream,
				NULL, /* no input */
				&outputParameters,
				freq,
				0, // FRAMES_PER_BUFFER
				paClipOff,      /* we won't output out of range samples so don't bother clipping them */
				NULL, /* no callback, use blocking API */
				NULL ); /* no callback, so no callback userData */
	if( err != paNoError )
            PA_ERROR( err );
	
        _audio_format = format;

        // Allocate internal sound buffer
        
	err = Pa_StartStream( stream );
        if( err != paNoError )
            PA_ERROR( err );
	
        DBGM1( "enabled ok" );
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

void PortAudioEngine::wait_audio()
{
  while ( ( outputParameters.device != paNoDevice ) && _enabled &&
	  _audio_playback && Pa_IsStreamActive( stream ) == 1 )
    {
      int milliseconds = 10;
#ifdef _WIN32
      Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
      struct timespec ts;
      ts.tv_sec = milliseconds / 1000;
      ts.tv_nsec = (milliseconds % 1000) * 1000000;
      nanosleep(&ts, NULL);
#else
      usleep(milliseconds * 1000);
#endif
    }
}

bool PortAudioEngine::play( const char* data, const size_t size )
{
    //wait_audio();

    if ( outputParameters.device == paNoDevice ) {
        return false;
    }
    if ( !_enabled ) {
        return true;
    }

    int err = Pa_WriteStream( stream, data, (unsigned)size );
    if( err != paNoError ) {
        PA_ERROR( err );
      _enabled = false;
      return false;
    }

    return true;
}



void PortAudioEngine::flush()
{
    if ( !stream ) return;

    int err = Pa_StopStream( stream );
    if( err != paNoError ) 
        PA_ERROR( err );
	
    _enabled = false;

}


bool PortAudioEngine::close()
{
    if (!stream) return false;

    flush();
    
    int err = Pa_CloseStream( stream );
    if( err != paNoError ) 
        PA_ERROR( err );

    _enabled = false;
    stream = NULL;
    return true;
}


} // namespace mrv


#endif
