### ChronoPath: A C++ Transit Route Planner

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A C++ public transit journey planner using the RAPTOR algorithm for time-dependent routing on GTFS data. Exposes a lightweight web UI for querying.

*   **Core Algorithm:** Efficient, round-based RAPTOR implementation optimized for transit networks.
*   **Data Source:** Ingests standard GTFS data feeds.
*   **Interface:** Lightweight C++ web server via `httplib.h`.

---

### Demo

| Web Interface | Server Log |
|---------------|------------|
| ![UI Screenshot](img/Screenshot%202025-08-20%20005027.png) | ![Log Screenshot](img/Screenshot%202025-08-20%20005141.png) |

---

### Quickstart

**Prerequisites:** C++17 Compiler (GCC/Clang) & GTFS data.

```bash
# 1. Clone the repository
git clone https://github.com/kwant-dbg/ChronoPath.git
cd ChronoPath

# 2. Add GTFS files (.txt) to a new `data/` directory

# 3. Compile the server
g++ Sources/main.cpp Sources/Raptor.cpp -o pathfinder -IHeaders -std=c++17 -pthread

# 4. Run the server
./pathfinder
# Access at http://localhost:8080
```

---

### Structure

```
ChronoPath/
├── Headers/
│   ├── DataTypes.h
│   ├── httplib.h
│   └── Raptor.h
└── Sources/
    ├── main.cpp
    └── Raptor.cpp
```
<img width="800" height="450" alt="Raptor(2)" src="https://github.com/user-attachments/assets/cc5a5844-d792-4f74-8ba5-efacd702a576" />
