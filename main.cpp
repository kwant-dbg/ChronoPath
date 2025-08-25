#define _WIN32_WINNT 0x0A00 // We are targeting Windows 10 or later

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include "httplib.h" // The web server library
#include "DataTypes.h"
#include "Raptor.h"

// Helper function implementations that were previously in main.cpp
std::ostream& operator<<(std::ostream& os, const Time& t) {
    char buffer[9];
    sprintf(buffer, "%02d:%02d:%02d", t.h, t.m, t.s);
    os << buffer;
    return os;
}

std::string getStopName(int stop_id, const std::map<int, Stop>& stops) {
    auto it = stops.find(stop_id);
    return (it != stops.end()) ? it->second.name : "Unknown Stop";
}


// --- NEW: Path Reconstruction Function ---
std::vector<PathStep> reconstructPath(int start_id, int end_id, const Journey& final_journey,
                                      const std::map<int, std::map<int, Journey>>& predecessors,
                                      const std::map<int, Stop>& stops) {
    std::vector<PathStep> path;
    Journey current_journey = final_journey;
    int current_stop = end_id;

    while (current_stop != start_id && current_journey.from_stop_id != -1) {
        path.push_back({current_stop, getStopName(current_stop, stops), current_journey.arrival_time, current_journey.method});
        int prev_stop = current_journey.from_stop_id;
        int prev_trips = current_journey.method == "Walk" ? current_journey.trips : current_journey.trips - 1;

        if (predecessors.count(prev_stop) && predecessors.at(prev_stop).count(prev_trips)) {
            current_journey = predecessors.at(prev_stop).at(prev_trips);
            current_stop = prev_stop;
        } else {
            break; // Path reconstruction finished or error
        }
    }
    std::reverse(path.begin(), path.end());
    return path;
}

int main() {
    // --- 1. Load and Pre-process GTFS Data (Happens once at startup) ---
    std::map<int, Stop> stops;
    std::vector<StopTime> stop_times;
    std::map<int, std::vector<Transfer>> transfers_map;
    std::map<std::string, std::vector<StopTime>> trips_map;
    std::map<int, std::vector<std::string>> routes_serving_stop;

    // [Omitted repetitive file loading code for brevity - keep your existing loaders]
    std::ifstream stops_file("stops.txt"); std::string line; getline(stops_file, line);
    //while (getline(stops_file, line)) { std::stringstream ss(line); std::string field; Stop s; getline(ss, field, ','); s.id = std::stoi(field); getline(ss, field, ','); s.name = field; stops[s.id] = s; }
    while (getline(stops_file, line)) {
        std::stringstream ss(line);
        std::string stop_id_str, stop_code, stop_name, stop_lat_str, stop_lon_str;
        Stop s;
        // Read the first 5 columns, ignoring the rest
        getline(ss, stop_id_str, ',');
        getline(ss, stop_code, ',');
        getline(ss, stop_name, ',');
        getline(ss, stop_lat_str, ',');
        getline(ss, stop_lon_str, ',');

        try {
            s.id = std::stoi(stop_id_str);
            s.name = stop_name;
            s.lat = std::stod(stop_lat_str);
            s.lon = std::stod(stop_lon_str);
            stops[s.id] = s;
        } catch (const std::exception& e) {
            // This safely skips any lines with bad data
        }
    }
    std::ifstream st_file("stop_times.txt"); getline(st_file, line);
    while (getline(st_file, line)) { std::stringstream ss(line); std::string field; StopTime st; getline(ss, field, ','); st.trip_id = field; getline(ss, field, ','); st.arrival_time = Time(field); getline(ss, field, ','); st.departure_time = Time(field); getline(ss, field, ','); st.stop_id = std::stoi(field); getline(ss, field, ','); st.stop_sequence = std::stoi(field); stop_times.push_back(st); }
    std::ifstream tr_file("transfers.txt"); getline(tr_file, line);
    while (getline(tr_file, line)) { std::stringstream ss(line); std::string field; Transfer t; getline(ss, field, ','); t.from_stop_id = std::stoi(field); getline(ss, field, ','); t.to_stop_id = std::stoi(field); getline(ss, field, ','); t.duration_seconds = std::stoi(field); transfers_map[t.from_stop_id].push_back(t); }

    for (const auto& st : stop_times) {
        trips_map[st.trip_id].push_back(st);
        routes_serving_stop[st.stop_id].push_back(st.trip_id);
    }
    for (auto& pair : trips_map) { std::sort(pair.second.begin(), pair.second.end(), [](const StopTime& a, const StopTime& b) { return a.stop_sequence < b.stop_sequence; }); }
    for (auto& pair : routes_serving_stop) { std::sort(pair.second.begin(), pair.second.end()); pair.second.erase(std::unique(pair.second.begin(), pair.second.end()), pair.second.end()); }
    std::cout << "Data loaded and pre-processed for server." << std::endl;

    // --- 2. Create and Configure the Web Server ---
    httplib::Server svr;

    // This tells the server to serve files from the same directory the .exe is in
    svr.set_base_dir("./");



    // API Endpoint to get the list of all stops
    svr.Get("/api/stops", [&](const httplib::Request& req, httplib::Response& res) {
        std::stringstream json;
        json << "[";
        for (auto it = stops.begin(); it != stops.end(); ++it) {
            // Add lat and lon to the JSON response
            json << "{\"id\":" << it->first
                << ",\"name\":\"" << it->second.name
                << "\",\"lat\":" << it->second.lat
                << ",\"lon\":" << it->second.lon
                << "}";
            if (std::next(it) != stops.end()) json << ",";

        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    // API Endpoint to calculate a route
    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        // Check for required parameters
        if (!req.has_param("from") || !req.has_param("to") || !req.has_param("time")) {
            res.status = 400;
            res.set_content("{\"error\":\"Missing required parameters: from, to, time\"}", "application/json");
            return;
        }

        // Parse parameters from the URL
        int start_node = std::stoi(req.get_param_value("from"));
        int end_node = std::stoi(req.get_param_value("to"));
        std::string time_str = req.get_param_value("time");
        // --- ADD THESE DEBUGGING LINES ---
        std::cout << "--------------------------------" << std::endl;
        std::cout << "New Route Request:" << std::endl;
        std::cout << "FROM: " << start_node << " (" << getStopName(start_node, stops) << ")" << std::endl;
        std::cout << "TO:   " << end_node << " (" << getStopName(end_node, stops) << ")" << std::endl;
        std::cout << "TIME: " << time_str << std::endl;
        std::cout << "--------------------------------" << std::endl;

        // Execute the RAPTOR algorithm
        std::map<int, std::vector<Journey>> final_profiles;

        // *** FIX 1: DECLARE the predecessors map here ***
        std::map<int, std::map<int, Journey>> predecessors;

        // *** FIX 2: PASS the predecessors map to the function ***
        runMultiCriteriaRaptor(start_node, end_node, Time(time_str), stops, transfers_map, trips_map, routes_serving_stop, final_profiles, predecessors);

        // Format the result as a JSON string
        std::stringstream json;
        json << "{\"from\":\"" << getStopName(start_node, stops) << "\",\"to\":\"" << getStopName(end_node, stops) << "\",\"results\":[";

        if (final_profiles.count(end_node)) {
            const auto& results = final_profiles.at(end_node);
            for (auto it = results.begin(); it != results.end(); ++it) {
                // For each journey, reconstruct its path
                std::vector<PathStep> path = reconstructPath(start_node, end_node, *it, predecessors, stops);

                // *** THIS IS THE LINE TO CHANGE ***
                json << "{\"departure_time\":\"" << it->departure_time << "\",\"arrival_time\":\"" << it->arrival_time << "\",\"trips\":" << it->trips << ",\"path\":[";

                // Add path steps to JSON
                for (auto p_it = path.begin(); p_it != path.end(); ++p_it) {
                    json << "{\"stop_id\":" << p_it->stop_id << ", \"stop_name\":\"" << p_it->stop_name << "\", \"arrival_time\":\"" << p_it->arrival_time << "\", \"method\":\"" << p_it->method << "\"}";
                    if (std::next(p_it) != path.end()) json << ",";
                }
                json << "]}";
                if (std::next(it) != results.end()) json << ",";
            }
        }

        json << "]}";

        // Send the JSON back as the response
        res.set_content(json.str(), "application/json");
    });

    // --- 3. Start the Server ---
    std::cout << "Server starting on http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}
