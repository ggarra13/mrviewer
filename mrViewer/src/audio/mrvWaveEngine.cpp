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
#include <cassert>

#include <iostream>

#include "gui/mrvIO.h"
#include "audio/mrvWaveEngine.h"

#include <mmreg.h>   // for manufacturer and product IDs


namespace 
{
  const char* kModule = "wmm";
}


namespace mrv {

#define kNUM_BUFFERS 5  // should be 2, was 32

#define THROW(x) throw( exception(x) )

  unsigned int     WaveEngine::_instances = 0;


  static void MMerror(char *function, MMRESULT code)
  {
    char errbuf[256];
    waveOutGetErrorText(code, errbuf, 255);
    LOG_ERROR( function << " - " << errbuf );
  }


  WaveEngine::WaveEngine() :
    AudioEngine(),
    _sample_size(0),
    _audio_device(0),
    _buffer( new WAVEHDR[ kNUM_BUFFERS ] ),
    _data( NULL ),
    _samples_per_block( 48000 ),  // 1 second of 48khz audio
    bytesPerBlock( 0 ),
    _idx( 0 )
  {
    initialize();
  }

  WaveEngine::~WaveEngine()
  {
    close();
    shutdown();
  }

  bool WaveEngine::initialize()
  {
    if ( _instances == 0 )
      {
	_devices.clear();

	Device def( "default", "Default Audio Device" );
	_devices.push_back( def );

	unsigned int num = waveOutGetNumDevs();

	char name[256];
	char desc[1024];

	for (unsigned i = 0; i < num; ++i )
	  {
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
		channels = "Dolby 5:1";
		break;
	      default:
		char buf[128];
		sprintf( buf, "%d Channels", woc.wChannels );
		channels = buf;
		break;
	      }

	    _channels = woc.wChannels;

	    sprintf( name, "%d", i );

	    std::string manufacturer;
	    switch( woc.wMid )
	      {
	      case MM_GRAVIS:
		manufacturer = "Advanced Gravis Computer Technology, Ltd.";
		break;
	      case MM_ANTEX:
		manufacturer = "Antex"; break;
	      case MM_APPS:
		manufacturer = "APPS"; break;
	      case MM_ARTISOFT:
		manufacturer = "Artisoft"; break;
	      case MM_AST:
		manufacturer = "AST Research, Inc."; break;
	      case MM_ATI:
		manufacturer = "ATI Technologies, Inc."; break;
	      case MM_AUDIOFILE:
		manufacturer = "Audio, Inc."; break;
	      case MM_APT:  // same as MM_AUDIOPT
	      case MM_AUDIOPT:
		manufacturer = "Audio Processing Technology"; break;
	      case MM_AURAVISION:
		manufacturer = "Auravision"; break;
	      case MM_AZTECH:
		manufacturer = "Aztech Labs, Inc."; break;
	      case MM_CANOPUS:
		manufacturer = "Canopus, Co., Ltd."; break;
	      case MM_COMPUSIC:
		manufacturer = "Compusic"; break;
	      case MM_CAT:
		manufacturer = "Computer Aided Technology, Inc."; break;
	      case MM_COMPUTER_FRIENDS:
		manufacturer = "Computer Friends, Inc."; break;
	      case MM_CONTROLRES:
		manufacturer = "Control Resources Corporation"; break;
	      case MM_CREATIVE:
		manufacturer = "Creative Labs, Inc."; break;
	      case MM_DIALOGIC:
		manufacturer = "Dialogic Corporation"; break;
	      case MM_DOLBY:
		manufacturer = "Dolby Laboratories"; break;
	      case MM_DSP_GROUP:
		manufacturer = "DSP Group, Inc."; break;
	      case MM_DSP_SOLUTIONS:
		manufacturer = "DSP Solutions, Inc."; break;
	      case MM_ECHO:
		manufacturer = "Echo Speech Corporation"; break;
	      case MM_ESS:
		manufacturer = "ESS Technology, Inc."; break;
	      case MM_EVEREX:
		manufacturer = "Everex Systems, Inc."; break;
	      case MM_EXAN:
		manufacturer = "EXAN, Ltd."; break;
	      case MM_FUJITSU:
		manufacturer = "Fujitsu, Ltd.."; break;
	      case MM_IOMAGIC:
		manufacturer = "I/O Magic Corporation"; break;
	      case MM_ICL_PS:
		manufacturer = "ICL Personal Systems"; break;
	      case MM_OLIVETTI:
		manufacturer = "Ing. C. Olivetti & C., S.p.A."; break;
	      case MM_ICS:
		manufacturer = "Integrated Circuit Systems, Inc."; break;
	      case MM_INTEL:
		manufacturer = "Intel Corporation"; break;
	      case MM_INTERACTIVE:
		manufacturer = "InterActive, Inc."; break;
	      case MM_IBM:
		manufacturer = "IBM"; break;
	      case MM_ITERATEDSYS:
		manufacturer = "Iterated Systems, Inc."; break;
	      case MM_LOGITECH:
		manufacturer = "Logitech, Inc."; break;
	      case MM_LYRRUS:
		manufacturer = "Lyrrus, Inc."; break;
	      case MM_MATSUSHITA:
		manufacturer = "Matsushita Electric Corporation of America"; 
		break;
	      case MM_MEDIAVISION:
		manufacturer = "Media Vision, Inc."; break;
	      case MM_METHEUS:
		manufacturer = "Metheus Corporation"; break;
	      case MM_MELABS:
		manufacturer = "microEngineering Labs"; break;
	      case MM_MICROSOFT:
		manufacturer = "Microsoft Corporation"; break;
	      case MM_MOSCOM:
		manufacturer = "MOSCOM Corporation"; break;
	      case MM_MOTOROLA:
		manufacturer = "Motorola, Inc."; break;
	      case MM_NMS:
		manufacturer = "Natural MicroSystems Corporation"; break;
	      case MM_NCR:
		manufacturer = "NCR Corporation"; break;
	      case MM_NEC:
		manufacturer = "NEC Corporation"; break;
	      case MM_NEWMEDIA:
		manufacturer = "New Media Corporation"; break;
	      case MM_OKI:
		manufacturer = "OKI"; break;
	      case MM_OPTI:
		manufacturer = "OPTi, Inc."; break;
	      case MM_ROLAND:
		manufacturer = "Roland"; break;
	      case MM_SCALACS:
		manufacturer = "SCALACS"; break;
	      case MM_EPSON:
		manufacturer = "Epson"; break;
	      case MM_SIERRA:
		manufacturer = "Sierra Semiconductor Corporation"; break;
	      case MM_SILICONSOFT:
		manufacturer = "Silicon Software"; break;
	      case MM_SONICFOUNDRY:
		manufacturer = "Sonic Foundry"; break;
	      case MM_SPEECHCOMP:
		manufacturer = "Speech Compression"; break;
	      case MM_SUPERMAC:
		manufacturer = "Supermac"; break;
	      case MM_TANDY:
		manufacturer = "Tandy"; break;
	      case MM_KORG:
		manufacturer = "Korg"; break;
	      case MM_TRUEVISION:
		manufacturer = "TrueVision"; break;
	      case MM_TURTLE_BEACH:
		manufacturer = "Turtle Beach Systems"; break;
	      case MM_VAL:
		manufacturer = "Video Associates Labs"; break;
	      case MM_VIDEOLOGIC:
		manufacturer = "VideoLogic"; break;
	      case MM_VITEC:
		manufacturer = "Visual Information Technologies"; break;
	      case MM_VOCALTEC:
		manufacturer = "VocalTec"; break;
	      case MM_VOYETRA:
		manufacturer = "Voyetra"; break;
	      case MM_WANGLABS:
		manufacturer = "Wang Labs"; break;
	      case MM_WILLOWPOND:
		manufacturer = "Willow Pond"; break;
	      case MM_WINNOV:
		manufacturer = "Winnov"; break;
	      case MM_XEBEC:
		manufacturer = "Xebec"; break;
	      case MM_YAMAHA:
		manufacturer = "Yamaha"; break;
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

	    Device dev( name, desc );
	    _devices.push_back( dev );
	  }

	if ( ! _devices.empty() ) _device_idx = 0;
	else                      _device_idx = WAVE_MAPPER;

      }

    ++_instances;
    return true;
  }


  bool WaveEngine::shutdown()
  {
    --_instances;

    close();

    delete [] _buffer;

    return true;
  }


  void WaveEngine::volume( float v )
  {
    if (!_audio_device) return;

    DWORD x = (DWORD) ( 0xFFFF * v );

    MMRESULT result = waveOutSetVolume( _audio_device, x );
    if ( result != MMSYSERR_NOERROR )
      {
	MMerror( "waveOutSetVolume", result );
	return;
      }
  }


  WAVEHDR* WaveEngine::get_header()
  {
    return &( _buffer[ _idx ] );
  }

  bool WaveEngine::open( const unsigned channels, 
			 const unsigned freq,
			 const AudioFormat format,
			 const unsigned bits )
  {
    try
      {
	flush();


	WAVEFORMATEX wavefmt;
	wavefmt.wFormatTag = WAVE_FORMAT_PCM;
	if ( channels > _channels ) wavefmt.nChannels = channels;
	else wavefmt.nChannels = channels;
	wavefmt.nSamplesPerSec = freq;
	wavefmt.wBitsPerSample = format <= kU8 ? 8 : 16;
	wavefmt.nBlockAlign = ( ( wavefmt.wBitsPerSample >> 3 ) * 
				wavefmt.nChannels );
	wavefmt.nAvgBytesPerSec = ( wavefmt.nSamplesPerSec * 
				    wavefmt.nBlockAlign );
	wavefmt.cbSize = 0;

	int device = _device_idx - 1;
	if ( device < 0 ) device = WAVE_MAPPER; // default device


	MMRESULT result = 
	  waveOutOpen(&_audio_device, WAVE_MAPPER, &wavefmt,
		      0, 0, CALLBACK_NULL|WAVE_ALLOWSYNC );

	if ( result != MMSYSERR_NOERROR )
	  {
	    close();
	    MMerror( "waveOutOpen", result );
	    _enabled = false;
	    return false;
	  }

	_audio_format = format;

	// Allocate internal sound buffer
	bytesPerBlock = wavefmt.nBlockAlign * _samples_per_block;
	unsigned int bytes = kNUM_BUFFERS * bytesPerBlock;
	_data = new aligned16_uint8_t[ bytes ];
	memset( _data, 0, bytes );

	// Set header memory
	char* ptr = (char*)_data;
	for ( unsigned i = 0; i < kNUM_BUFFERS; ++i )
	  {
	    WAVEHDR& hdr = _buffer[i];
	    memset( &hdr, 0, sizeof(WAVEHDR) );

	    assert( (((unsigned long)ptr) % wavefmt.nBlockAlign) == 0 );

	    hdr.lpData  = ptr;
	    hdr.dwFlags = WHDR_DONE;

	    ptr += bytesPerBlock;
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

  void WaveEngine::wait_audio()
  {
    while ( _audio_device && 
	    _enabled &&
	    ( ( _buffer[ _idx ].dwFlags & WHDR_DONE ) == 0 ) ) 
      {
	Sleep(20);
      }
  }

  bool WaveEngine::play( const char* data, const size_t size )
  {
    wait_audio();

    if ( !_audio_device) return false;
    if ( !_enabled ) return true;


    MMRESULT result;

    WAVEHDR* hdr = get_header();

    assert( size > 0 );
    assert( size < bytesPerBlock );
    assert( data != NULL );
    assert( hdr->lpData != NULL );

    // Copy data 
    memcpy( hdr->lpData, data, size );

    hdr->dwBufferLength = (DWORD)size;
    hdr->dwLoops        = 1;
    hdr->dwFlags       &= ~WHDR_DONE;

    _idx = ( _idx + 1 ) % kNUM_BUFFERS;

    result = waveOutPrepareHeader(_audio_device, hdr, sizeof(WAVEHDR));

    if ( result != MMSYSERR_NOERROR )
      {
	_enabled = false;
	MMerror( "waveOutPrepareHeader", result);
	return false;
      }

    result = waveOutWrite(_audio_device, hdr, sizeof(WAVEHDR));

    if ( result != MMSYSERR_NOERROR )
      {
	_enabled = false;
	MMerror("waveOutWrite", result);
	return false;
      }


    return true;
  }


  void WaveEngine::free_headers()
  {
    if (! _audio_device ) return;

    MMRESULT result;

    for ( unsigned i = 0; i < kNUM_BUFFERS; ++i )
      { 
	if ( _buffer[i].dwFlags & WHDR_PREPARED )
	  {
	    result = waveOutUnprepareHeader( _audio_device, &_buffer[i], 
					     sizeof(WAVEHDR) );
	    if ( result != MMSYSERR_NOERROR )
	      {
		_enabled = false;
		MMerror( "waveOutUnprepareHeader", result);
	      }
	    _buffer[i].dwFlags = WHDR_DONE;
	  }
      }

    delete [] _data;
  }

  void WaveEngine::flush()
  {
    if ( !_audio_device ) return;

    MMRESULT result = waveOutReset( _audio_device );
    if ( result != MMSYSERR_NOERROR )
      {
	_enabled = false;
	MMerror( "waveOutReset", result);
      }
    
    free_headers();
  }


  bool WaveEngine::close()
  {
    if (!_audio_device) return false;

    flush();

    MMRESULT result = waveOutClose( _audio_device );
    if ( result != MMSYSERR_NOERROR )
      {
	MMerror( "waveOutClose", result);
      }
    
    _enabled = false;
    _audio_device = NULL;
    return true;
  }


} // namespace mrv
