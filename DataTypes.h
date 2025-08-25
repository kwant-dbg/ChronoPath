#ifndef DATATYPES_H_INCLUDED
#define DATATYPES_H_INCLUDED

#define _USE_MATH_DEFINES // For M_PI constant
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath> // --- NEW --- For math functions

// --- Core Data Structures ---

struct Time {
    int h = 0, m = 0, s = 0;
    Time() = default;
    Time(const std::string& t_str) { sscanf(t_str.c_str(), "%d:%d:%d", &h, &m, &s); }
    int toSeconds() const { return h * 3600 + m * 60 + s; }
    static Time fromSeconds(int total_seconds) {
        Time t;
        t.h = total_seconds / 3600;
        t.m = (total_seconds % 3600) / 60;
        t.s = total_seconds % 60;
        return t;
    }
    bool operator>(const Time& other) const { return toSeconds() > other.toSeconds(); }
    bool operator<(const Time& other) const { return toSeconds() < other.toSeconds(); }
    bool operator>=(const Time& other) const { return toSeconds() >= other.toSeconds(); }
    bool operator<=(const Time& other) const { return toSeconds() <= other.toSeconds(); }
};

struct Stop {
    int id;
    std::string name;
    double lat = 0.0;
    double lon = 0.0;
};

struct StopTime { std::string trip_id; Time arrival_time; Time departure_time; int stop_id; int stop_sequence; };
struct Transfer { int from_stop_id; int to_stop_id; int duration_seconds; };

struct Journey {
    Time arrival_time;
    int trips;
    Time departure_time;
    int from_stop_id = -1;
    std::string method;
};

// --- Helper Functions ---
std::ostream& operator<<(std::ostream& os, const Time& t);


// --- NEW FUNCTION ---
// Calculates the distance in meters between two lat/lon points
inline double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0; // Earth radius in meters
    double phi1 = lat1 * M_PI / 180.0;
    double phi2 = lat2 * M_PI / 180.0;
    double delta_phi = (lat2 - lat1) * M_PI / 180.0;
    double delta_lambda = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(delta_phi / 2.0) * sin(delta_phi / 2.0) +
               cos(phi1) * cos(phi2) *
               sin(delta_lambda / 2.0) * sin(delta_lambda / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return R * c; // in meters
}


#endif // DATATYPES_H_INCLUDED
