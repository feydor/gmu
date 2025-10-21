#ifndef GMU_PLAYER_H
#define GMU_PLAYER_H

#include "GameMusicPlayer.h"

#include <gme/gme.h>

#include "SoundDriver.h"

class LibGmePlayer : public GameMusicPlayer {
public:
    LibGmePlayer(long sample_rate, bool loop);
    ~LibGmePlayer();
    /**
     * @param extension with or without leading period
     */
    static bool is_supported_file_extension(const std::string& extension);
    void load_file(const std::string& path) override;
    /** Loads an m3u playlist. Must have called load_file before this. */
    void load_m3u(const char* path);
    void start_track(int ntrack, int max_loops) override;
    /**
     * Try to start playback of next track
     * @returns true if next track was started
     */
    bool start_next_track() override;
    /**
     * Try to start playback of previous track
     * @returns true if previous track was started
     */
    bool start_prev_track() override;
    void toggle_play() override;
    void toggle_continuous_loop() override;
    void skip(int ms);    
    bool track_ended() const override;
    void print_current_track_info() override;
    void print_now_playing_info() const override;

private:
    int track_count() const;
    static void handle_error(const char* str);

    Music_Emu *emu = nullptr;
    gme_info_t* track_info = nullptr;
    SoundDriver *driver = nullptr;
    bool paused = true;
    long sample_rate = 44100;
    bool loop = false;
    int current_track = 0;
};

#endif
