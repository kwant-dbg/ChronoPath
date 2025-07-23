#pragma once

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include "data_model.h"

// pq state
struct State {
    int arrival_time;
    std::string stop_id;

    // so the priority queue knows to sort by earliest time
    bool operator>(const State& other) const {
        return arrival_time > other.arrival_time;
    }
};

class Router {
public:
    std::vector<Connection> find_earliest_arrival_path(
        const TransitGraph& the_map,
        const std::string& start_stop,
        const std::string& end_stop,
        int leave_at) {

        // keeps track of the earliest arrival time we've found for each stop
        std::unordered_map<std::string, int> best_time_to_stop;
        for (const auto& pair : the_map.stops) {
            best_time_to_stop[pair.first] = std::numeric_limits<int>::max();
        }

        //  backtrack and build the final path
        std::unordered_map<std::string, Connection> how_we_got_here;

        std::priority_queue<State, std::vector<State>, std::greater<State>> q;

        // setting up the start point
        best_time_to_stop[start_stop] = leave_at;
        q.push({leave_at, start_stop});

        while (!q.empty()) {
            State curr = q.top();
            q.pop();

            // if we've already found a quicker way here just skip it
            if (curr.arrival_time > best_time_to_stop[curr.stop_id]) {
                continue;
            }
            
            // optimization if we hit the destination we're done
            if (curr.stop_id == end_stop) {
                break;
            }

            // now check all the rides from the current stop
            if (the_map.adj.count(curr.stop_id)) {
                const auto& rides_from_here = the_map.adj.at(curr.stop_id);
                
                // for buses/trains find the next available ride after we get to the stop
                // ez binary search
                auto it = std::lower_bound(rides_from_here.begin(), rides_from_here.end(), curr.arrival_time,
                   [](const Connection& ride, int time) {
                        if (!ride.trip_id.empty()) {
                            return ride.departure_time < time;
                        }
                        return false;
                    });

                for (auto ride_it = it; ride_it!= rides_from_here.end(); ++ride_it) {
                    const auto& ride = *ride_it;
                    
                    // only for transit rides
                    if (!ride.trip_id.empty()) {
                        // gotta make sure we don't miss the bus lol
                        if (ride.departure_time >= curr.arrival_time) {
                            if (ride.arrival_time < best_time_to_stop[ride.to_stop_id]) {
                                best_time_to_stop[ride.to_stop_id] = ride.arrival_time;
                                how_we_got_here[ride.to_stop_id] = ride;
                                q.push({ride.arrival_time, ride.to_stop_id});
                            }
                        }
                    }
                }
                
                // walking
                for (const auto& ride : rides_from_here) {
                    if (ride.trip_id.empty()) { // yup its a walk
                        int next_stop_arrival = curr.arrival_time + ride.arrival_time; // ride.arrival_time is walk duration
                        if (next_stop_arrival < best_time_to_stop[ride.to_stop_id]) {
                            best_time_to_stop[ride.to_stop_id] = next_stop_arrival;
                            // gotta make a temp walk connection to store it
                            Connection this_walk = {ride.from_stop_id, ride.to_stop_id, curr.arrival_time, next_stop_arrival, ""};
                            how_we_got_here[ride.to_stop_id] = this_walk;
                            q.push({next_stop_arrival, ride.to_stop_id});
                        }
                    }
                }
            }
        }

        // okay all done now lets build the path backwards from the end
        std::vector<Connection> final_path;
        if (best_time_to_stop[end_stop] == std::numeric_limits<int>::max()) {
            return final_path; // rip no path found
        }

        std::string curr_stop = end_stop;
        while (how_we_got_here.count(curr_stop)) {
            const auto& ride = how_we_got_here.at(curr_stop);
            final_path.push_back(ride);
            curr_stop = ride.from_stop_id;
        }
        std::reverse(final_path.begin(), final_path.end());
        return final_path;
    }
};
