#include "LibGmePlayer.h"
#include "LibVgmPlayer.h"
#include "../argspp/src/args.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <filesystem> 

// TODO: All Posix only, used for IO
// Maybe use define to switch code based on OS? (conio.h on Windows)
#include <unistd.h>
#include <termios.h>
#include <sys/utsname.h>

#define SPACE 32
#define VERSION "v1.1.0"

using namespace std;
namespace fs = std::filesystem;

// Globals
static atomic<char> latest_key_pressed('\0');
std::atomic<bool> running{true};
bool playlist_mode = false;
int playlist_size = 1;
struct termios old_tio;

static void check_for_key_press();
static int play_file(const string& path, int i);
static vector<string> get_filepaths(const string& dir);
static string get_version();

int main(int argc, char* argv[]) {
    args::ArgParser parser("Usage: gmu FILE-OR-DIRECTORY [-l loop] [-v] [-h]\nvideo game music format player (VGM, VGZ, NSF, GBS, etc)", get_version());
    parser.option("track t", "0");
    parser.flag("loop l");
    parser.parse(argc, argv);
    if (parser.args.size() == 0) {
        printf("%s\n", parser.helptext.c_str());
        exit(EXIT_FAILURE);
    }

    string path = parser.args[0];
    if (!fs::exists(path)) {
	fprintf(stderr, "File does not exist!\n");
	return 1;
    }
    
    string opt_playlist = parser.args.size() == 2 ? parser.args[1] : "";
    // int track = stoi(parser.value("track"));
    
    // start keyboard input thread
    thread input_thread(check_for_key_press);

    if (fs::is_directory(path)) {
	playlist_mode = true;
	vector<string> filepaths = get_filepaths(path);
	playlist_size = filepaths.size();
	size_t i = 0;
	while (i < filepaths.size()) {
	    i = play_file(filepaths[i], i);
	}
    } else {
	play_file(path, 0);
    }

    printf("Bye.\n");
    // Restore the old CLI settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    exit(0);
}

static vector<string> get_filepaths(const string& dir) {
    vector<string> out;
    for (const auto& entry : fs::directory_iterator(dir)) {
	if (fs::is_regular_file(entry)
	    && GameMusicPlayer::is_supported_file_extension(entry.path().extension())) {
	    out.push_back(entry.path().string());
	}
    }
    return out;
}

static int play_file(const string& path, int playlist_index) {
    auto player = GameMusicPlayer::from_file(path, 44100);
    // clear warnings from portaudio
    printf("\033[1J\033[H"); fflush(stdout);

    player->load_file(path.c_str());
    // TODO: currently unsupported, maybe treat as similar to a directory?
    // if (ends_with(opt_playlist, ".m3u"))
    //     player.load_m3u(opt_playlist.c_str());
    // player.start_track(track);
    player->start_track();
    running = true;
    while (running) {
        if (player->track_ended() == true) {
            running = false;
            break;
        }

        // non-blocking check for keyboard input
        if (latest_key_pressed != '\0') {
            switch (latest_key_pressed) {
            case '\0': break;
            case 'p':
            case SPACE:
                player->toggle_play();
                break;
            case '>':
		if (playlist_mode && (playlist_index + 1) < playlist_size) {
		    running = false;
		    break;
		}
                break;
            case '<':
		if (playlist_mode && playlist_index >= 1)
		    return playlist_index - 1;               
                break;
	    case '.':
		player->start_next_track();
		break;
	    case ',':
		player->start_prev_track();
		break;
            // case '.':
            //     player.skip(5000);
            //     break;
            // case ',':
            //     player.skip(-5000);
            //     break;
            case 'L':
                player->toggle_continuous_loop();
                break;
            default:
                break;
            }

            latest_key_pressed = '\0';
        }

        player->print_now_playing_info();
        this_thread::sleep_for(chrono::milliseconds(16)); // 60fps
    }

    return playlist_index + 1;
}

static string get_version() {
    string version = VERSION;
    struct utsname uts;
    int err = uname(&uts);
    if (err < 0) return version + "-unknown_machine";
    return version + "-" + uts.machine + "-" + uts.sysname;
}

// Function to continuously check for keyboard input
static void check_for_key_press() {
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);

    // Save the old settings to restore them at the end
    new_tio = old_tio;

    // Disable canonical mode (buffered i/o) and local echo
    new_tio.c_lflag &=(~ICANON & ~ECHO);

    // Set the new settings immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    
    unsigned char c;
    while (running) {
        c = getchar();
        latest_key_pressed = c;
    }

    // Restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}
