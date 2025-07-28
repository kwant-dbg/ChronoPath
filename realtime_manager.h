#pragma once
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include "cpr/cpr.h"
#include "gtfs-realtime.pb.h" 

struct LiveUpdate {
    std::string trip_id;
    int delay_sec;
};

class RealtimeManager {
public:
    RealtimeManager(const std::string& url, const std::string& api_key) 
        : feed_url(url), api_key(api_key), running(true) {
        fetch_thread = std::thread(&RealtimeManager::fetch_loop, this);
    }

    ~RealtimeManager() {
        running = false;
        if (fetch_thread.joinable()) {
            fetch_thread.join();
        }
    }

    std::unordered_map<std::string, int> get_delays() {
        std::lock_guard<std::mutex> lock(mtx);
        return trip_delays;
    }

private:
    void fetch_loop() {
        while (running) {
            fetch_data();
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    }

    void fetch_data() {
        // Make the HTTP request
        cpr::Response r = cpr::Get(cpr::Url{feed_url}, cpr::Parameters{{"key", api_key}});

        if (r.status_code == 200) {
            transit_realtime::FeedMessage feed;
            if (feed.ParseFromString(r.text)) {
                std::unordered_map<std::string, int> new_delays;
                for (const auto& entity : feed.entity()) {
                    if (entity.has_trip_update()) {
                        const auto& trip_update = entity.trip_update();
                        const std::string& trip_id = trip_update.trip().trip_id();
                        
                        // Get the first delay value we find
                        if (!trip_update.stop_time_update().empty()) {
                            const auto& stop_time_update = trip_update.stop_time_update(0);
                            new_delays[trip_id] = stop_time_update.departure().delay();
                        }
                    }
                }
                
                std::lock_guard<std::mutex> lock(mtx);
                trip_delays = new_delays;
                std::cout << "[Realtime] Updated delays for " << trip_delays.size() << " trips." << std::endl;
            }
        }
    }

    std::string feed_url;
    std::string api_key;
    std::thread fetch_thread;
    std::mutex mtx;
    bool running;
    std::unordered_map<std::string, int> trip_delays;
};
