#pragma once
// utils dot h
// just a bunch of helper functions for time and distance stuff

#include <string>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>

namespace utils {

// HHMMSS into sec
// gtfs time can be weird like 25:00:00 so this handles it
int time_to_seconds(const std::string& time_str) {
    int h, m, s;
    char colon;
    std::stringstream ss(time_str);
    ss >> h >> colon >> m >> colon >> s;
    return h * 3600 + m * 60 + s;
}

// turns secs back into HHMMSS

std::string seconds_to_time(int total_seconds) {
    int h = total_seconds / 3600;
    int m = (total_seconds % 3600) / 60;
    int s = total_seconds % 60;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << h << ":"
       << std::setw(2) << std::setfill('0') << m << ":"
       << std::setw(2) << std::setfill('0') << s;
    return ss.str();
}

// using the haversine formula for walking distances
double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371e3; // earth radius in meters lol
    double phi1 = lat1 * M_PI / 180.0;
    double phi2 = lat2 * M_PI / 180.0;
    double delta_phi = (lat2 - lat1) * M_PI / 180.0;
    double delta_lambda = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(delta_phi / 2.0) * sin(delta_phi / 2.0) +
               cos(phi1) * cos(phi2) *
               sin(delta_lambda / 2.0) * sin(delta_lambda / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return R * c; 
}

}
