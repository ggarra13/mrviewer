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
 * @file   mrvRtAudioEngine.h
 * @author gga
 * @date   Wed Jul 25 20:28:00 2007
 *
 * @brief  Linux/MacOS RtAudio engine
 *
 *
 */
#ifdef RT_AUDIO_ENGINE

#ifndef mrvRtAudioEngine_h
#define mrvRtAudioEngine_h

#include "core/mrvAudioEngine.h"
#include "RtAudio.h"



namespace mrv {

class RtAudioEngine : public mrv::AudioEngine
{
public:
    RtAudioEngine();
    virtual ~RtAudioEngine();

    // Name of audio engine
    virtual const char* name() {
        return "RtAudio driver";
    }

    virtual AudioFormat default_format();

    // Open an audio stream for playback
    virtual bool open(
        const unsigned int channels,
        const unsigned int frequency,
        const AudioFormat  format
    );

    virtual void refresh_devices();


    bool enabled() const { return _enabled; }

    // Play some samples (this function does not return until
    // playback has finished)
    virtual bool play( const char* data, const size_t size );

    virtual float volume() const;

    // Change volume of playback
    virtual void volume( float f );

    void getOutputBuffer( void* out, unsigned nFrames );

    bool stopped() const { return _stopped; }

    bool aborted() const { return _aborted; }

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
    RtAudio      audio;

    unsigned     sample_size;
    unsigned     _freq;
    unsigned     _channels;
    unsigned     _bits;


    bool         _stopped, _aborted;

    /* Our internal queue of samples waiting to be consumed by
       CoreAudio */
    unsigned char*               buffer;
    unsigned int                 bufferByteCount;
    unsigned int                 firstValidByteOffset;
    unsigned int                 validByteCount;

    unsigned int                 buffer_time;
    unsigned int                 totalBytesToCopy;
    unsigned int                 bufferSize;

protected:
    static std::atomic<unsigned int>     _instances;
};


} // namespace mrv


#endif // mrvRtAudioEngine_h

#endif // RT_AUDIO_ENGINE
