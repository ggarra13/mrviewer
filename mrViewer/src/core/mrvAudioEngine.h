/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
#include <atomic>


class ViewerUI;

namespace mrv {

class AudioEngine
{
public:

    // Storage for a device
    struct Device
    {
        std::string name;
        std::string description;

        Device( const std::string n,
                const std::string desc ) :
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
        kNoAudioFormat = 0,
        kU8 = 1,
        kS16LSB,
        kS16MSB,
        kS24LSB,
        kS24MSB,
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

    virtual void refresh_devices() {};

    // List devices available for playback on machine
    static const DeviceList& devices();

    // Return default device (must be first one added)
    static std::string default_device();

    // Allow the user to select a device for playback
    static bool device( const std::string& b );

    static bool device( const unsigned int idx );

    static std::string device();

    static unsigned device_index() {
        return _device_idx;
    }
    static unsigned old_device_index() {
        return _old_device_idx;
    }

    // Name of audio engine
    virtual const char* name() = 0;


    virtual void buffers( int num ) {}

    // Open an audio stream for playback
    virtual bool open(
        const unsigned int channels,
        const unsigned int frequency,
        const AudioFormat  format = kFloatLSB,
        const unsigned int bits = 32
    ) = 0;

    // Play some samples (this function does not return until
    // playback has finished)
    virtual bool play( const char* data, const size_t size ) = 0;

    // Return current master volume
    virtual float volume() const = 0;

    // Change volume
    virtual void volume( float f ) = 0;

    // Flush all audio sent for playback
    virtual void flush() = 0;

    // Close an audio stream
    virtual bool close() = 0;

    void enable()        {
        _enabled = true;
    }
    bool enabled() const {
        return _enabled;
    }
    void disable()       {
        _enabled = false;
    }

    inline  unsigned channels() const {
        return _channels;
    }

    inline AudioFormat format() const {
        return _audio_format;
    }

    virtual AudioFormat default_format();

    static unsigned short bits_for_format( const AudioFormat f );

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
    static unsigned int _device_idx;  //!< index to current device being used
    static unsigned int _old_device_idx;  //!< index to previous device used
    bool         _enabled;
    std::atomic<float>        _volume;
    std::atomic<unsigned int> _channels;
    std::atomic<AudioFormat>  _audio_format;
};

}  // namespace mrv


#endif // mrvAudioEngine
