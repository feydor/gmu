#ifndef GMU_PORTAUDIO_SOUNDDRIVER_H
#define GMU_PORTAUDIO_SOUNDDRIVER_H

#include "SoundDriver.h"

#include <portaudio.h>

class PortAudioSoundDriver : public SoundDriver {
public:
    PortAudioSoundDriver(std::function<void(short *, unsigned long)> samples_cb, long sample_rate);
    ~PortAudioSoundDriver();
    void start_audio() override;
    void stop_audio() override;
    void handle_error(PaError error);
    virtual void load_samples(short *buf, unsigned long nframes) const override;

private:
    /**
     * Number of audio buffers per second. Adjust if you encounter audio skipping.
     * Note that this sets the floor on how often you'll see changes to the audio
     *scope
     */
    const int fill_rate = 80;
    std::function<void(short *, unsigned long)> samples_callback;
    PaStream *stream = nullptr;
};

#endif
