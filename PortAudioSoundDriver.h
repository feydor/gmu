#ifndef GMU_PORTAUDIO_SOUNDDRIVER_H
#define GMU_PORTAUDIO_SOUNDDRIVER_H

#include <functional>
#include <portaudio.h>

typedef signed short int16;

// typedef void sample_callback_t(short *samples, unsigned long frame_count);

class SoundDriver {
public:
    SoundDriver(std::function<void(short *, unsigned long)> samples_cb, long sample_rate);
    ~SoundDriver();
    void start_audio();
    void stop_audio();
    void handle_error(PaError error);
    
    std::function<void(short *, unsigned long)> samples_callback;

private:
    /**
     * Number of audio buffers per second. Adjust if you encounter audio skipping.
     * Note that this sets the floor on how often you'll see changes to the audio
     *scope
     */
    const int fill_rate = 80;
    long sample_rate;
    PaStream *stream;
};

#endif
