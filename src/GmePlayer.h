#ifndef GMU_PLAYER_H
#define GMU_PLAYER_H

#include <gme/gme.h>

#include "SoundDriver.h"

class GmePlayer {
public:
    /** Using default sound driver */
    GmePlayer(long sample_rate, bool loop);
    ~GmePlayer();

    /* Control methods */
    void load_file(const char *path);
    /** Loads an m3u playlist. Must have called load_file before this. */
    void load_m3u(const char* path);
    /** Starts playback. Must have called load_file before this. */
    void start_track(int track);
    void toggle_play();
    void toggle_loop();
    void skip(int ms);

    /* Info methods */
    bool track_ended() const;
    int track_count() const;
    void print_track_info(int track) const;
    /** Prints a progress meter. Run this in the main event loop. Uses carriage return (\\r) to reprint on the same line each call. */
    void print_now_playing_line() const;

    static void handle_error(const char* str);

private:
    Music_Emu *emu = nullptr;
    gme_info_t* track_info = nullptr;
    SoundDriver *driver = nullptr;
    bool paused = true;
    long sample_rate = 44100;
    bool loop = false;
};

#endif
