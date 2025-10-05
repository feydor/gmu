#ifndef GMU_PORTAUDIO_SOUNDDRIVER_H
#define GMU_PORTAUDIO_SOUNDDRIVER_H

#include "SoundDriver.h"

#include <portaudio.h>

class PortAudioSoundDriver : public SoundDriver {
public:
    /** Given callback will be called whenever the driver needs samples. It should write signed 16 bit samples to the buffer. */
    PortAudioSoundDriver(std::function<bool(i16 *, unsigned long)> samples_cb, long sample_rate);
    ~PortAudioSoundDriver();
    void start_audio() override;
    void stop_audio() override;
    void handle_error(PaError error);
    /**
     * Invokes the samples_callback
     * @return true if playback should end
     * @see PortAudioSoundDriver()
     */
    bool load_samples(i16 *buf, unsigned long nframes) const override;
    bool stream_running() const override;
    void print_devices_info();
private:
    /**
     * Number of audio buffers per second. Adjust if you encounter audio skipping.
     * Note that this sets the floor on how often you'll see changes to the audio
     *scope
     */
    const int fill_rate = 40;
    std::function<bool(i16 *, unsigned long)> samples_callback;
    PaStream *stream = nullptr;
    bool initialized = false;
    bool running = false;
};

#endif
