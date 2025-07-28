#pragma once

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include "data_model.h"
#include "visualizer.h"

struct State {
    int arr_time;
    std::string stop_id;

    bool operator>(const State& other) const {
        return arr_time > other.arr_time;
    }
};

class Router {
public:
    std::vector<Ride> find_path(
        const TransitMap& graph,
        const std::string& start_stop,
        const std::string& end_stop,
        int leave_at,
        const std::unordered_map<std::string, int>& delays,
        Visualizer* viz = nullptr) {

        std::unordered_map<std::string, int> best_time;
        for (const auto& [stop_id, stop] : graph.stops) {
            best_time[stop_id] = std::numeric_limits<int>::max();
        }

        std::unordered_map<std::string, Ride> prev_ride;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

        best_time[start_stop] = leave_at;
        pq.push({leave_at, start_stop});
        if (viz) viz->update(start_stop, 'S', Ansi::GREEN);

        while (!pq.empty()) {
            State curr = pq.top();
            pq.pop();

            if (curr.arr_time > best_time[curr.stop_id]) {
                continue;
            }

            if (curr.stop_id == end_stop) {
                if(viz) viz->update(end_stop, 'E', Ansi::RED);
                break;
            }
            
            if (viz && curr.stop_id != start_stop) {
                viz->update(curr.stop_id, 'o', Ansi::YELLOW);
            }

            if (!graph.adj_list.count(curr.stop_id)) {
                continue;
            }

            const auto& rides = graph.adj_list.at(curr.stop_id);
            for (const auto& ride : rides) {
                if (!ride.trip_id.empty()) { // Transit ride
                    int effective_dep_time = ride.dep_time;
                    int effective_arr_time = ride.arr_time;

                    if (delays.count(ride.trip_id)) {
                        int delay = delays.at(ride.trip_id);
                        effective_dep_time += delay;
                        effective_arr_time += delay;
                    }

                    if (effective_dep_time >= curr.arr_time && effective_arr_time < best_time[ride.to_stop]) {
                        best_time[ride.to_stop] = effective_arr_time;
                        prev_ride[ride.to_stop] = {ride.from_stop, ride.to_stop, effective_dep_time, effective_arr_time, ride.trip_id};
                        pq.push({effective_arr_time, ride.to_stop});
                    }
                } else { // Walking transfer
                    int arrival_if_walking = curr.arr_time + ride.arr_time; // arr_time is duration for walks
                    if (arrival_if_walking < best_time[ride.to_stop]) {
                        best_time[ride.to_stop] = arrival_if_walking;
                        prev_ride[ride.to_stop] = {ride.from_stop, ride.to_stop, curr.arr_time, arrival_if_walking, ""};
                        pq.push({arrival_if_walking, ride.to_stop});
                    }
                }
            }
        }

        if (best_time[end_stop] == std::numeric_limits<int>::max()) {
            return {}; // No path found
        }

        std::vector<Ride> path;
        std::string curr_stop = end_stop;
        while (prev_ride.count(curr_stop)) {
            const auto& ride = prev_ride.at(curr_stop);
            path.push_back(ride);
            if (viz && ride.from_stop != start_stop) viz->update(ride.from_stop, '*', Ansi::CYAN);
            curr_stop = ride.from_stop;
        }
        std::reverse(path.begin(), path.end());
        
        if (viz) std::cout << "\033[32;1H";

        return path;
    }
};
