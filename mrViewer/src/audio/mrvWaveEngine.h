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
