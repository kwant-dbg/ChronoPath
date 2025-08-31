#ifndef RAPTOR_H_INCLUDED
#define RAPTOR_H_INCLUDED

//#include <map>
#include <vector>
#include <string>
#include "DataTypes.h"
#include "robin_hood.h"
// Struct to hold a single step of a reconstructed path
struct PathStep {
    int stop_id;
    std::string stop_name;
    Time arrival_time;
    std::string method;
};

// Main algorithm function declaration
void runMultiCriteriaRaptor(int start_stop_id, int end_stop_id, const Time& start_time,
                            const robin_hood::unordered_map<int, Stop>& stops,
                            const robin_hood::unordered_map<int, std::vector<Transfer>>& transfers_map,
                            const robin_hood::unordered_map<std::string, std::vector<StopTime>>& trips_map,
                            const robin_hood::unordered_map<int, std::vector<std::string>>& routes_serving_stop,
                            const robin_hood::unordered_map<std::string, robin_hood::unordered_map<int, int>>& trip_stop_indices,
                            robin_hood::unordered_map<int, std::vector<Journey>>& final_profiles,
                            robin_hood::unordered_map<int, robin_hood::unordered_map<int, Journey>>& predecessors
                           );
#endif // RAPTOR_H_INCLUDED
