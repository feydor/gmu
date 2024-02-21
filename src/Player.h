#ifndef GMU_PLAYER_H
#define GMU_PLAYER_H

#include <gme/gme.h>

#include "SoundDriver.h"

class Player {
public:
    /** Using default sound driver */
    Player(long sample_rate);
    ~Player();

    /* Control methods */
    void load_file(const char *path);
    void start_track(int track);
    void toggle_play();

    /* Info methods */
    bool track_ended() const;
    int track_count() const;
    void print_track_info(int track) const;

    static void handle_error(const char* str);

private:
    Music_Emu *emu = nullptr;
    gme_info_t* track_info = nullptr;
    SoundDriver *driver = nullptr;
    bool paused = true;
    long sample_rate = 44100;
};

#endif
