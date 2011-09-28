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
