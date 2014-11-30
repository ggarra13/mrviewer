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
 * @file   mrvWaveEngine.h
 * @author gga
 * @date   Wed Jul 25 20:28:00 2007
 * 
 * @brief  MSwindows wave audio engine
 * 
 * 
 */
#ifndef mrvWaveEngine_h
#define mrvWaveEngine_h

#if !defined(WIN32) && !defined(WIN64)
#  error WaveEngine is for mswindows only
#endif


#include <windows.h>
#define IN
#define OUT
#define INOUT
#include <mmsystem.h>

#include "core/mrvAlignedData.h"
#include "core/mrvAudioEngine.h"


namespace mrv {

  class WaveEngine : public mrv::AudioEngine
  {
  public:
    WaveEngine();
    virtual ~WaveEngine();

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

    WAVEHDR* get_header();

    void free_headers();
    void wait_audio();

  protected:
    unsigned int _sample_size;
    HWAVEOUT     _audio_device;
    WAVEHDR*     _buffer;
    aligned16_uint8_t* _data;
    unsigned int _idx;
    unsigned int _samples_per_block;
    unsigned int bytesPerBlock;

  protected:
    static unsigned int     _instances;
  };


} // namespace mrv


#endif // mrvWaveEngine_h
