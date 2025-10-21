#include "LibVgmPlayer.h"
#include "Utils.h"
#include <stdexcept>

static INT16 sampleConv_toS16(INT32 value) {
	value >>= 8;	// 24 bit -> 16 bit
	if (value < -0x8000)
		value = -0x8000;
	else if (value > +0x7FFF)
		value = +0x7FFF;
	return (INT16)value;
}

static void check_err(int err, const char* msg) {
    if (err) {
        fprintf(stderr, "%s\n", msg);
        throw std::runtime_error(msg);
    }
}

static std::unordered_map<std::string, std::string> get_track_tags(const char* const* tagList) {
    std::unordered_map<std::string, std::string> map;
    for (const char* const* t = tagList; *t; t += 2) {
        map[std::string{t[0]}] = std::string{t[1]};
    }
    return map;
}

LibVgmPlayer::LibVgmPlayer(long sample_rate) {
    player = new VGMPlayer();
    player->SetSampleRate(sample_rate);
    player->SetPlaybackSpeed(1.0);

    auto samples_callback = [&](i16* buf, unsigned long buf_size) -> bool {
        if (buf_size < 2L) {
            return false;
        }

        if ((int)player->GetCurLoop() >= max_loops) {
            // TODO: fadeout
            player->Stop();
            return true;
        }

        // pack interleaved frames into 32bit samples
        WAVE_32BS samples[buf_size/2];
        int j = 0;
        for (unsigned long i=1; i<buf_size; i += 2, ++j) {
            samples[j].L = buf[i-1];
            samples[j].R = buf[i];
        }

        player->Render(buf_size / 2, samples);

        // unpack samples back
        j = 0;
        for (unsigned long i=1; i<buf_size; i += 2, ++j) {
            buf[i-1] = sampleConv_toS16(samples[j].L);
            buf[i] = sampleConv_toS16(samples[j].R);
        }
        return false;
    };
    driver = new PortAudioSoundDriver(samples_callback, sample_rate);
}

LibVgmPlayer::~LibVgmPlayer() {
    driver->stop_audio();
    delete driver;
    DataLoader_Deinit(loader);
    delete player;
}

void LibVgmPlayer::load_file(const std::string& path) {
    loader = FileLoader_Init(path.c_str());
    Utils::require_nonnull(loader, "Failed to create file loader!");

    /* attempt to load 256 bytes, bail if not possible */
    DataLoader_SetPreloadBytes(loader,0x100);
    check_err(DataLoader_Load(loader), "Failed to load DataLoader");

    /* associate the fileloader to the player -
     * automatically reads the rest of the file */
    check_err(player->LoadFile(loader), "Failed to load file");
}

// TODO: there are no tracks in VGM?
void LibVgmPlayer::start_track(int ntrack, int max_loops) {
    (void) ntrack;
    this->max_loops = max_loops;
    current_track_length_sec = player->Tick2Second(player->GetTotalTicks());
    current_track_tags = get_track_tags(player->GetTags());
    print_current_track_info();

    player->Start();

    // start audio
    paused = false;
    if (!driver->stream_running())
        driver->start_audio();
}

void LibVgmPlayer::toggle_play() {
    paused = !paused;
    if (paused)
        driver->stop_audio();
    else
        driver->start_audio();
}

bool LibVgmPlayer::track_ended() const {
    return player->GetState() == 0;
}

void LibVgmPlayer::toggle_continuous_loop() {
    continuous_loop = !continuous_loop;
    if (continuous_loop) {
        max_loops = INT32_MAX;
    } else {
        max_loops = player->GetCurLoop() + 1;
    }
}

bool LibVgmPlayer::start_next_track() {
    // TODO: FIX
    return true;
}

bool LibVgmPlayer::start_prev_track() {
    // TODO: FIX
    return true;
}

void LibVgmPlayer::print_current_track_info() {
    printf("GMU Player\n");
    printf("-------------\n");
    printf("VGM v%3X\n", player->GetFileHeader()->fileVer);    
    for (const auto& [key, val] : current_track_tags) {
        if (!val.empty()) printf("%s: %s\n", key.c_str(), val.c_str());
    }
    printf("\n");
}

void LibVgmPlayer::print_now_playing_info() const {
    int since_s = player->Tick2Second(player->GetCurPos(PLAYPOS_TICK));
    printf("Playing : %s", Utils::format_min_sec(since_s).c_str());
    printf(" / %s seconds", Utils::format_min_sec(current_track_length_sec).c_str());
    if (continuous_loop) printf(" (loop)");
    printf("\r");
    fflush(stdout);
}
