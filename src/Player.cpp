#include "Player.h"
#include "PortAudioSoundDriver.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <functional>
#include <stdexcept>

Player::Player(long sample_rate) : sample_rate{sample_rate} {
    driver = new PortAudioSoundDriver([&](short* buf, unsigned long frame_count) {
        auto err = gme_play(this->emu, frame_count, buf);
        if (err) {
            fprintf(stderr, "err=%s\n", err);
            throw std::runtime_error(err);
        }
    },
    sample_rate);
}

Player::~Player() {
    driver->stop_audio();
    gme_delete(emu);
    gme_free_info(track_info);
    delete driver;
}

bool Player::track_ended() const {
    return emu ? gme_track_ended(emu) : false;
}

int Player::track_count() const {
    return emu ? gme_track_count(emu) : -1;
}

void Player::toggle_play() {
    paused = !paused;
    if (paused)
        driver->stop_audio();
    else
        driver->start_audio();
}

void Player::load_file(const char *path) {
    // Determine file type
    gme_type_t file_type = gme_identify_extension(path);
    if (!file_type)
        throw std::runtime_error("Unsupported music file type");

    printf("Loading file %s...\n", path);
    fflush(stdout);
    Player::handle_error(gme_open_file(path, &emu, sample_rate));
}

void Player::start_track(int track) {
    if (!emu) throw std::runtime_error("Didn't initialize emulator");

    if (!track_info) {
        gme_free_info(track_info);
        track_info = nullptr;
    }

    Player::handle_error(gme_track_info(emu, &track_info, track));
    print_track_info(track);

    // Start track
    Player::handle_error(gme_start_track(emu, track));

    // Calculate track length
    if (track_info->length <= 0)
        track_info->length = track_info->intro_length + track_info->loop_length; // *2

    // If above failed, set default track length
    if (track_info->length <= 0 )
        track_info->length = (long) (2.5 * 60 * 1000);

    // Set fade out point
    gme_set_fade(emu, track_info->length - 8000);

    // Start audio
    paused = false;
    if (!driver->stream_running())
        driver->start_audio();
}

void Player::handle_error(const char* str) {
    if (str)
        throw std::runtime_error(str);
}

void Player::print_track_info(int track) const {
    // Get and print main info for track
    if (!track_info)
        throw std::runtime_error("Track not loaded");

       printf( "System   : %s\n", track_info->system );
       printf( "Game     : %s\n", track_info->game );
       printf( "Author   : %s\n", track_info->author );
       printf( "Copyright: %s\n", track_info->copyright );
       printf( "Comment  : %s\n", track_info->comment );
       printf( "Dumper   : %s\n", track_info->dumper );
       printf( "Tracks   : %d\n", (int) gme_track_count(emu));
       printf( "\n" );
       printf( "Track    : %d\n", (int) track + 1);
       printf( "Name     : %s\n", track_info->song);
       printf( "Length   : %ld:%02ld",
               (long) track_info->length / 1000 / 60, (long) track_info->length / 1000 % 60 );
       if (track_info->loop_length != 0)
           printf(" (endless)");
       printf("\n\n");
}
