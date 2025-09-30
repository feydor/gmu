#include "PortAudioSoundDriver.h"

#include <stdexcept>
#include <string.h>
#include <vector>
#include <algorithm>

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int portaudio_callback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData) {
    // extract data
    auto driver = (PortAudioSoundDriver*) userData;
    i16 *out = (i16*)outputBuffer;
    (void) inputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    // do buffer update, left + right samples
    i16 samples[2*framesPerBuffer] = {0};
    driver->load_samples(samples, 2*framesPerBuffer);
    for (unsigned i=1; i<2*framesPerBuffer; i+=2) {
        *out++ = samples[i-1];
        *out++ = samples[i];
    }

    return paContinue;
}

void PortAudioSoundDriver::print_devices_info() {
    if (!initialized)
        throw std::runtime_error("SoundDriver not initialized");

    int device = 0;
    char devname[] = "default";
    auto apicount = Pa_GetHostApiCount();
    for (int i=0; i<apicount; ++i) {
        auto api_info = Pa_GetHostApiInfo(i);
        printf("host_api_name=%s\n", api_info->name);
        printf("host_api_devcount=%d\n", api_info->deviceCount);
        auto default_devid = api_info->defaultOutputDevice;
        if (api_info->deviceCount > 0) {
            printf("printing devices...\n");
            for (int j=0; j<api_info->deviceCount; ++j) {
                auto device_info = Pa_GetDeviceInfo(j);
                if (strcmp(device_info->name, devname) == 0) {
                    printf("Found the device=%s!\n", devname);
                    device = j;
                }
                printf("Device %d: %s", j, device_info->name);
                if (default_devid == j)
                    printf(" [DEFAULT]");
                printf("\n");
            }
        }
        printf("\n");
    }
    fflush(stdout);

    PaStreamParameters outParams;
    outParams.device = device;
    outParams.sampleFormat = paInt16;
    outParams.channelCount = 2;
    outParams.hostApiSpecificStreamInfo = NULL;
    outParams.suggestedLatency = Pa_GetDeviceInfo(outParams.device)->defaultLowOutputLatency;
}

// TODO: Unused, use to matche certain devices?
static PaDeviceIndex get_pa_device_index() {
    std::vector<std::string> possible_device_names = {"pulse"};
    auto api_count = Pa_GetHostApiCount();
    for (int i=0; i<api_count; ++i) {
        auto api_info = Pa_GetHostApiInfo(i);
        auto device_count = api_info->deviceCount;
        for (int j=0; j<device_count; ++j) {
            const PaDeviceInfo* device_info = Pa_GetDeviceInfo(j);
            bool found = std::any_of(std::begin(possible_device_names), std::end(possible_device_names), [&](auto name){
                return name == device_info->name;
            });
            if (found)
                return j;
        }
    }

    printf("DID NOT FIND ANY matching device names!!!!");
    return 0;
}

PortAudioSoundDriver::PortAudioSoundDriver(std::function<void(i16 *, unsigned long)> samples_cb, long sample_rate)
        : samples_callback{samples_cb} {

    // calculate buffer size based on fill_rate and sample_rate
    int min_size = sample_rate * 2 / fill_rate;
    int frames_per_buf = 1024;
    while (frames_per_buf < min_size)
        frames_per_buf *= 2;

    // init portaudio
    handle_error(Pa_Initialize());
    initialized = true;

    //TODO: using default device for now
    PaStreamParameters outParams;
    outParams.device = Pa_GetDefaultOutputDevice();
    if (outParams.device == paNoDevice) {
        fprintf(stderr, "No default device found!\n");
        handle_error(-1);
    }
    outParams.sampleFormat = paInt16;
    outParams.channelCount = 2;
    outParams.hostApiSpecificStreamInfo = NULL;
    outParams.suggestedLatency = Pa_GetDeviceInfo(outParams.device)->defaultLowOutputLatency;

    // print_devices_info();

    auto err = Pa_OpenStream(&stream,
                        NULL,
                        &outParams,
                        sample_rate,
                        frames_per_buf, /* frames per buffer */
                        paClipOff,
                        portaudio_callback,
                        this);
    handle_error(err);
}

PortAudioSoundDriver::~PortAudioSoundDriver() {
    PaError err = Pa_StopStream(stream);
    if (err != paNoError) goto error;
    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;
error:
    Pa_Terminate();
}

bool PortAudioSoundDriver::stream_running() const {
    return running;
}

void PortAudioSoundDriver::load_samples(i16 *buf, unsigned long nframes) const {
    samples_callback(buf, nframes);
};

void PortAudioSoundDriver::start_audio() {
    handle_error(Pa_StartStream(stream));
    running = true;
}

void PortAudioSoundDriver::stop_audio() {
    handle_error(Pa_StopStream(stream));
    running = false;
}

void PortAudioSoundDriver::handle_error(PaError error) {
    if (error != paNoError) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        throw std::runtime_error(Pa_GetErrorText(error));
    }
}
