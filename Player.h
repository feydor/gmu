#ifndef GMU_PLAYER_H
#define GMU_PLAYER_H

#include <gme/gme.h>

#include "PortAudioSoundDriver.h"

class Player {
public:
    Player(long sample_rate);
    ~Player();
    gme_err_t load_file(const char *path);
    gme_err_t start_track(int track);
    static void handle_error(const char* str);
    
    /* Info & control methods */
    bool track_ended();
    int track_count();
    void stop();
    void toggle_play();
    void pause(bool paused);
    void print_track_info(int track);

    Music_Emu *emu;
    gme_info_t* track_info;
    SoundDriver *driver;
    bool paused;
    long sample_rate = 44100;
};

#endif
