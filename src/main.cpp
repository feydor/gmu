#include "GmePlayer.h"
#include "../argspp/src/args.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>

// TODO: Unix only, Maybe use define to switch code based on OS (conio.h on Windows)
#include <unistd.h>
#include <termios.h>

using namespace std;

static atomic<char> latest_key_pressed('\0');

static bool ends_with(const string &str, const string &ext);
static void check_for_key_press();

int main(int argc, char* argv[]) {
    args::ArgParser parser("Usage: gmu file [m3u-playlist] [-t track] [-l loop]\nvideo game music format player", "1.0.0");
    parser.option("track t", "0");
    parser.flag("loop l");
    parser.parse(argc, argv);
    if (parser.args.size() == 0) {
        printf("%s\n", parser.helptext.c_str());
        exit(EXIT_FAILURE);
    }

    string path = parser.args[0];
    string opt_playlist = parser.args.size() == 2 ? parser.args[1] : "";
    int track = stoi(parser.value("track"));
    GmePlayer player{44100, parser.found("loop")};
    printf("\033[1J\033[H"); fflush(stdout); // clear warnings from portaudio

    player.load_file(path.c_str());
    if (ends_with(opt_playlist, ".m3u"))
        player.load_m3u(opt_playlist.c_str());
    player.start_track(track);

    // start keyboard input thread
    thread input_thread(check_for_key_press);

    bool running = true;
    while (running) {
        if (player.track_ended()) {
            if (!player.start_next_track())
                running = false;
        }

        // non-blocking check for keyboard input
        if (latest_key_pressed != '\0') {
            switch (latest_key_pressed) {
            case '\0': break;
            case 'p':
            case 32:
                player.toggle_play();
                break;
            case 'n':
            case '>':
                player.start_next_track();
                break;
            case 'b':
            case '<':
                player.start_prev_track();
                break;
            case '.':
                player.skip(5000);
                break;
            case ',':
                player.skip(-5000);
                break;
            case 'L':
                player.toggle_loop();
                break;
            default:
                break;
            }

            latest_key_pressed = '\0';
        }

        player.print_now_playing_line();
        this_thread::sleep_for(chrono::milliseconds(16)); // 60fps
    }

    input_thread.join();
    return 0;
}

static bool ends_with(const string &str, const string &ext) {
    if (ext[0] != '.')
        throw runtime_error("ext must start with a period");
    return str.length() >= ext.length() + 1 &&
           str.compare(str.length() - ext.length(), ext.length(), ext) == 0;
}

// Function to continuously check for keyboard input
static void check_for_key_press() {
    struct termios old_tio, new_tio;
    unsigned char c;

    tcgetattr(STDIN_FILENO, &old_tio);

    // Save the old settings to restore them at the end
    new_tio = old_tio;

    // Disable canonical mode (buffered i/o) and local echo
    new_tio.c_lflag &=(~ICANON & ~ECHO);

    // Set the new settings immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    while (true) {
        c = getchar();
        latest_key_pressed = c;
    }

    // Restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}
