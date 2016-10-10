/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

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
 * @file   mrvAudioEngine.cpp
 * @author gga
 * @date   Fri Jul 20 00:39:45 2007
 * 
 * @brief  
 * 
 * 
 */


#include <iostream>

#include "mrvException.h"
#include "mrvAudioEngine.h"

#if defined(WIN32) || defined(WIN64)
#    include "audio/mrvWaveEngine.h"
#elif defined(LINUX)
#    include "audio/mrvALSAEngine.h"
#else
#    error Unknown audio engine for OS
#endif

namespace mrv {

AudioEngine::DeviceList AudioEngine::_devices;
unsigned  AudioEngine::_device_idx = 0;
unsigned  AudioEngine::_old_device_idx = 99999;

AudioEngine::AudioEngine() :
_enabled( false ),
_volume( 1.0f ),
_channels( 0 ),
_audio_format( kS16LSB )
{
}

AudioEngine::~AudioEngine()
{
}


AVSampleFormat AudioEngine::ffmpeg_format( const AudioFormat f )
{
   AVSampleFormat ffmpegformat;
   switch( f )
   {
      case kU8:
	 ffmpegformat = AV_SAMPLE_FMT_U8;
	 break;
      case kS16LSB:
      case kS16MSB:
	 ffmpegformat = AV_SAMPLE_FMT_S16;
	 break;
      case kS32LSB:
      case kS32MSB:
	 ffmpegformat = AV_SAMPLE_FMT_S32;
	 break;
      case kFloatLSB:
      case kFloatMSB:
	 ffmpegformat = AV_SAMPLE_FMT_FLT;
	 break;
      case kDoubleLSB:
      case kDoubleMSB:
	 ffmpegformat = AV_SAMPLE_FMT_DBL;
	 break;
      default:
	 ffmpegformat = AV_SAMPLE_FMT_NONE;
	 break;
   }
   return ffmpegformat;
}

std::string AudioEngine::default_device()
{
   if ( _devices.empty() )
      EXCEPTION( "No audio device found" );

   return _devices.front().name; 
}


const AudioEngine::DeviceList& AudioEngine::devices()
{ 
   return _devices; 
}

std::string AudioEngine::device()
{ 
   if ( _devices.empty() )
      EXCEPTION( "No audio device found" );

   return _devices[ _device_idx ].name; 
}

bool AudioEngine::device( const unsigned int idx )
{
   if ( idx >= _devices.size() ) return false;

   _device_idx = idx;

   return true;
}

bool AudioEngine::device( const std::string& d )
{
   DeviceList::iterator i = _devices.begin();
   DeviceList::iterator e = _devices.end();

   unsigned idx = 0;
   for ( ; i != e; ++i, ++idx )
   {
      if ( (*i).name == d ) break;
   }

   return device( idx );
}
 

AudioEngine* AudioEngine::factory()
{
   AudioEngine* r;

#if defined(_WIN32) || defined(_WIN64)
   //r = new mrv::DirectXEngine();
   r = new mrv::WaveEngine();
#elif defined(LINUX) 
   r = new mrv::ALSAEngine();
#endif

   return r;
}


} // namespace mrv
