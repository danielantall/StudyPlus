# Study++ - Made for 3307 Software Engineering

Study++ is an all-in-one note-taking and school management desktop app built with C++17 and Qt. It replaces multiple online tools with a single local application that runs on any machine supporting QMake.

## Video Demo
https://www.youtube.com/watch?v=-M3_kgILIXM

## Features

- **Task Management** — Create, open, and organize tasks with titles, deadlines, and completion tracking
- **Document Editor** — Create and edit text documents linked to each task; documents are auto-saved to local JSON storage
- **Pomodoro Timer** — Built-in focus timer with work/break intervals, audio alerts, and session history tracking
- **Drawing Canvas** — Freehand sketch pad for diagrams and annotations, with adjustable pen color/width and base64 export
- **Calendar View** — Visual calendar highlighting task deadlines; click any date to see tasks due that day
- **Task Selector** — Searchable, filterable task list with filters for All, Incomplete, Completed, Has Deadline, and Overdue
- **Checklists** — Add checklist items to any task and toggle them as complete
- **Local Persistence** — All data (tasks, documents, sessions) is saved locally as JSON files via the `FileSystemStorage` layer

## Quick Start

### Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| C++17 compiler | `g++` or `clang++` | Xcode Command Line Tools on macOS |
| Qt | 5 or 6 | Must include `core`, `gui`, `widgets`, and `multimedia` modules |
| QMake | Bundled with Qt | Used to generate Makefiles |
| Git | Any | To clone the repository |

### Running Locally

```bash
# Clone the repository
git clone https://github.com/danielantall/StudyPlus.git
cd StudyPlus/code/ProjectTemplate

# Generate the Makefile and compile
qmake ProjectTemplate.pro
make

# Run the app
./ProjectTemplate
```

> **Note:** On macOS you may need to run `open ProjectTemplate.app` instead if Qt builds an app bundle.


```

## Build & Test Instructions

To compile and test locally using CMake and GoogleTest:

```bash
mkdir build && cd build

# Generate build files
cmake ../code -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .

# Run tests (if using GoogleTest)
ctest --output-on-failure
```
