# ChronoPath: A C++ Temporal Pathfinding Engine

ChronoPath is a command-line application written in modern C++ that computes the optimal journey between two points in a city's public transit system. It solves the **Earliest-Arrival Path Problem** on time-dependent graphs, where classic shortest-path algorithms fall short. This project showcases advanced graph algorithms applied to real-world data, emphasizing performance and clean architectural design.

---

## ⚡ Key Features

* **Advanced Temporal Pathfinding**

  * Implements a time-aware Dijkstra's algorithm (Profile Search) to handle networks with time-varying edge availability.

* **Multi-Modal Routing**

  * Supports walking transfers between nearby stops for practical journey computation.

* **Live ASCII Visualization**

  * An optional `--visualize` mode renders the algorithm's execution in real-time as an ASCII map in your terminal.

* **High-Performance Modern C++**

  * Built with C++17, using efficient data structures and clean, modular code architecture.

---

## 🔮 The Data Challenge

Real-world transit data is often inconsistent or unavailable. ChronoPath is designed to handle any valid **GTFS** (General Transit Feed Specification) feed, whether it's a public data source or a custom-built dataset.

---

## 📅 Prerequisites

* A C++17-compliant compiler (GCC 7+, Clang 5+)
* CMake 3.10+
* Terminal with ANSI escape code support (most Linux/macOS terminals, or Windows Terminal)

---

## 📁 Project Structure

```
chronopath/
├── CMakeLists.txt
├── main.cpp
├── include/
│   └── just_gtfs.h
├── src/
│   ├── data_model.h
│   ├── router.h
│   ├── utils.h
│   └── visualizer.h
├── data/
│   └── gtfs_patiala/  # Example dataset
└── build/
```

---

## 📀 Setup Instructions

### 1. Dependencies & Data

* **GTFS Parsing**

  * Download `just_gtfs.h` (a header-only GTFS parser) from its official repository and place it in `include/`.

* **Transit Data**

  * **Option A: Official Feed (Recommended)**

    * Download a GTFS feed (e.g., Delhi or Bengaluru) and place the `.txt` files inside a new directory like `data/gtfs_delhi/`.
  * **Option B: Custom Patiala Feed**

    * Use the sample GTFS files (stops.txt, routes.txt, etc.) provided with this project and place them in `data/gtfs_patiala/`.

* **IMPORTANT**: Update the path to the dataset in `main.cpp` depending on the data you use.

---

## ⚙️ Build Instructions

Navigate to the `build/` directory and compile using CMake:

```bash
cd chronopath/build
cmake ..
make
```

This will generate an executable named `chronopath` inside the `build/` directory.

---

## 🚀 Run Instructions

From the root directory, run the application using one of the modes below:

### Standard Mode:

```bash
./build/chronopath
```

### Visualization Mode:

```bash
./build/chronopath --visualize
```

You'll be prompted to input:

* Source stop
* Destination stop
* Departure time

---
