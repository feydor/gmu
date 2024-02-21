#include "Player.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <functional>
#include <stdexcept>

Player::Player(long sample_rate) : emu{nullptr}, track_info{nullptr}, driver{nullptr}, paused{true}, sample_rate{sample_rate} {
    driver = new SoundDriver([&](short* buf, unsigned long frame_count) {
        auto err = gme_play(this->emu, frame_count, buf);
        if (err) {
            printf("returned=%s\n", err);
            throw std::runtime_error(err);
        }
    },
    sample_rate);
    driver->start_audio();
}

Player::~Player() {
    stop();
    gme_free_info(track_info);
    delete driver;
}

void Player::stop() {
    gme_delete(emu);
    emu = nullptr;
}

bool Player::track_ended() {
    return emu ? gme_track_ended(emu) : false;
}

int Player::track_count() {
    return emu ? gme_track_count(emu) : false;
}

void Player::pause(bool paused) {
	this->paused = paused;
	if (paused)
		driver->stop_audio();
	else
		driver->start_audio();
}

void Player::toggle_play() {
    paused = !paused;
    if (paused)
		driver->stop_audio();
	else
		driver->start_audio();       
}

gme_err_t Player::load_file(const char *path) {
    // Determine file type
    gme_type_t file_type = gme_identify_extension(path);
	if (!file_type)
		Player::handle_error("Unsupported music type");

    printf("Loading file %s...\n", path);
    fflush(stdout);
    Player::handle_error(gme_open_file(path, &emu, sample_rate));
    return 0;
}

gme_err_t Player::start_track(int track) {
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

    paused = false;
	return 0;
}

void Player::handle_error(const char* str) {
	if (str)
	    throw std::runtime_error(str);
}

void Player::print_track_info(int track) {
	/* Get and print main info for track */
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
