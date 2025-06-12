cmake_minimum_required(VERSION 3.10)
project(TankGame VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
file(GLOB_RECURSE SOURCES 
    "*.cpp"
    "common/*.cpp"
    "game_management/*.cpp"
    "constants/*.cpp"
)

# Add header files
file(GLOB_RECURSE HEADERS 
    "*.h"
    "common/*.h"
    "game_management/*.h"
    "constants/*.h"
)

# Create executable
add_executable(tank_game ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(tank_game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/game_management
    ${CMAKE_CURRENT_SOURCE_DIR}/constants
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 