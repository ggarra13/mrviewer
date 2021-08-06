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
 * @date   Tue Jul 10 03:05:41 2007
 *
 * @brief  Simple RtAudio sound engine class
 *
 *
 */


#ifndef mrvRtAudioEngine_h
#define mrvRtAudioEngine_h

#include "RtAudio.h"

#include "mrvAudioEngine.h"


namespace mrv {

class RtAudioEngine : public mrv::AudioEngine
{
public:
    RtAudioEngine();
    virtual ~RtAudioEngine();

    // Name of audio engine
    virtual const char* name() {
        return "RtAudio";
    }

    // Open an audio stream for playback
    virtual bool open(
        const unsigned int channels,
        const unsigned int frequency,
        const AudioFormat  format
    );

    // Play some samples (this function does not return until
    // playback has finished)
    virtual bool play( const char* data, const size_t size );

    // Retrieve current master volume
    virtual float volume() const;

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
    RtAudio  audio;
protected:
    static unsigned int _instances;
};


} // namespace mrv


#endif // mrvRtAudioEngine_h
