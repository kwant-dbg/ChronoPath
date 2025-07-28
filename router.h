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

    // so the priority queue knows to sort by earliest time
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

        // keeps track of the earliest arrival time we've found for each stop
        std::unordered_map<std::string, int> best_time;
        for (const auto& p : graph.stops) {
            best_time[p.first] = std::numeric_limits<int>::max();
        }

        // so we can backtrack and build the final path
        std::unordered_map<std::string, Ride> prev_ride;

        // pq to make sure we always check the stop with the earliest arrival time next
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

        // setting up the start point
        best_time[start_stop] = leave_at;
        pq.push({leave_at, start_stop});
        if (viz) viz->update(start_stop, 'S', Ansi::GREEN);

        while (!pq.empty()) {
            State curr = pq.top();
            pq.pop();

            // if we've already found a quicker way here just skip it
            if (curr.arr_time > best_time[curr.stop_id]) {
                continue;
            }
            
            if (viz && curr.stop_id != start_stop && curr.stop_id != end_stop) {
                viz->update(curr.stop_id, 'o', Ansi::YELLOW);
            }

            // optimization if we hit the destination we're done
            if (curr.stop_id == end_stop) {
                if(viz) viz->update(end_stop, 'E', Ansi::RED);
                break;
            }

            // now check all the rides from the current stop
            if (graph.adj.count(curr.stop_id)) {
                const auto& rides = graph.adj.at(curr.stop_id);
                
                auto it = std::lower_bound(rides.begin(), rides.end(), curr.arr_time,
                   [&](const Ride& ride, int time) {
                        if (ride.trip_id.empty()) return false;
                        int dep = ride.dep_time;
                        if (delays.count(ride.trip_id)) {
                            dep += delays.at(ride.trip_id);
                        }
                        return dep < time;
                    });

                for (auto ride_it = it; ride_it!= rides.end(); ++ride_it) {
                    auto ride = *ride_it;
                    
                    if (!ride.trip_id.empty()) {
                        
                        if (delays.count(ride.trip_id)) {
                            int delay = delays.at(ride.trip_id);
                            ride.dep_time += delay;
                            ride.arr_time += delay;
                        }

                        // gotta make sure we don't miss the bus lol
                        if (ride.dep_time >= curr.arr_time) {
                            if (ride.arr_time < best_time[ride.to]) {
                                best_time[ride.to] = ride.arr_time;
                                prev_ride[ride.to] = ride;
                                pq.push({ride.arr_time, ride.to});
                            }
                        }
                    }
                }
                
                // now deal with walking
                for (const auto& ride : rides) {
                    if (ride.trip_id.empty()) { // yup its a walk
                        int next_arr = curr.arr_time + ride.arr_time;
                        if (next_arr < best_time[ride.to]) {
                            best_time[ride.to] = next_arr;
                            Ride walk = {ride.from, ride.to, curr.arr_time, next_arr, ""};
                            prev_ride[ride.to] = walk;
                            pq.push({next_arr, ride.to});
                        }
                    }
                }
            }
        }

        std::vector<Ride> path;
        if (best_time[end_stop] == std::numeric_limits<int>::max()) {
            return path; // rip no path found
        }

        std::string curr_stop = end_stop;
        while (prev_ride.count(curr_stop)) {
            const auto& ride = prev_ride.at(curr_stop);
            path.push_back(ride);
            if (viz && ride.from != start_stop) viz->update(ride.from, '*', Ansi::CYAN);
            curr_stop = ride.from;
        }
        std::reverse(path.begin(), path.end());
        
        if (viz) std::cout << "\033[32;1H";

        return path;
    }
};
