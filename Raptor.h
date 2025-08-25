#ifndef RAPTOR_H_INCLUDED
#define RAPTOR_H_INCLUDED

#include <map>
#include <vector>
#include <string>
#include "DataTypes.h"

// Struct to hold a single step of a reconstructed path
struct PathStep {
    int stop_id;
    std::string stop_name;
    Time arrival_time;
    std::string method;
};

// Main algorithm function declaration
void runMultiCriteriaRaptor(int start_stop_id, int end_stop_id, const Time& start_time,
                            const std::map<int, Stop>& stops,
                            const std::map<int, std::vector<Transfer>>& transfers_map,
                            const std::map<std::string, std::vector<StopTime>>& trips_map,
                            const std::map<int, std::vector<std::string>>& routes_serving_stop,
                            std::map<int, std::vector<Journey>>& final_profiles,
                            std::map<int, std::map<int, Journey>>& predecessors
                           );

#endif // RAPTOR_H_INCLUDED
