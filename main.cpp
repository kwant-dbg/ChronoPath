#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include "data_model.h"
#include "router.h"
#include "utils.h"
#include "visualizer.h"
#include "realtime_manager.h"

std::string find_stop_id_by_substr(const TransitMap& graph, const std::string& name_substr) {
    std::string query = name_substr;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    for (const auto& [id, stop] : graph.stops) {
        std::string stop_name = stop.stop_name;
        std::transform(stop_name.begin(), stop_name.end(), stop_name.begin(), ::tolower);
        if (stop_name.find(query) != std::string::npos) {
            return id;
        }
    }
    return "";
}

void display_itinerary(const std::vector<Ride>& path, const TransitMap& graph) {
    if (path.empty()) {
        std::cout << "\nNo path found.\n";
        return;
    }

    std::cout << "\n--- Itinerary ---\n\n";
    for (size_t i = 0; i < path.size(); ++i) {
        const auto& ride = path[i];
        const auto& from = graph.stops.at(ride.from_stop);
        const auto& to = graph.stops.at(ride.to_stop);

        std::cout << i + 1 << ". ";
        if (!ride.trip_id.empty()) {
            const auto& route = graph.routes.at(graph.trips.at(ride.trip_id).route_id);
            std::cout << "At " << utils::seconds_to_time(ride.dep_time)
                      << ", take Route " << route.route_short_name
                      << " from " << from.stop_name << " to " << to.stop_name
                      << " (Arrive at " << utils::seconds_to_time(ride.arr_time) << ")\n";
        } else {
            int duration_min = (ride.arr_time - ride.dep_time) / 60;
            std::cout << "Walk for " << duration_min << " min from "
                      << from.stop_name << " to " << to.stop_name
                      << " (Arrive at " << utils::seconds_to_time(ride.arr_time) << ")\n";
        }
    }
    
    int total_duration_min = (path.back().arr_time - path.front().dep_time) / 60;
    std::cout << "\n---" << std::endl;
    std::cout << "Total Journey Time: " << total_duration_min << " minutes." << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        bool visualize = (argc > 1 && std::string(argv[1]) == "--visualize");

        RealtimeManager realtime_manager("YOUR_REALTIME_FEED_URL", "YOUR_API_KEY");
        
        std::cout << "Loading transit data..." << std::flush;
        TransitMap graph;
        graph.load_from_gtfs("./data/gtfs_patiala/");
        std::cout << " Done. (" << graph.stops.size() << " stops)\n";

        std::string from_query, to_query, time_str;
        std::cout << "Source stop: ";
        std::getline(std::cin, from_query);
        std::cout << "Destination stop: ";
        std::getline(std::cin, to_query);
        std::cout << "Departure time (HH:MM:SS): ";
        std::getline(std::cin, time_str);

        std::string start_id = find_stop_id_by_substr(graph, from_query);
        std::string end_id = find_stop_id_by_substr(graph, to_query);
        
        if (start_id.empty() || end_id.empty()) {
            std::cerr << "Error: A specified stop could not be found." << std::endl;
            return 1;
        }

        int dep_time_sec = utils::time_to_seconds(time_str);

        std::cout << "\nCalculating route..." << std::flush;

        Visualizer* viz_ptr = visualize ? new Visualizer(graph, 80, 24) : nullptr;
        
        auto delays = realtime_manager.get_delays();
        auto path = Router().find_path(graph, start_id, end_id, dep_time_sec, delays, viz_ptr);

        std::cout << " Done.\n";

        if (viz_ptr) delete viz_ptr;

        display_itinerary(path, graph);

    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
