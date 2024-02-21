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
        throw runtime_error("Needs an argument");

    int track = 0;
    Player player{44100};
    player.load_file(argv[1]);
    player.start_track(track);

    bool running = true;
    while (running) {
        if (player.track_ended()) {
            if (track < player.track_count())
                player.start_track(++track);
            else
                running = false;
        }

        this_thread::sleep_for(chrono::seconds(1));
    }

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
