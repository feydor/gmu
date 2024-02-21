#ifndef GMU_SOUNDDRIVER_H
#define GMU_SOUNDDRIVER_H

#include <functional>

typedef signed short int16;

class SoundDriver {
public:
    virtual void start_audio() = 0;
    virtual void stop_audio() = 0;
    virtual ~SoundDriver(){}
    /** Loads a 16bit signed buffer with samples */
    virtual void load_samples(short *buf, unsigned long nframes) const = 0;
};

#endif
