#ifndef GMU_SOUNDDRIVER_H
#define GMU_SOUNDDRIVER_H

#include <functional>
#include <stdint.h>

typedef	int16_t	i16; /** Signed 16 bit value */

/** Interface for a sound driver */
class SoundDriver {
public:
    /** Starts the audio stream for writing */
    virtual void start_audio() = 0;

    /** Stops the audio stream. Call start_audio() to start again. */
    virtual void stop_audio() = 0;
    
    /** Loads a 16bit signed buffer with samples */
    virtual bool load_samples(i16 *buf, unsigned long nframes) const = 0;

    virtual bool stream_running() const = 0;

    virtual ~SoundDriver(){}
};

#endif
