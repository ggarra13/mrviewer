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
 * @file   mrvAOEngine.h
 * @author gga
 * @date   Wed Jul 25 20:28:00 2007
 * 
 * @brief  MSwindows wave audio engine
 * 
 * 
 */
#ifndef mrvAOEngine_h
#define mrvAOEngine_h

#include "core/mrvAudioEngine.h"

struct ao_sample_format;
struct ao_device;
struct ao_option;  


namespace mrv {

  class AOEngine : public mrv::AudioEngine
  {
  public:
    AOEngine();
    virtual ~AOEngine();

    // Name of audio engine
    virtual const char* name() { return "Windows Multimedia"; }

    // Open an audio stream for playback
    virtual bool open( 
		      const unsigned int channels,
		      const unsigned int frequency,
		      const AudioFormat  format,
		      const unsigned int bits
		       );

    // Play some samples (this function does not return until
    // playback has finished)
    virtual bool play( const char* data, const size_t size );

    // Change volume of playback
    virtual void volume( float f );
  
    // Flush all audio sent for playback
    virtual void flush();

    // Close an audio stream
    virtual bool close();

  protected:
    // Shutdown the audio engine
    virtual bool shutdown();

    // Initialize the audio engine if needed and choose default
    // device for playback
    virtual bool initialize();


  protected:
    unsigned int _sample_size;
    unsigned int _audio_device;

    ao_sample_format* _format;
    ao_device*        _device;
    ao_option*        _options;

  protected:
    static unsigned int     _instances;
  };


} // namespace mrv


#endif // mrvAOEngine_h
