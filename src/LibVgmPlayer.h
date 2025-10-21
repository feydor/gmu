#ifndef GMU_LIBVGM_PLAYER_H
#define GMU_LIBVGM_PLAYER_H

#include "GameMusicPlayer.h"
#include "PortAudioSoundDriver.h"
#include <vgm/player/vgmplayer.hpp>
#include <vgm/utils/FileLoader.h>

class LibVgmPlayer : public GameMusicPlayer {
public:
    LibVgmPlayer(long sample_rate);
    ~LibVgmPlayer();
    static bool is_supported_file_extension(const std::string& ext);
    void load_file(const std::string& path) override;
    void start_track(int ntrack, int max_loops) override;
    void toggle_play() override;
    bool track_ended() const override;
    void toggle_continuous_loop() override;
    bool start_next_track() override;
    bool start_prev_track() override;
    void print_now_playing_info() const override;
    void print_current_track_info() override;
private:
    VGMPlayer *player = nullptr;
    DATA_LOADER *loader = nullptr;
    SoundDriver *driver = nullptr;
    bool paused = false;
    int max_loops = 2;
    bool continuous_loop = false;
    int current_track_length_sec = -1;
    std::unordered_map<std::string, std::string> current_track_tags;
};


#endif
