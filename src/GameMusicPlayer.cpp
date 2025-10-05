#include "GameMusicPlayer.h"
#include "LibVgmPlayer.h"
#include "LibGmePlayer.h"

#include <algorithm>

static bool is_file_vgm_or_vgz(const std::string& path) {
    size_t dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos || dot_pos == 0) {
        return false;
    }

    std::string ext = path.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
    return ext == "VGM" || ext == "VGZ";
}

std::unique_ptr<GameMusicPlayer>
GameMusicPlayer::from_file(const std::string& path, long sample_rate) {
    std::unique_ptr<GameMusicPlayer> player;

    if (is_file_vgm_or_vgz(path)) {
        player = std::make_unique<LibVgmPlayer>(sample_rate);
    } else {
        player = std::make_unique<LibGmePlayer>(sample_rate, false);
    }

    player->load_file(path);
    return player;
}
