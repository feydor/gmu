#include "GmePlayer.h"
#include "Utils.h"
#include "PortAudioSoundDriver.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <climits>

static int fade_time(int track_length);

GmePlayer::GmePlayer(long sample_rate, bool loop)
    : sample_rate{sample_rate}, loop{loop} {
    auto samples_callback = [&](i16* buf, unsigned long frame_count) {
        auto err = gme_play(this->emu, frame_count, buf);
        if (err) {
            fprintf(stderr, "err=%s\n", err);
            throw std::runtime_error(err);
        }
    };
    driver = new PortAudioSoundDriver(samples_callback, sample_rate);
}

GmePlayer::~GmePlayer() {
    driver->stop_audio();
    gme_delete(emu);
    gme_free_info(track_info);
    delete driver;
}

bool GmePlayer::track_ended() const {
    return emu ? gme_track_ended(emu) : false;
}

int GmePlayer::track_count() const {
    return emu ? gme_track_count(emu) : -1;
}

void GmePlayer::toggle_play() {
    paused = !paused;
    if (paused)
        driver->stop_audio();
    else
        driver->start_audio();
}

void GmePlayer::toggle_loop() {
    loop = !loop;
    if (loop) {
        // set fade to indefinte time
        gme_set_fade(emu, INT_MAX - 8000);
    } else {
        // do one more loop, then end
        gme_set_fade(emu, gme_tell(emu) + fade_time(track_info->length));
    }
}

void GmePlayer::load_file(const char *path) {
    // Determine file type
    gme_type_t file_type = gme_identify_extension(path);
    Utils::require_nonnull(file_type, "Unsupported music file type");

    // printf("Loading file %s...\n", path); fflush(stdout);
    GmePlayer::handle_error(gme_open_file(path, &emu, sample_rate));
}

void GmePlayer::start_track(int track) {
    Utils::require_nonnull(emu, "Didn't initialize emulator");

    if (!track_info) {
        gme_free_info(track_info);
        track_info = nullptr;
    }

    GmePlayer::handle_error(gme_track_info(emu, &track_info, track));
    // Calculate track length
    if (track_info->length <= 0)
        track_info->length = track_info->intro_length + track_info->loop_length * 2;

    // If above failed, set default track length
    if (track_info->length <= 0 )
        track_info->length = (long) (2.5 * 60 * 1000);

    print_track_info(track);

    // Start track
    GmePlayer::handle_error(gme_start_track(emu, track));

    // Set fade out point
    if (!loop)
        gme_set_fade(emu, fade_time(track_info->length));

    // Start audio
    paused = false;
    if (!driver->stream_running())
        driver->start_audio();
}

void GmePlayer::load_m3u(const char* path) {
    Utils::require_nonnull(emu, "Didn't initialize emulator");
    Utils::require_nonnull(path, "path was null");

    GmePlayer::handle_error(gme_load_m3u(emu, path));
}

void GmePlayer::handle_error(const char* str) {
    if (str) {
        fprintf(stderr, "Found error: %s\n", str); fflush(stderr);
        throw std::runtime_error(str);
    }
}

void GmePlayer::skip(int ms) {
    Utils::require_nonnull(emu, "Didn't initialize emulator");
    int since_ms = gme_tell(emu);
    if (since_ms + ms > 0 && since_ms + ms < track_info->play_length)
        gme_seek(emu, since_ms + ms);
}

/** Get the time in ms where the track should start fading out */
static int fade_time(int track_length) {
    return track_length - (track_length < 8000 ? 1000 : 8000);
}

void GmePlayer::print_track_info(int track) const {
    // Get and print main info for track
    Utils::require_nonnull(track_info, "Track not loaded");

    #define PRINT_NONEMPTY(format, str) if (str[0] != '\0') printf(format, str)

    printf("GMU VGM Player\n");
    printf("-------------\n\n");
    PRINT_NONEMPTY("System   : %s\n", track_info->system);
    PRINT_NONEMPTY("Game     : %s\n", track_info->game);
    PRINT_NONEMPTY("Composer : %s\n", track_info->author);
    PRINT_NONEMPTY("Copyright: %s\n", track_info->copyright);
    PRINT_NONEMPTY("Comment  : %s\n", track_info->comment);
    PRINT_NONEMPTY("Dumper   : %s\n", track_info->dumper);
    printf("Tracks   : %d\n", (int) gme_track_count(emu));
    printf("\n" );
    printf("Track    : %d\n", (int) track + 1);
    PRINT_NONEMPTY("Name     : %s\n", track_info->song);
    printf("Length   : %02ld:%02ld",
            (long) track_info->length / 1000 / 60, (long) track_info->length / 1000 % 60 );
    if (loop)
        printf(" (loop)");
    printf("\n\n");
}

void GmePlayer::print_now_playing_line() const {
    Utils::require_nonnull(emu, "Didn't initialize emulator");

    int since_ms = gme_tell(emu);
    int total_ms = track_info->play_length;
    printf("Playing %01.2f%%   \b\b\b\t", 100.0f * since_ms / total_ms);
    printf("%s", Utils::format_min_sec(since_ms / 1000.0f).c_str());
    printf(" / ");
    printf("%s seconds", Utils::format_min_sec(track_info->play_length / 1000.0f).c_str());
    if (loop)
        printf(" (loop)");
    else
        printf("       ");
    printf("\r");
    fflush(stdout);
}
