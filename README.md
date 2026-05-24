# Study++ - Made for 3307 Software Engineering

Study++ was made to replace you online tools into an all in one note taking, and school management app. Runs locally on any machine supporting QMake.

## Motivation
Nowadays theres an app for everything, Notion for notes, Goodnotes for written notes, Google calendar for organization. Study++ Aims to solve this by making all education apps into a single local app. Build with QT, C++.
## Video Demo

https://www.youtube.com/watch?v=-M3_kgILIXM

## Repository Structure

- `code/` – C++ source code and tests
- `report/` – Project report, UML diagrams, and any design files
- `README.md` – This file

## Quick Start

To compile and test locally (assuming you're using CMake and GoogleTest):

```bash

mkdir build && cd build

# Generate build files
cmake ../code -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .

# Run tests (if using GoogleTest)
ctest --output-on-failure
