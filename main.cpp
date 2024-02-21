#include "Player.h"

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

using namespace std;

static void setup_terminal();
static void restore_terminal();

int main(int argc, char* argv[]) {
    if (argc != 2)
        Player::handle_error("Needs an argument");
    
    long sample_rate = 44100;
    char *path = argv[1];    
    int track = 0;
    Player player{sample_rate};
    player.load_file(path);
	player.start_track(track);
	
	setup_terminal();
	
	bool running = true;
	char c;
	while ((read(STDIN_FILENO, &c, 1) == 1) && running) {
	    if (player.track_ended()) {
	        if (track < player.track_count())
	            player.start_track(++track);
	        else
	            running = false;
	    }
	    
        if (c == ' ') {
            printf("Paused. Press any key to continue...\n");
            player.toggle_play();
            getchar();
            player.toggle_play();
            printf("Continuing...\n");
        }
        
        if (c == 'q') {
            running = false;
        }
        
        this_thread::sleep_for(chrono::seconds(1));
    }

// 	bool running = true;
// 	while (running) {
// 	    if (player.track_ended()) {
// 	        if (track < player.track_count())
// 	            player.start_track(++track);
// 	        else
// 	            running = false;
// 	    }
// 
// 	    this_thread::sleep_for(chrono::seconds(1));
// 	}

    restore_terminal();
	
	return 0;
}

static void setup_terminal() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Disable canonical mode (buffered i/o) and local echo
    term.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void restore_terminal() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Enable canonical mode (buffered i/o) and local echo
    term.c_lflag |= (ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
