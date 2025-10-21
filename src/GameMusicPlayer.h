#ifndef GMU_GAME_MUSIC_PLAYER_H
#define GMU_GAME_MUSIC_PLAYER_H

#include <memory>
#include <string>

class GameMusicPlayer {
public:
    /**
     * Construct the correct player by filetype
     */
    static std::unique_ptr<GameMusicPlayer> from_file(const std::string& path, long sample_rate);
    static bool is_supported_file_extension(const std::string& extension);
    virtual void load_file(const std::string& path) = 0;
    /** Starts playback. Must have called load_file before this. */
    virtual void start_track(int ntrack = 0, int max_loops = 2) = 0;
    virtual void toggle_play() = 0;
    virtual void toggle_continuous_loop() = 0;
    virtual bool track_ended() const = 0;
    virtual void print_current_track_info() = 0;
    virtual bool start_next_track() = 0;
    virtual bool start_prev_track() = 0;
    /**
     * Prints a progress meter. Run this in the main event loop.
     * Uses carriage return (\\r) to reprint on the same line each call.
     */
    virtual void print_now_playing_info() const = 0;

    virtual ~GameMusicPlayer(){}
};

#endif
