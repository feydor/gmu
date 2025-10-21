#include "Utils.h"
#include <algorithm>

std::string Utils::format_min_sec(float time_s) {
    unsigned time_m = time_s / 60;
    time_s -= time_m * 60;

    char buf[100 ];
    snprintf(buf, sizeof(buf), "%02hu:%05.2f", time_m, time_s);
    std::string str = buf;
    return str;
}

bool Utils::is_blank(const std::string& str) {
    return str.empty() || std::all_of(std::begin(str), std::end(str),
                                      [](char c){ return c == ' ';});
}

