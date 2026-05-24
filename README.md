# Study++ - Made for 3307 Software Engineering

Study++ was made to replace you online tools into an all in one note taking, and school management app. Runs locally on any machine supporting QMake.

## Quick Start

### Prerequisites

- A C++17-compatible compiler (e.g., `g++`, `clang++`)
- **CMake** (≥ 3.5) — or — **QMake** (bundled with Qt)
- Git

### Running Locally (CMake)

```bash
# Clone the repository
git clone https://github.com/danielantall/StudyPlus.git
cd StudyPlus

# Create a build directory and compile
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run the app
./assignment2
```

### Running Locally (QMake)

```bash
cd code/ProjectTemplate

# Generate the Makefile and compile
qmake ProjectTemplate.pro
make

# Run the app
./ProjectTemplate
```

## Video Demo
https://www.youtube.com/watch?v=-M3_kgILIXM

## Repository Structure

- `code/` – C++ source code and tests
- `report/` – Project report, UML diagrams, and any design files
- `README.md` – This file

## Build & Test Instructions

To compile and test locally (assuming you're using CMake and GoogleTest):

```bash

mkdir build && cd build

# Generate build files
cmake ../code -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .

# Run tests (if using GoogleTest)
ctest --output-on-failure
