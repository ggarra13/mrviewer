/**
 * @file   mrvAudio.h
 * @author gga
 * @date   Fri Jul 20 00:28:19 2007
 * 
 * @brief  An abstract class used for playing audio.
 * 
 * 
 */

#ifndef mrvAudioEngine_h
#define mrvAudioEngine_h

extern "C" {
#include <libavutil/samplefmt.h>
}
#include <vector>
#include <string>


namespace mrv {

  class AudioEngine
  {
  public:

    // Storage for a device
    struct Device
    {
      std::string name;
      std::string description;

      Device( const std::string& n,
	      const std::string& desc ) :
	name( n ),
	description( desc )
      {
      }

      Device( const Device& b ) :
	name( b.name ),
	description( b.description )
      {
      }
    };


    typedef std::vector< Device > DeviceList;

    enum AudioFormat
      {
      kU8 = 1,
      kS16LSB,
      kS16MSB,
      kS32LSB,
      kS32MSB,
      kFloatLSB,
      kFloatMSB,
      kDoubleLSB,
      kDoubleMSB,
      kLastPCMFormat
      };

    struct exception : public std::exception
    {
      exception( const char* wh ) : _what( wh )
      {
      }

      virtual ~exception() throw() {}

      virtual const char* what() const throw()
      {
	return _what.c_str();
      }

    private:
      std::string _what;
    };


    AudioEngine();
    virtual ~AudioEngine();

    // List devices available for playback on machine
    static const DeviceList& devices();

    // Return default device (must be first one added)
    std::string default_device() const;

    // Allow the user to select a device for playback
    bool device( const std::string& b );

    bool device( const unsigned int idx );

    std::string device() const;

    // Name of audio engine
    virtual const char* name() = 0;

    // Open an audio stream for playback
    virtual bool open( 
		      const unsigned int channels,
		      const unsigned int frequency,
		      const AudioFormat  format = kS16LSB,
		      const unsigned int bits = 16
		       ) = 0;

    // Play some samples (this function does not return until
    // playback has finished)
    virtual bool play( const char* data, const size_t size ) = 0;

    // Change volume
    virtual void volume( float f ) = 0;

    // Flush all audio sent for playback
    virtual void flush() = 0;

    // Close an audio stream
    virtual bool close() = 0;

    void enable()        { _enabled = true; }
    bool enabled() const { return _enabled; }
    void disable()       { _enabled = false; }

    unsigned channels() const { return _channels; }

    AudioFormat format() const { return _audio_format; }

    static AVSampleFormat ffmpeg_format( const AudioFormat f );

    // Create an appropriate audio engine for this OS.
    static AudioEngine* factory();

  protected:
    // Release the audio engine
    virtual bool shutdown() = 0;

    // Initialize the audio engine if needed and choose default
    // device for playback
    virtual bool initialize() = 0;


  protected:
    static DeviceList _devices;     //!< list of devices available
    unsigned int _device_idx;  //!< index to current device being used
    bool         _enabled;
    float        _volume;
    unsigned int _channels;
    AudioFormat  _audio_format;
  };

}  // namespace mrv


#endif // mrvAudioEngine
