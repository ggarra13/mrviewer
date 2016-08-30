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
 * @file   mrvALSAEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 * 
 * @brief  An Audio Engine using ALSA for modern Linux
 * 
 * 
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>
using namespace std;

#include "audio/mrvALSAEngine.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"

#ifdef _WIN32
#undef fprintf
#endif

namespace mrv {

static const char* kModule = "alsa";

#define AO_ALSA_BUFFER_TIME 5000
/* number of samples between interrupts
 * supplying a period_time to ao overrides the use of this  */
#define AO_ALSA_SAMPLE_XFER 256


#define THROW(x) throw( exception(x) )

  unsigned int ALSAEngine::_instances = 0;
  snd_mixer_t* ALSAEngine::_mixer = NULL;

  ALSAEngine::ALSAEngine() :
    AudioEngine(),
    _sample_size(1),
    _pcm_handle(0)
  {
    initialize();
  }

  ALSAEngine::~ALSAEngine()
  {
    shutdown();
  }

  bool ALSAEngine::initialize()
  {

    if ( _devices.empty() )
      {
	// Create default device
	Device def( "default", "Default Audio Device" );
	_devices.push_back( def );

	// Now get all others
	int err = 0;
	int card = -1;
	if ( (err = snd_card_next(&card)) != 0 )
	  return false;

	while ( card > -1 )
	  {
	    char* psz_card_name = NULL;
	    snd_ctl_t*      p_ctl = NULL;
	    snd_pcm_info_t* pcm_info = NULL;
	    int pcm_device = -1;

	    char psz_dev[64];
	    sprintf(psz_dev, "hw:%i", card);

	    err = snd_ctl_open(&p_ctl, psz_dev, 0);

	    if ( err == 0 )
	      {
		err = snd_card_get_name(card, &psz_card_name);
		if ( err == 0 ) psz_card_name = (char*)"Unknown";

		snd_pcm_info_alloca(&pcm_info);

		for (;;)
		  {
		    err = snd_ctl_pcm_next_device(p_ctl, &pcm_device);
		    if (err < 0)
		      break;

		    if ( pcm_device < 0 )
		      break;

		    snd_pcm_info_set_device(pcm_info, pcm_device);
		    snd_pcm_info_set_subdevice(pcm_info, 0);
		    snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);

		    if ((err = snd_ctl_pcm_info(p_ctl, pcm_info)) < 0)
		      continue;

		    char psz_device[256], psz_descr[1024];
		    sprintf( psz_device, "hw:%d,%d", card, pcm_device );
		    sprintf( psz_descr, "%s: %s (%s)", psz_card_name,
			     snd_pcm_info_get_name(pcm_info), psz_device );

		    Device dev( psz_device, psz_descr );
		    _devices.push_back( dev );
		  }


	      }
          
            if ( p_ctl )
                snd_ctl_close( p_ctl );

	    err = snd_card_next(&card);
	    if ( err != 0 ) break;
	  }
      }
    ++_instances;
    return true;
  }


  bool ALSAEngine::shutdown()
  {
    --_instances;

    close();

    if ( _instances == 0 )
      {
          if ( _mixer ) 
              snd_mixer_close( _mixer );

          _mixer = NULL;
      }
    return true;
  }


  void ALSAEngine::volume( float v )
  {
#if 1

     _volume = v;

    //
    // This code is a modified version of similar code in amixer.
    //
    char buf[1024];

    if ( !_mixer )
      {
	int err;
	if ( (err = snd_mixer_open(&_mixer, 0)) < 0) {
	  sprintf( buf, _("Mixer %s open error: %s\n"), 
		   device().c_str(), snd_strerror(err) );
	  THROW(buf);
	}

	if ( (err = snd_mixer_attach( _mixer, device().c_str() )) < 0) {
	  sprintf(buf, _("Mixer attach %s error: %s"), 
		  device().c_str(), snd_strerror(err) );
	  snd_mixer_close( _mixer );
	  _mixer = NULL;
	  THROW(buf);
	}

	if ((err = snd_mixer_selem_register(_mixer, NULL, NULL)) < 0) {
	  sprintf( buf, _("Mixer register error: %s"), snd_strerror(err));
	  snd_mixer_close(_mixer);
	  _mixer = NULL;
	  THROW(buf);
	}

	err = snd_mixer_load( _mixer );
	if (err < 0) {
	  sprintf( buf, _("Mixer %s load error: %s"), 
		   device().c_str(), snd_strerror(err));
	  snd_mixer_close(_mixer);
	  _mixer = NULL;
	  THROW(buf);
	}
      }


    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_name( sid, "PCM" );

    snd_mixer_elem_t* elem = snd_mixer_find_selem( _mixer, sid);
    if ( !elem )
      {
	// Try with master        
	snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_name(sid, "Master");
	elem = snd_mixer_find_selem( _mixer, sid );
      }

    if ( !elem )
      {
	sprintf( buf, _("Unable to find simple control '%s', id: %i\n"),
		 snd_mixer_selem_id_get_name(sid), 
		 snd_mixer_selem_id_get_index(sid) );
	snd_mixer_close(_mixer);
	_mixer = NULL;
	THROW(buf);
      }

    long pmin, pmax;
    if ( snd_mixer_selem_get_playback_volume_range( elem, &pmin, &pmax ) < 0 )
      {
	sprintf( buf, _("Unable to find volume range '%s', id: %i\n"),
		 snd_mixer_selem_id_get_name(sid), 
		 snd_mixer_selem_id_get_index(sid) );
	snd_mixer_close(_mixer);
	_mixer = NULL;
	THROW(buf);
      }

    float orig_v= v;
    if ( v > 1.0f ) v = 1.0f;

    long smixer_level = pmin + long( float(pmax-pmin) * v);

    for (unsigned int i = 0; i <= SND_MIXER_SCHN_LAST; ++i) {
      snd_mixer_selem_channel_id_t chn = (snd_mixer_selem_channel_id_t) i;
      if ( ! snd_mixer_selem_has_playback_channel(elem, chn) )
	continue;
      snd_mixer_selem_set_playback_volume( elem, chn, smixer_level );
    }

#else
    int smixer_level = int(v * 31);
    char cmd[256];
    sprintf( cmd, N_("amixer -q set PCM %d"), smixer_level );
    system( cmd );
#endif
  }


  bool ALSAEngine::open( const unsigned channels, 
			 const unsigned freq,
			 const AudioFormat format,
			 const unsigned bits )
  {

    int                  status;
    unsigned int         test_format = (unsigned int) format;
    char buf[256];


    try
      {
	/* Open the audio device */
	/* Name of device should depend on # channels in spec */
          _pcm_handle = NULL;
          status = snd_pcm_open(&_pcm_handle, device().c_str(), 
                                SND_PCM_STREAM_PLAYBACK, 0);

          if ( status < 0 || _pcm_handle == NULL ) {
              sprintf( buf, "Couldn't open audio device: %s", 
                       snd_strerror(status));
              THROW(buf);
          }

          _old_device_idx = _device_idx;

          /* Figure out what the hardware is capable of */
          snd_pcm_hw_params_t *hwparams = NULL;

          // Allocate a hardware parameters object
          snd_pcm_hw_params_alloca(&hwparams);


          // Fill it with default values
          status = snd_pcm_hw_params_any(_pcm_handle, hwparams);
          if ( status < 0 ) {
              sprintf( buf, "Couldn't get hardware config: %s", 
                       snd_strerror(status));
              THROW(buf);
          }


          /* Interleaved mode */
          status = snd_pcm_hw_params_set_access(_pcm_handle, hwparams, 
                                                SND_PCM_ACCESS_RW_INTERLEAVED);
          if ( status < 0 ) {
              sprintf( buf, "Couldn't set access: %s", 
                       snd_strerror(status));
              THROW( buf );
          }


          /* Try for a closest match on audio format */
          status = -1;
          for ( ; test_format > 0; --test_format) {
              switch ( test_format ) {
                  case kU8:
                      _pcm_format = SND_PCM_FORMAT_U8;
                      break;
                  case kS16LSB:
                      _pcm_format = SND_PCM_FORMAT_S16_LE;
                      break;
                  case kS16MSB:
                      _pcm_format = SND_PCM_FORMAT_S16_BE;
                      break;
                  case kS32LSB:
                      _pcm_format = SND_PCM_FORMAT_U32_LE;
                      break;
                  case kS32MSB:
                      _pcm_format = SND_PCM_FORMAT_U32_BE;
                      break;
                  case kFloatLSB:
                      _pcm_format = SND_PCM_FORMAT_FLOAT_LE;
                      break;
                  case kFloatMSB:
                      _pcm_format = SND_PCM_FORMAT_FLOAT_BE;
                      break;
                  default:
                      _pcm_format = (snd_pcm_format_t) 0;
                      break;
              }
              if ( _pcm_format != 0 ) {
                  /* set the sample bitformat */
                  status = snd_pcm_hw_params_set_format(_pcm_handle, hwparams,
                                                        _pcm_format);
                  if ( status >= 0 ) break;
              }
          }
          if ( status < 0 ) {
              THROW( _("Couldn't find any hardware audio formats") );
          }

          _audio_format = (mrv::AudioEngine::AudioFormat) test_format;

          /* Set the number of channels */
          unsigned int ch = channels;
          status = snd_pcm_hw_params_set_channels(_pcm_handle, hwparams, ch);
          if ( status < 0 ) {
              status = snd_pcm_hw_params_get_channels(hwparams, &ch);
              if ( (status <= 0) || (status > 2) ) {
                  THROW( _("Couldn't set/get audio channels") );
              }
              status = snd_pcm_hw_params_set_channels(_pcm_handle, 
                                                      hwparams, ch);
              if ( status < 0 ) {
                  THROW( _("Couldn't set audio channels") );
              }
          }

          _channels = ch;
          _sample_size = ( bits * ch ) / 8;

          /* Set the audio rate */
          unsigned int exact_rate = freq;
          status = snd_pcm_hw_params_set_rate_near(_pcm_handle, hwparams, 
                                                   &exact_rate, NULL);
          if ( status < 0 ) {
              sprintf( buf, _("Couldn't set audio frequency: %s"), 
                       snd_strerror(status));
              THROW(buf);
          }

          if (freq != exact_rate) {
              LOG_WARNING( _("The rate ") << freq 
                           << _(" Hz is not supported by your hardware.") 
                           << std::endl
                           << _("  ==> Using ") << exact_rate << _(" Hz instead.") );
          }

          /* calculate a period time of one half sample time */
          unsigned int period_time = 1000000 * AO_ALSA_SAMPLE_XFER / exact_rate;

          /* set the time per hardware sample transfer */
          status = snd_pcm_hw_params_set_period_time_near(_pcm_handle,
                                                          hwparams,
                                                          &period_time,
                                                          0);
          if ( status < 0 )
	  {
              sprintf(buf, _("Couldn't get hw_params_set_period_time_near: %s"),
                      snd_strerror(status));
              THROW( buf );
	  }
  
          unsigned int buffer_time = AO_ALSA_BUFFER_TIME;
          status = snd_pcm_hw_params_set_buffer_time_near(_pcm_handle,
                                                          hwparams,
                                                          &buffer_time,
                                                          0);
          if ( status < 0 )
	  {
              sprintf(buf, _("Couldn't set buffer time near: %s"),
                      snd_strerror(status));
              THROW( buf );
	  }


          snd_pcm_uframes_t period_size;
          status = snd_pcm_hw_params_get_period_size(hwparams, &period_size, 0);
          if ( status < 0 ) {
              sprintf(buf, _("Couldn't get period size: %s"),
                      snd_strerror(status));
              THROW(buf);
          }

          /* "set" the hardware with the desired parameters */
          status = snd_pcm_hw_params(_pcm_handle, hwparams);
          if ( status < 0 ) {
              sprintf(buf, _("Couldn't set hardware audio parameters: %s"), 
                      snd_strerror(status));
              THROW(buf);
          }

          //snd_pcm_hw_params_free( hwparams );

          /* This is useful for debugging... */
          //   { 
          //     snd_pcm_sframes_t bufsize; int fragments;
          //     bufsize = snd_pcm_hw_params_get_period_size(hwparams);
          //     fragments = snd_pcm_hw_params_get_periods(hwparams);
    
          //     fprintf(stderr, "ALSA: bufsize = %ld, fragments = %d\n", bufsize, fragments);
          //   }


          /* Set the software parameters */
          snd_pcm_sw_params_t *swparams = NULL;
          snd_pcm_sw_params_alloca(&swparams);
          status = snd_pcm_sw_params_current(_pcm_handle, swparams);
          if ( status < 0 ) {
              sprintf( buf, _("Couldn't get software config: %s"),
                       snd_strerror(status));
              THROW(buf);
          }

          /* allow transfers to start when there are four periods */
          status = snd_pcm_sw_params_set_start_threshold(_pcm_handle, swparams, 
                                                         0);
          if ( status < 0 ) {
              sprintf( buf, _("Couldn't set start threshold: %s"), 
                       snd_strerror(status));
              THROW(buf);
          }

          status = snd_pcm_sw_params_set_avail_min(_pcm_handle, swparams, 
                                                   period_size);
          if ( status < 0 ) {
              sprintf( buf, _("Couldn't set avail min: %s"), snd_strerror(status));
              THROW(buf);
          }

          /* do not align transfers */

          /* commit the params structure to ALSA */
          status = snd_pcm_sw_params(_pcm_handle, swparams);
          if ( status < 0 ) {
              sprintf( buf, _("Couldn't set software audio parameters: %s"), 
                       snd_strerror(status));
              THROW(buf);
          }



          // All okay, enable device
          _enabled = true;

          /* We're ready to rock and roll. :-) */
          return true;
      }
    catch( const AudioEngine::exception& e )
    {
	close();
	_enabled = false;
    }
  }


  bool ALSAEngine::play( const char* data, const size_t size )
  {
    if ( !_pcm_handle || size == 0 || data == NULL ) return false;
    if ( !_enabled )   return true;

    long int           status = 0;


    unsigned sample_len = (unsigned)size / _sample_size;
    const char* sample_buf = data;

    while ( sample_len > 0 ) {
        status = snd_pcm_writei(_pcm_handle, sample_buf, sample_len);
        if ( status < 0 ) {
            if ( status == -EAGAIN ) {
                LOG_WARNING( "EAGAIN: " << snd_strerror( status ) );
                continue;
            }
            if ( status == -ESTRPIPE ) {
                /* application was suspended, wait until suspend flag clears */
                do {
                    status = snd_pcm_resume(_pcm_handle);
                } while ( status == -EAGAIN );
            }
            if ( status == -EPIPE ) {
                /* output buffer underrun */
                DBG( _("Buffer underrun: ") << snd_strerror( status ) );
                status = snd_pcm_prepare(_pcm_handle);
                if ( status < 0 )
                    LOG_ERROR( "snd_pcm_prepare failed" );
            }
            if ( status == -EBADFD )
            {
                LOG_ERROR( "EBADFD: " << snd_strerror( status ) );
                status = snd_pcm_prepare(_pcm_handle);
                if ( status < 0 )
                    LOG_ERROR( "snd_pcm_prepare failed" );
            } 
            if ( status < 0 ) {
                /* Hmm, not much we can do - abort */
                _enabled = false;
                return false;
            }
            continue;
        }
        sample_buf += status * _sample_size;
        sample_len -= (unsigned)status;
    }
    
    return true;
  }


  void ALSAEngine::flush()
  {
    if ( _pcm_handle ) {
      snd_pcm_drain( _pcm_handle ); 
    }
  }


  bool ALSAEngine::close()
  {
    if ( _pcm_handle )
      {
	snd_pcm_drain( _pcm_handle ); 
	snd_pcm_close( _pcm_handle );
	_pcm_handle = NULL;
      }
    return true;
  }

} // namespace mrv
