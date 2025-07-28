#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <chrono>
#include "data_model.h"

namespace Ansi {
    const std::string CLEAR_SCREEN = "\033[2J";
    const std::string CURSOR_HOME = "\033[H";
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
}

class Visualizer {
public:
    // sets up the map based on stop locations
    Visualizer(const TransitMap& graph, int w, int h) : width(w), height(h) {
        grid.resize(h, std::string(w, ' '));

        double min_lat = 90.0, max_lat = -90.0;
        double min_lon = 180.0, max_lon = -180.0;

        for (const auto& p : graph.stops) {
            min_lat = std::min(min_lat, p.second.stop_lat);
            max_lat = std::max(max_lat, p.second.stop_lat);
            min_lon = std::min(min_lon, p.second.stop_lon);
            max_lon = std::max(max_lon, p.second.stop_lon);
        }

        // map all the stops to grid coords
        for (const auto& p : graph.stops) {
            int x = static_cast<int>(((p.second.stop_lon - min_lon) / (max_lon - min_lon)) * (width - 2));
            int y = static_cast<int>(((p.second.stop_lat - min_lat) / (max_lat - min_lat)) * (height - 2));
            stop_coords[p.first] = {x + 1, y + 1};
            grid[y + 1][x + 1] = '.';
        }
        
        // initial draw
        std::cout << Ansi::CLEAR_SCREEN << Ansi::CURSOR_HOME;
        draw_frame();
    }

    // updates a stop's character on the map
    void update(const std::string& stop_id, char symbol, const std::string& color) {
        if (stop_coords.count(stop_id)) {
            auto& coord = stop_coords.at(stop_id);
            // move cursor to the spot and draw
            std::cout << "\033[" << coord.y + 1 << ";" << coord.x + 1 << "H";
            std::cout << color << symbol << Ansi::RESET << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(2)); 
        }
    }

private:
    void draw_frame() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                    grid[y][x] = '#';
                }
            }
        }
        for (const auto& row : grid) {
            std::cout << row << std::endl;
        }
    }

    int width, height;
    std::vector<std::string> grid;
    std::unordered_map<std::string, std::pair<int, int>> stop_coords;
};
