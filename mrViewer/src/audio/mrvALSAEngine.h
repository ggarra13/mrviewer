
/**
 * @file   mrvALSAEngine.h
 * @author gga
 * @date   Tue Jul 10 03:05:41 2007
 * 
 * @brief  Simple alsa sound engine class
 * 
 * 
 */

#ifndef mrvALSAEngine_h
#define mrvALSAEngine_h

#include "mrvAudioEngine.h"

typedef struct _snd_pcm   snd_pcm_t;
typedef struct _snd_mixer snd_mixer_t;

namespace mrv {

class ALSAEngine : public mrv::AudioEngine
{
public:
  ALSAEngine();
  virtual ~ALSAEngine();

  // Name of audio engine
  virtual const char* name() { return "ALSA"; }

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
  snd_pcm_t*   _pcm_handle;

protected:
  static unsigned int _instances;
  static snd_mixer_t* _mixer;
};


} // namespace mrv


#endif // mrvALSAEngine_h
