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
 * @file   mrvRtAudioEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using RtAudio.
 *
 *
 */

#ifdef RT_AUDIO_ENGINE

#include <cstdio>
#include <cstdlib>

#include <pthread.h>

#include "gui/mrvIO.h"
#include "audio/mrvRtAudioEngine.h"

namespace
{

const char* kModule = "rtaudio";

}

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

namespace mrv {

#define THROW(x) throw( exception(x) )


    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    std::atomic< unsigned int >    RtAudioEngine::_instances( 0 );

RtAudioEngine::RtAudioEngine() :
    AudioEngine(),
    _stopped( false ),
    _aborted( false )
{
    initialize();
}

RtAudioEngine::~RtAudioEngine()
{
    close();
    shutdown();
}

    void RtAudioEngine::refresh_devices()
    {
        // Create default device
        _devices.clear();
        _device_idx = -1;
        // Determine the number of devices available
        unsigned int devices = audio.getDeviceCount();
        // Scan through devices for various capabilities
        RtAudio::DeviceInfo info;
        for ( unsigned int i = 0; i < devices; ++i )
        {
            info = audio.getDeviceInfo( i );
            if ( info.probed == false || info.inputChannels ) continue;


            if ( info.isDefaultOutput )
            {
                _device = info;
                Device d( "default", info.name, i );
                _devices.insert( _devices.begin(), d );
            }
            else
            {
                char buf[64];
                sprintf( buf, "%d", i );

                Device d( buf, info.name, i );
                _devices.push_back( d );
            }
        }
    }

bool RtAudioEngine::initialize()
{
    refresh_devices();


    ++_instances;
    return true;
}


bool RtAudioEngine::shutdown()
{

    close();

    --_instances;

    if ( _instances == 0 )
    {
    }

    return true;
}


float RtAudioEngine::volume() const
{

    return _volume;
}

void RtAudioEngine::volume( float v )
{
    _volume = v;
}

    RtAudioEngine::AudioFormat RtAudioEngine::default_format()
    {
        // if ( _device.nativeFormats & RTAUDIO_FLOAT64 )
        //     return kDoubleLSB;
        if ( _device.nativeFormats & RTAUDIO_FLOAT32 )
            return kFloatLSB;
        if ( _device.nativeFormats & RTAUDIO_SINT32 )
            return kS32LSB;
        if ( _device.nativeFormats & RTAUDIO_SINT16 )
            return kS16LSB;
        if ( _device.nativeFormats & RTAUDIO_SINT8 )
            return kU8;
        else
            return kFloatLSB;
    }

    int callback( void *outputBuffer, void *inputBuffer,
                  unsigned int nFrames,
                  double streamTime,
                  RtAudioStreamStatus status,
                  void *userData )
    {
        RtAudioEngine* e = (RtAudioEngine*) userData;
        // if ( e->stopped() ) return 1;
        // if ( e->aborted() ) return 2;

        e->getOutputBuffer( outputBuffer, nFrames );

        // if ( status == RTAUDIO_OUTPUT_UNDERFLOW )
        //     LOG_WARNING( "output underflow" );
        return 0;
    }

    void errorCallback( RtAudioError::Type type,
                        const std::string &errorText )
    {
        LOG_ERROR( type << " " << errorText );
    }

    void RtAudioEngine::getOutputBuffer( void* out, unsigned nFrames )
    {
        pthread_mutex_lock(&mutex);

        unsigned int totalBytesToCopy = nFrames * sample_size;

        if (validByteCount < totalBytesToCopy && !(stopped() || aborted())) {
            /* Not enough data ... let it build up a bit more before we start
               copying stuff over. If we are stopping, of course, we should
               just copy whatever we have. This also happens if an application
               pauses output. */
            memset(out, 0, totalBytesToCopy);
            pthread_mutex_unlock(&mutex);
            return;
        }

        char *outBuffer = (char*)out;
        unsigned int outBufSize = totalBytesToCopy;
        unsigned int bytesToCopy = MIN(outBufSize, validByteCount);
        unsigned int firstFrag = bytesToCopy;
        unsigned char *sample = buffer + firstValidByteOffset;

        /* Check if we have a wrap around in the ring buffer If yes then
           find out how many bytes we have */
        if (firstValidByteOffset + bytesToCopy > bufferByteCount)
            firstFrag = bufferByteCount - firstValidByteOffset;

        /* If we have a wraparound first copy the remaining bytes off the end
           and then the rest from the beginning of the ringbuffer */
        if (firstFrag < bytesToCopy) {
            memcpy(outBuffer, sample, firstFrag);
            memcpy(outBuffer+firstFrag, buffer, bytesToCopy-firstFrag);
        } else {
            memcpy(outBuffer, sample, bytesToCopy);
        }
        if(bytesToCopy < outBufSize) /* the stopping case */
        {
            memset(outBuffer+bytesToCopy, 0, outBufSize-bytesToCopy);
        }

        validByteCount -= bytesToCopy;
        firstValidByteOffset = (firstValidByteOffset + bytesToCopy) %
                               bufferByteCount;


        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }

bool RtAudioEngine::open( const unsigned channels,
                          const unsigned samplerate,
                          const AudioFormat format )
{
    try
    {
        close();

        _enabled = false;

        unsigned freq = (( samplerate + 1 ) / 10 ) * 10;


        RtAudioFormat fmt;

        switch( format )
        {
        case kU8:
        {
            fmt = RTAUDIO_SINT8; _bits = 8; break;
        }
        case kS16LSB:
        {
            fmt = RTAUDIO_SINT16; _bits = 16; break;
        }
        case kS24LSB:
        {
            fmt = RTAUDIO_SINT24; _bits = 24; break;
        }
        case kS32LSB:
        {
            fmt = RTAUDIO_SINT32; _bits = 32; break;
        }
        case kFloatLSB:
        {
            fmt = RTAUDIO_FLOAT32; _bits = 32; break;
        }
        case kDoubleLSB:
        {
            fmt = RTAUDIO_FLOAT64; _bits = 64; break;
        }
        default:
        {
            LOG_ERROR( "Unknown audio format.  Setting float 32 bits" );
            fmt = RTAUDIO_FLOAT32; _bits = 32; break;
        }
        }

        bool ok = false;
        for (auto i : _device.sampleRates )
        {
            if ( i == freq ) ok = true;
        }
        if (! ok ) {
            std::string r;
            char buf[48];
            bool ok = false;
            for (auto i : _device.sampleRates )
            {
                sprintf( buf, " %d", i );
                r += buf;
            }
            LOG_ERROR( _("Audio frequency ") << freq <<  _(" is invalid.") );
            LOG_ERROR( _("Valid rates are:") << r );
            return true;
        }

        outputParameters.deviceId = device("default");
        outputParameters.nChannels = channels;
        outputParameters.firstChannel = 0;

        options.streamName = "mrViewer";

        unsigned int bufferFrames = 250;

        try
        {
            audio.openStream( &outputParameters, NULL, fmt, freq, &bufferFrames,
                              callback, this, &options, errorCallback );
        }
        catch( const RtAudioError& e )
        {
            _enabled = false;
            LOG_ERROR( freq << " " << e.getMessage() );
            return true;
        }

        buffer_time = 250;

        bufferByteCount =  (buffer_time * freq / 1000) *
                           (channels * _bits / 8);

        firstValidByteOffset = 0;
        validByteCount = 0;
        buffer = (unsigned char*)malloc(bufferByteCount);
        if (!buffer) {
            LOG_ERROR("Unable to allocate queue buffer.");
            return false;
        }
        memset(buffer, 0, bufferByteCount);

        // Store these for future round
        _audio_format = format;

        _channels = channels;
        _old_device_idx = _device_idx;

        sample_size = _channels * ( _bits / 8 );

        // All okay, enable device
        _enabled = true;

        audio.startStream();

        return true;
    }
    catch( const AudioEngine::exception& e )
    {
        close();
        _enabled = false;
        throw(e);
    }
}


bool RtAudioEngine::play( const char* d, const size_t size )
{
    if ( !_enabled ) return true;


    uint8_t* data = (uint8_t*)d;
    if ( _volume < 0.99f )
    {
        switch ( _audio_format )
        {
        case kU8:
        {
            data = new uint8_t[size];
            memcpy( data, d, size );
            for ( size_t i = 0; i < size; ++i )
            {
                data[i] *= _volume;
            }
            break;
        }
        case kFloatLSB:
        {

            size_t samples = size / 4;
            float* fdata = new float[samples];
            memcpy( fdata, d, size );
            for ( size_t i = 0; i < samples; ++i )
            {
                fdata[i] *= _volume;
            }
            data = (uint8_t*) fdata;
            break;
        }
        case kS32LSB:
        {
            size_t samples = size / 4;
            int32_t* s32data = new int32_t[samples];
            memcpy( s32data, d, size );
            for ( size_t i = 0; i < samples; ++i )
            {
                s32data[i] *= _volume;
            }
            data = (uint8_t*) s32data;
            break;
        }
        default:
        case kS16LSB:
        {
            size_t samples = size / 2;
            int16_t* s16data = new int16_t[samples];
            memcpy( data, d, size );
            for ( size_t i = 0; i < samples; ++i )
            {
                s16data[i] *= _volume;
            }
            data = (uint8_t*) s16data;
            break;
        }
        }
    }


    int err;
    unsigned int bytesToCopy;
    unsigned int firstEmptyByteOffset, emptyByteCount;
    uint32_t num_bytes = size;

    while (num_bytes) {
        // Get a consistent set of data about the available space in the queue,
        // figure out the maximum number of bytes we can copy in this chunk,
        // and claim that amount of space
        pthread_mutex_lock(&mutex);

        emptyByteCount = bufferByteCount - validByteCount;

        while (emptyByteCount == 0) {

            err = pthread_cond_wait(&cond, &mutex);
            if (err)
                LOG_ERROR("pthread_cond_wait() => " << err);

            emptyByteCount = bufferByteCount - validByteCount;
        }

        // Compute the offset to the first empty byte and the maximum number of
        // bytes we can copy given the fact that the empty space might wrap
        // around the end of the queue.
        firstEmptyByteOffset = (firstValidByteOffset + validByteCount) %
                               bufferByteCount;

        if (firstEmptyByteOffset + emptyByteCount > bufferByteCount)
            bytesToCopy = MIN(num_bytes,
                              bufferByteCount - firstEmptyByteOffset);
        else
            bytesToCopy = MIN(num_bytes, emptyByteCount);

        memcpy(buffer + firstEmptyByteOffset, data, bytesToCopy);

        num_bytes -= bytesToCopy;
        data += bytesToCopy;
        validByteCount += bytesToCopy;


        pthread_mutex_unlock(&mutex);
    }

    return true;
}

void RtAudioEngine::flush()
{
}


bool RtAudioEngine::close()
{
    if ( _enabled )
    {
        if ( ! audio.isStreamOpen() ) return false;

        try
        {
            audio.abortStream();
            _aborted = true;

            while ( audio.isStreamRunning() ) ;
        }
        catch( const RtAudioError& e )
        {
            LOG_ERROR( e.getMessage() );
        }

        audio.closeStream();

        _enabled = false;
        return true;
    }
    return false;
}


} // namespace mrv

#endif // RT_AUDIO_ENGINE
