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
#    include "audio/mrvDirectXEngine.h"
#    include "audio/mrvWaveEngine.h"
#elif defined(LINUX)
#    include "audio/mrvALSAEngine.h"
#else
#    error Unknown audio engine for OS
#endif

namespace mrv {

AudioEngine::DeviceList AudioEngine::_devices;

AudioEngine::AudioEngine() :
_device_idx( 0 ),
_channels( 0 ),
_audio_format( kFloatLSB )
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
	 ffmpegformat = (AVSampleFormat) 0;
	 break;
   }
   return ffmpegformat;
}

std::string AudioEngine::default_device() const
{ 
   if ( _devices.empty() )
      EXCEPTION( "No audio device found" );

   return _devices.front().name; 
}


const AudioEngine::DeviceList& AudioEngine::devices()
{ 
   return _devices; 
}

std::string AudioEngine::device() const
{ 
   if ( _devices.empty() )
      EXCEPTION( "No audio device found" );

   return _devices[ _device_idx ].name; 
}

bool AudioEngine::device( const unsigned int idx )
{
   if ( idx >= _devices.size() ) return false;

   close();

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
