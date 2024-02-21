#include "PortAudioSoundDriver.h"

#include <stdexcept>

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int portaudio_callback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData)
{
    // extract data
    SoundDriver* driver = (SoundDriver*) userData;
    int16 *out = (int16*)outputBuffer;
    (void) inputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    // do buffer update, left + right samples
    short samples[2*framesPerBuffer];
    driver->samples_callback(samples, 2*framesPerBuffer);

    for(unsigned int i=1; i<2*framesPerBuffer; i+=2) {
        *out++ = samples[i-1];
        *out++ = samples[i];
    }
    return 0;
}

SoundDriver::SoundDriver(std::function<void(short *, unsigned long)> samples_cb, long sample_rate)
        : samples_callback{samples_cb}, sample_rate{sample_rate} {
    
    // calculate buffer size based on fill_rate and sample_rate
    int min_size = sample_rate * 2 / fill_rate;
	int buf_size = 512;
	while (buf_size < min_size)
		buf_size *= 2;

    // init portaudio
    //TODO: use default device for now
    handle_error(Pa_Initialize());
    auto err = Pa_OpenDefaultStream(&stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paInt16,    /* 16 bit digital output */
                                sample_rate,
                                buf_size, /* frames per buffer */
                                portaudio_callback,
                                this);
    handle_error(err);
}

SoundDriver::~SoundDriver() {
    PaError err = Pa_StopStream(stream);
    if (err != paNoError) goto error;
    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;
error:
    Pa_Terminate();
}

void SoundDriver::handle_error(PaError error) {
    if (error != paNoError) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        throw std::runtime_error(Pa_GetErrorText(error));
    }
}

void SoundDriver::start_audio() {
    handle_error(Pa_StartStream(stream));
}

void SoundDriver::stop_audio() {
    handle_error(Pa_StopStream(stream));
}

