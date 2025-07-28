#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "just_gtfs/just_gtfs.h"
#include "utils.h"

struct Ride {
    std::string from_stop;
    std::string to_stop;
    int dep_time;
    int arr_time;
    std::string trip_id; // empty if its a walk
};

class TransitMap {
public:
    std::unordered_map<std::string, std::vector<Ride>> adj_list;
    std::unordered_map<std::string, gtfs::Stop> stops;
    std::unordered_map<std::string, gtfs::Trip> trips;
    std::unordered_map<std::string, gtfs::Route> routes;

    // loads everything from the gtfs files
    void load_from_gtfs(const std::string& path) {
        gtfs::Feed feed(path);
        if (feed.read_feed()!= gtfs::ResultCode::OK) {
            throw std::runtime_error("gtfs read failed from " + path);
        }

        // load main stuff into maps for quick access
        for (const auto& s : feed.get_stops()) {
            stops[s.stop_id] = s;
        }
        for (const auto& t : feed.get_trips()) {
            trips[t.trip_id] = t;
        }
        for (const auto& r : feed.get_routes()) {
            routes[r.route_id] = r;
        }

        // turn stop_times into connections
        // group by trip id first
        std::unordered_map<std::string, std::vector<gtfs::StopTime>> trips_to_stops;
        for (const auto& st : feed.get_stop_times()) {
            trips_to_stops[st.trip_id].push_back(st);
        }

        for (auto& pair : trips_to_stops) {
            // sort stops in a trip by sequence
            std::sort(pair.second.begin(), pair.second.end(), 
               [](const gtfs::StopTime& a, const gtfs::StopTime& b) {
                return a.stop_sequence < b.stop_sequence;
            });

            for (size_t i = 0; i + 1 < pair.second.size(); ++i) {
                const auto& from = pair.second[i];
                const auto& to = pair.second[i+1];
                adj_list[from.stop_id].push_back({
                    from.stop_id,
                    to.stop_id,
                    utils::time_to_seconds(from.departure_time),
                    utils::time_to_seconds(to.arrival_time),
                    from.trip_id
                });
            }
        }

        // add walking paths
        add_walks();

        // sort all rides by time for the main algo
        for (auto& pair : adj_list) {
            std::sort(pair.second.begin(), pair.second.end(), 
               [](const Ride& a, const Ride& b) {
                return a.dep_time < b.dep_time;
            });
        }
    }

private:
    void add_walks() {
        const double walk_speed = 1.4; // mps
        const double max_walk_dist = 500.0; // 500m

        std::vector<gtfs::Stop> all_stops;
        for(const auto& pair : stops) all_stops.push_back(pair.second);

        for (size_t i = 0; i < all_stops.size(); ++i) {
            for (size_t j = i + 1; j < all_stops.size(); ++j) {
                const auto& s1 = all_stops[i];
                const auto& s2 = all_stops[j];
                double d = utils::haversine_distance(s1.stop_lat, s1.stop_lon, s2.stop_lat, s2.stop_lon);

                if (d <= max_walk_dist) {
                    int walk_time = static_cast<int>(d / walk_speed);
                    // add walk both ways
                    adj_list[s1.stop_id].push_back({s1.stop_id, s2.stop_id, 0, walk_time, ""});
                    adj_list[s2.stop_id].push_back({s2.stop_id, s1.stop_id, 0, walk_time, ""});
                }
            }
        }
    }
};
