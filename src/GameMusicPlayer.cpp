#include "GameMusicPlayer.h"
#include "LibVgmPlayer.h"
#include "LibGmePlayer.h"

#include <algorithm>

static bool is_libvgm_supported_file(const std::string& path) {
    size_t dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos || dot_pos == 0) {
        return false;
    }

    std::string ext = path.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
    return LibVgmPlayer::is_supported_file_extension(ext);
}

bool GameMusicPlayer::is_supported_file_extension(const std::string& extension) {
    return LibVgmPlayer::is_supported_file_extension(extension) ||
        LibGmePlayer::is_supported_file_extension(extension);
}

std::unique_ptr<GameMusicPlayer>
GameMusicPlayer::from_file(const std::string& path, long sample_rate) {
    std::unique_ptr<GameMusicPlayer> player;

    if (is_libvgm_supported_file(path)) {
        player = std::make_unique<LibVgmPlayer>(sample_rate);
    } else {
        player = std::make_unique<LibGmePlayer>(sample_rate, false);
    }

    player->load_file(path);
    return player;
}
