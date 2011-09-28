/**
 * @file   mrvWMMEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 * 
 * @brief  An Audio Engine using Windows' Multimedia (WMM) engine
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
    _device( NULL ),
    _format( NULL ),
    _options( NULL ),
    _sample_size(0),
    _audio_device(0)
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
	// ao_shutdown();
      }

    return true;
  }


  void AOEngine::volume( float v )
  {
    if (!_device) return;

  }

  bool AOEngine::open( const unsigned channels, 
		       const unsigned freq,
		       const AudioFormat format,
		       const unsigned bits )
  {
    try
      {
	flush();

	ao_sample_format fmt;
	fmt.bits = 16;
	fmt.rate = freq;
	fmt.channels = channels;
	fmt.byte_format = AO_FMT_LITTLE;
	
	_device = ao_open_live( _audio_device, &fmt, NULL );
	if ( _device == NULL )
	  {
	    _enabled = false;
	    LOG_ERROR("Error opening driver");
	    return false;
	  }


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


  bool AOEngine::play( const char* data, const size_t size )
  {

    if ( !_device) return false;
    if ( !_enabled ) return true;

    int ok = ao_play( _device, (char*)data, size );
    if ( ok == 0 )
      {
	LOG_ERROR("Error playing sample");
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
	    LOG_ERROR("Error closing ao device");
	  }

	_device = NULL;
	return true;
      }
    return false;
  }


} // namespace mrv
