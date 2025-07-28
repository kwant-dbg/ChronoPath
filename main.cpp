#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "data_model.h"
#include "router.h"
#include "utils.h"
#include "visualizer.h"


// find stop id by partial name match
std::string find_stop_id(const TransitMap& graph, const std::string& name) {
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    for (const auto& p : graph.stops) {
        std::string stop_name = p.second.stop_name;
        std::transform(stop_name.begin(), stop_name.end(), stop_name.begin(), ::tolower);
        if (stop_name.find(query) != std::string::npos) {
            return p.first;
        }
    }
    return ""; // no match
}

void print_path(const std::vector<Ride>& path, const TransitMap& graph) {
    if (path.empty()) {
        std::cout << "\nNo path found.\n" << std::endl;
        return;
    }

    std::cout << "\n--- Your Itinerary ---\n" << std::endl;
    int step = 1;

    for (const auto& ride : path) {
        const auto& from_stop = graph.stops.at(ride.from);
        const auto& to_stop = graph.stops.at(ride.to);

        if (!ride.trip_id.empty()) { // bus/train
            const auto& trip = graph.trips.at(ride.trip_id);
            const auto& route = graph.routes.at(trip.route_id);
            std::cout << step++ << ". " << utils::sec_to_time_hm(ride.dep_time)
                      << ": Take Route " << route.route_short_name << std::endl;
            std::cout << "   From: " << from_stop.stop_name << std::endl;
            std::cout << "   -> Arrive at " << to_stop.stop_name
                      << " at " << utils::sec_to_time_hm(ride.arr_time) << std::endl;
        } else { // walk
            int walk_mins = (ride.arr_time - ride.dep_time) / 60;
            std::cout << step++ << ". " << utils::sec_to_time_hm(ride.dep_time)
                      << ": Walk for about " << walk_mins << " minutes" << std::endl;
            std::cout << "   From: " << from_stop.stop_name << std::endl;
            std::cout << "   -> Arrive at " << to_stop.stop_name
                      << " at " << utils::sec_to_time_hm(ride.arr_time) << std::endl;
        }
        std::cout << std::endl;
    }
    
    int total_dur = (path.back().arr_time - path.front().dep_time) / 60;
    std::cout << "---" << std::endl;
    std::cout << "Total Journey Time: " << total_dur << " minutes." << std::endl;
}

int main() {
    try {
        std::cout << "Loading transit data..." << std::endl;
        TransitMap graph;
        // point this to your gtfs folder
        graph.load_from_gtfs("./data/gtfs_patiala/");
        std::cout << "Data loaded " << graph.stops.size() << " stops." << std::endl;

        Router router;
        std::string from_str, to_str, time_str;

        std::cout << "\nEnter source stop (e.g., 'Thapar'): ";
        std::getline(std::cin, from_str);

        std::cout << "Enter destination stop (e.g., 'Bus Stand'): ";
        std::getline(std::cin, to_str);

        std::cout << "Enter departure time (HH:MM:SS, e.g., 07:55:00): ";
        std::getline(std::cin, time_str);

        std::string start_id = find_stop_id(graph, from_str);
        std::string end_id = find_stop_id(graph, to_str);
        
        // check for valid stops
        if (start_id.empty() || end_id.empty()) {
            std::cerr << "Error: couldn't find one of the stops" << std::endl;
            return 1;
        }

        int dep_time = utils::time_to_sec(time_str);

        std::cout << "\nCalculating route from '" << graph.stops.at(start_id).stop_name
                  << "' to '" << graph.stops.at(end_id).stop_name << "'..." << std::endl;

        auto path = router.find_path(graph, start_id, end_id, dep_time);

        print_path(path, graph);

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
