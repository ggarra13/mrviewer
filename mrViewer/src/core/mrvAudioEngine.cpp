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

#ifdef HAVE_LIB_AO
#  include "audio/mrvAOEngine.h"
#else
#  if defined(WIN32) || defined(WIN64)
#    include "audio/mrvWaveEngine.h"
#  elif defined(LINUX)
#    include "audio/mrvALSAEngine.h"
#  else
#    error Unknown audio engine for OS
#  endif
#endif

namespace mrv {

  AudioEngine::DeviceList AudioEngine::_devices;

  AudioEngine::AudioEngine() :
    _device_idx( 0 )
  {
  }

  AudioEngine::~AudioEngine()
  {
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

#ifdef HAVE_LIB_AO
    r = new mrv::AOEngine();
#else
  #if defined(WIN32) || defined(WIN64)
    r = new mrv::WaveEngine();
  #elif defined(LINUX) 
    r = new mrv::ALSAEngine();
  #endif
#endif

    return r;
  }


void AudioEngine::convert_surround_6( const char* data, const size_t size )
{
}

} // namespace mrv
