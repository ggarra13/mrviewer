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
 * @brief  An Audio Engine using Windows' Multimedia (WMM) engine
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
    SPEAKER_FRONT_CENTER,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT  | SPEAKER_LOW_FREQUENCY,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT     | SPEAKER_LOW_FREQUENCY,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_SIDE_LEFT    | SPEAKER_SIDE_RIGHT     | SPEAKER_BACK_CENTER  | SPEAKER_LOW_FREQUENCY,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_SIDE_LEFT    | SPEAKER_SIDE_RIGHT     | SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT | SPEAKER_LOW_FREQUENCY,
    SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_SIDE_LEFT    | SPEAKER_SIDE_RIGHT     | SPEAKER_BACK_LEFT  | SPEAKER_BACK_CENTER  | SPEAKER_BACK_RIGHT | SPEAKER_LOW_FREQUENCY
};


static void MMerror(char *function, int err)
{
    LOG_ERROR( function << " - " << err << " " << Pa_GetErrorText(err) );
}

#define PA_ERROR(x) MMerror( __FUNCTION__, x );


PortAudioEngine::PortAudioEngine() :
    AudioEngine(),
    _sample_size(0),
    stream( NULL ),
    _samples_per_block( 48000 ),  // 1 second of 48khz audio
    bytesPerBlock( 0 ),
    _audio_playback( false )
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

    unsigned int num = waveOutGetNumDevs();

    char name[256];
    char desc[1024];

    for (unsigned i = 0; i < num; ++i )
    {
        sprintf( name, "%d", i );

#ifdef _WIN32
        WAVEOUTCAPS woc;
        if ( waveOutGetDevCaps(i, &woc, sizeof(woc) ) !=
             MMSYSERR_NOERROR ) continue;

        std::string channels;
        switch( woc.wChannels )
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
            sprintf( buf, "%d Channels", woc.wChannels );
            channels = buf;
            break;
        }

        _channels = 0;

        std::string manufacturer;
        switch( woc.wMid )
        {
        case MM_GRAVIS:
            manufacturer = "Advanced Gravis Computer Technology, Ltd.";
            break;
        case MM_ANTEX:
            manufacturer = "Antex";
            break;
        case MM_APPS:
            manufacturer = "APPS";
            break;
        case MM_ARTISOFT:
            manufacturer = "Artisoft";
            break;
        case MM_AST:
            manufacturer = "AST Research, Inc.";
            break;
        case MM_ATI:
            manufacturer = "ATI Technologies, Inc.";
            break;
        case MM_AUDIOFILE:
            manufacturer = "Audio, Inc.";
            break;
        case MM_APT:  // same as MM_AUDIOPT
        case MM_AUDIOPT:
            manufacturer = "Audio Processing Technology";
            break;
        case MM_AURAVISION:
            manufacturer = "Auravision";
            break;
        case MM_AZTECH:
            manufacturer = "Aztech Labs, Inc.";
            break;
        case MM_CANOPUS:
            manufacturer = "Canopus, Co., Ltd.";
            break;
        case MM_COMPUSIC:
            manufacturer = "Compusic";
            break;
        case MM_CAT:
            manufacturer = "Computer Aided Technology, Inc.";
            break;
        case MM_COMPUTER_FRIENDS:
            manufacturer = "Computer Friends, Inc.";
            break;
        case MM_CONTROLRES:
            manufacturer = "Control Resources Corporation";
            break;
        case MM_CREATIVE:
            manufacturer = "Creative Labs, Inc.";
            break;
        case MM_DIALOGIC:
            manufacturer = "Dialogic Corporation";
            break;
        case MM_DOLBY:
            manufacturer = "Dolby Laboratories";
            break;
        case MM_DSP_GROUP:
            manufacturer = "DSP Group, Inc.";
            break;
        case MM_DSP_SOLUTIONS:
            manufacturer = "DSP Solutions, Inc.";
            break;
        case MM_ECHO:
            manufacturer = "Echo Speech Corporation";
            break;
        case MM_ESS:
            manufacturer = "ESS Technology, Inc.";
            break;
        case MM_EVEREX:
            manufacturer = "Everex Systems, Inc.";
            break;
        case MM_EXAN:
            manufacturer = "EXAN, Ltd.";
            break;
        case MM_FUJITSU:
            manufacturer = "Fujitsu, Ltd..";
            break;
        case MM_IOMAGIC:
            manufacturer = "I/O Magic Corporation";
            break;
        case MM_ICL_PS:
            manufacturer = "ICL Personal Systems";
            break;
        case MM_OLIVETTI:
            manufacturer = "Ing. C. Olivetti & C., S.p.A.";
            break;
        case MM_ICS:
            manufacturer = "Integrated Circuit Systems, Inc.";
            break;
        case MM_INTEL:
            manufacturer = "Intel Corporation";
            break;
        case MM_INTERACTIVE:
            manufacturer = "InterActive, Inc.";
            break;
        case MM_IBM:
            manufacturer = "IBM";
            break;
        case MM_ITERATEDSYS:
            manufacturer = "Iterated Systems, Inc.";
            break;
        case MM_LOGITECH:
            manufacturer = "Logitech, Inc.";
            break;
        case MM_LYRRUS:
            manufacturer = "Lyrrus, Inc.";
            break;
        case MM_MATSUSHITA:
            manufacturer = "Matsushita Electric Corporation of America";
            break;
        case MM_MEDIAVISION:
            manufacturer = "Media Vision, Inc.";
            break;
        case MM_METHEUS:
            manufacturer = "Metheus Corporation";
            break;
        case MM_MELABS:
            manufacturer = "microEngineering Labs";
            break;
        case MM_MICROSOFT:
            manufacturer = "Microsoft Corporation";
            break;
        case MM_MOSCOM:
            manufacturer = "MOSCOM Corporation";
            break;
        case MM_MOTOROLA:
            manufacturer = "Motorola, Inc.";
            break;
        case MM_NMS:
            manufacturer = "Natural MicroSystems Corporation";
            break;
        case MM_NCR:
            manufacturer = "NCR Corporation";
            break;
        case MM_NEC:
            manufacturer = "NEC Corporation";
            break;
        case MM_NEWMEDIA:
            manufacturer = "New Media Corporation";
            break;
        case MM_OKI:
            manufacturer = "OKI";
            break;
        case MM_OPTI:
            manufacturer = "OPTi, Inc.";
            break;
        case MM_ROLAND:
            manufacturer = "Roland";
            break;
        case MM_SCALACS:
            manufacturer = "SCALACS";
            break;
        case MM_EPSON:
            manufacturer = "Epson";
            break;
        case MM_SIERRA:
            manufacturer = "Sierra Semiconductor Corporation";
            break;
        case MM_SILICONSOFT:
            manufacturer = "Silicon Software";
            break;
        case MM_SONICFOUNDRY:
            manufacturer = "Sonic Foundry";
            break;
        case MM_SPEECHCOMP:
            manufacturer = "Speech Compression";
            break;
        case MM_SUPERMAC:
            manufacturer = "Supermac";
            break;
        case MM_TANDY:
            manufacturer = "Tandy";
            break;
        case MM_KORG:
            manufacturer = "Korg";
            break;
        case MM_TRUEVISION:
            manufacturer = "TrueVision";
            break;
        case MM_TURTLE_BEACH:
            manufacturer = "Turtle Beach Systems";
            break;
        case MM_VAL:
            manufacturer = "Video Associates Labs";
            break;
        case MM_VIDEOLOGIC:
            manufacturer = "VideoLogic";
            break;
        case MM_VITEC:
            manufacturer = "Visual Information Technologies";
            break;
        case MM_VOCALTEC:
            manufacturer = "VocalTec";
            break;
        case MM_VOYETRA:
            manufacturer = "Voyetra";
            break;
        case MM_WANGLABS:
            manufacturer = "Wang Labs";
            break;
        case MM_WILLOWPOND:
            manufacturer = "Willow Pond";
            break;
        case MM_WINNOV:
            manufacturer = "Winnov";
            break;
        case MM_XEBEC:
            manufacturer = "Xebec";
            break;
        case MM_YAMAHA:
            manufacturer = "Yamaha";
            break;
        default:
            char buf[64];
            sprintf( buf, "Manufacturer: %d", woc.wMid );
            manufacturer = buf;
            break;
        }

        std::string product;
        char buf[64];
        sprintf( buf, "Product: %d", woc.wPid );
        product = buf;

        sprintf( desc, "%s (%s) - %s %s",
                 woc.szPname, channels.c_str(),
                 manufacturer.c_str(), product.c_str() );
#else
        sprintf( desc, "Board #%d", i );
#endif
        
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
                outputParameters.sampleFormat = paInt16;
                break;
            case kU8:
                outputParameters.sampleFormat = paUInt8;
                break;
        }
	
	int err = Pa_OpenStream(
				&stream,
				NULL, /* no input */
				&outputParameters,
				freq,
				1024, // FRAMES_PER_BUFFER
				0,      /* we won't output out of range samples so don't bother clipping them */
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
	  _audio_playback )
    {
        Sleep(10);
    }
}

bool PortAudioEngine::play( const char* data, const size_t size )
{
    wait_audio();

    if ( outputParameters.device == paNoDevice ) {
        return false;
    }
    if ( !_enabled ) {
        return true;
    }

    
    _audio_playback = true;
    
    int err = Pa_WriteStream( stream, data, (unsigned)size );
    if( err != paNoError ) {
        PA_ERROR( err );
      _enabled = false;
      return false;
    }
    
    _audio_playback = false;

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
