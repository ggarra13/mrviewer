/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @file   mrvPortAudioEngine.cpp
 * @author gga
 * @date   Tue Jul 10 03:26:02 2007
 *
 * @brief  An Audio Engine using PortAudio engine
 *
 *
 */


#ifdef PORTAUDIO

#include <cstdio>
#include <cstdlib>
#include <pthread.h>



#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "audio/mrvPortAudioEngine.h"

#ifdef _WIN32
#include <mmreg.h>   // for manufacturer and product IDs
#endif

namespace
{
const char* kModule = "paudio";
}

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

namespace mrv {

#define THROW(x) throw( exception(x) )



unsigned int     PortAudioEngine::_instances = 0;

/* Microsoft speaker definitions */
#define WAVE_SPEAKER_FRONT_LEFT             0x1
#define WAVE_SPEAKER_FRONT_RIGHT            0x2
#define WAVE_SPEAKER_FRONT_CENTER           0x4
#define WAVE_SPEAKER_LOW_FREQUENCY          0x8
#define WAVE_SPEAKER_BACK_LEFT              0x10
#define WAVE_SPEAKER_BACK_RIGHT             0x20
#define WAVE_SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define WAVE_SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define WAVE_SPEAKER_BACK_CENTER            0x100
#define WAVE_SPEAKER_SIDE_LEFT              0x200
#define WAVE_SPEAKER_SIDE_RIGHT             0x400
#define WAVE_SPEAKER_TOP_CENTER             0x800
#define WAVE_SPEAKER_TOP_FRONT_LEFT         0x1000
#define WAVE_SPEAKER_TOP_FRONT_CENTER       0x2000
#define WAVE_SPEAKER_TOP_FRONT_RIGHT        0x4000
#define WAVE_SPEAKER_TOP_BACK_LEFT          0x8000
#define WAVE_SPEAKER_TOP_BACK_CENTER        0x10000
#define WAVE_SPEAKER_TOP_BACK_RIGHT         0x20000
#define WAVE_SPEAKER_RESERVED               0x80000000

static const int channel_mask[] = {
    WAVE_SPEAKER_FRONT_CENTER,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_BACK_LEFT    | WAVE_SPEAKER_BACK_RIGHT     | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_CENTER  | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_LEFT  | WAVE_SPEAKER_BACK_RIGHT | WAVE_SPEAKER_LOW_FREQUENCY,
    WAVE_SPEAKER_FRONT_LEFT   | WAVE_SPEAKER_FRONT_CENTER | WAVE_SPEAKER_FRONT_RIGHT  | WAVE_SPEAKER_SIDE_LEFT    | WAVE_SPEAKER_SIDE_RIGHT     | WAVE_SPEAKER_BACK_LEFT  | WAVE_SPEAKER_BACK_CENTER  | WAVE_SPEAKER_BACK_RIGHT | WAVE_SPEAKER_LOW_FREQUENCY
};


static void MMerror(const char *function, unsigned line, int err)
{
    LOG_ERROR( function << " (" << line << ") - "
               << err << " " << Pa_GetErrorText(err) );
}

#define PA_ERROR(x) MMerror( __FUNCTION__, __LINE__, x );


PortAudioEngine::PortAudioEngine() :
    AudioEngine(),
    sample_size(0),
    stream( NULL ),
    _stopped( false ),
    _aborted( false )
{
    initialize();
}

PortAudioEngine::~PortAudioEngine()
{
    shutdown();
}

void PortAudioEngine::refresh_devices()
{
    _devices.clear();

    _device_idx = -1;

    unsigned int num = Pa_GetDeviceCount();

    char name[256];
    char desc[1024];

    for (unsigned i = 0; i < num; ++i )
    {
        const PaDeviceInfo* woc;
        woc = Pa_GetDeviceInfo( i );


        if ( woc->maxOutputChannels <= 0 ) continue;


        std::string channels;
        switch( woc->maxOutputChannels )
        {
        case 1:
            channels = "Mono";
        case 2:
            channels = "Stereo";
            break;
        case 6:
            channels = "5:1";
            break;
        case 8:
            channels = "7:1";
            break;
        default:
            char buf[128];
            sprintf( buf, "%d Channels", woc->maxOutputChannels );
            channels = buf;
            break;
        }

        if ( _device_idx < 0 ) {
            _device_idx = i;
            sprintf( name, "default" );
        }
        else
            sprintf( name, "Card #%d", i );

        sprintf( desc, "%s (%s)",
                 woc->name, channels.c_str() );

        Device dev( name, desc, i );
        _devices.push_back( dev );
    }

}

bool PortAudioEngine::initialize()
{
    int err;
    if ( _instances == 0 )
    {
      err = Pa_Initialize();
      if ( err != paNoError )
        {
            PA_ERROR( err );
            return false;
        }

      refresh_devices();
    }

    outputParameters.device = device("default");
    _channels = _freq = 0;

    ++_instances;
    return true;
}


bool PortAudioEngine::shutdown()
{
    --_instances;

    close();

    if ( ! _instances )
      {
        Pa_Terminate();
      }

    return true;
}


float PortAudioEngine::volume() const
{
    if ( outputParameters.device == paNoDevice ) return 1.0f;
    return _volume;
}


void PortAudioEngine::volume( float v )
{
    if ( outputParameters.device == paNoDevice ) return;

    _volume = v;
}


int
callback( const void *inputBuffer, void *outputBuffer,
          unsigned long nFrames,
          const PaStreamCallbackTimeInfo* streamTime,
          PaStreamCallbackFlags status,
          void *userData )
{
    PortAudioEngine* e = (PortAudioEngine*) userData;


    e->getOutputBuffer( outputBuffer, nFrames );

    if ( status & paOutputUnderflow )
         LOG_WARNING( "output underflow" );
    if ( status & paOutputOverflow )
         LOG_WARNING( "output overflow" );
    return paContinue;
}

bool PortAudioEngine::open( const unsigned channels,
                            const unsigned freq,
                            const AudioFormat format )
{
    try
    {
        if ( _freq != freq || _channels != channels )
            close();

        if ( stream && Pa_IsStreamActive( stream ) )
            return true;

        const PaDeviceInfo* info = Pa_GetDeviceInfo( _device_idx );
        if ( info == nullptr )
        {
            LOG_ERROR( _("Could not get device info for ") << _device_idx );
        }


        outputParameters.channelCount = channels;
        outputParameters.suggestedLatency = info->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;

        _bits = 32;
        switch( format )
        {
        case kFloatLSB:
        case kFloatMSB:
          outputParameters.sampleFormat = paFloat32;
          break;
        case kS32LSB:
        case kS32MSB:
          outputParameters.sampleFormat = paInt32;
          break;
        case kS24LSB:
        case kS24MSB:
          _bits = 24;
          outputParameters.sampleFormat = paInt24;
        case kS16LSB:
        case kS16MSB:
          _bits = 16;
          outputParameters.sampleFormat = paInt16;
          break;
        case kU8:
          _bits = 8;
          outputParameters.sampleFormat = paUInt8;
          break;
        default:
            LOG_ERROR( _("Audio format not known.  Choosing float32. " ) );
            outputParameters.sampleFormat = paFloat32;
        }

        int err = Pa_OpenStream(
                                &stream,
                                NULL, /* no input */
                                &outputParameters,
                                freq,
                                250,
                                paClipOff,  /* we don't output out of range samples so no need to clip them */
                                callback, /* callback, use non-blocking API */
                                this ); /* userData */
        if( err != paNoError )
            PA_ERROR( err );



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

        _audio_format = format;

        _channels = channels;
        _old_device_idx = _device_idx;
        _freq = freq;

        sample_size = _channels * ( _bits / 8 );

        // Allocate internal sound buffer
        // std::cerr << "open stream " << stream << std::endl;

        err = Pa_StartStream( stream );
        if( err != paNoError )
            PA_ERROR( err );

        DBGM1( "enabled ok" );
        // All okay, enable device
        _aborted = _stopped = false;
        _enabled = true;
        return true;
    }
    catch( const AudioEngine::exception& e )
    {
        close();
        _enabled = false;
        throw(e);
    }
}

void
PortAudioEngine::getOutputBuffer( void* out, unsigned long nFrames )
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


bool PortAudioEngine::play( const char* d, const size_t size )
{
    if ( outputParameters.device == paNoDevice ) {
        return false;
    }
    if ( !_enabled ) {
        return true;
    }

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



void PortAudioEngine::flush()
{
    if ( !stream ) return;



    int err = Pa_AbortStream( stream );
    if( err != paNoError )
        PA_ERROR( err );


    _enabled = false;

}


bool PortAudioEngine::close()
{
    if (!stream) return false;

    flush();


    int err = Pa_CloseStream( stream );
    if( err != paNoError )
        PA_ERROR( err );

    stream = NULL;
    _enabled = false;
    return true;
}


} // namespace mrv


#endif
