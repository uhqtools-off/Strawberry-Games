cmake_minimum_required(VERSION 3.16)
project(nx2emu VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Release flags ──────────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ── SDL2 ──────────────────────────────────────────────────────────────────────
find_package(SDL2 REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/emulator.cpp
    src/cpu.cpp
    src/memory_map.cpp
    src/input.cpp
    src/renderer.cpp
    src/rom_loader.cpp
)

add_executable(nx2emu ${SOURCES})

target_include_directories(nx2emu PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(nx2emu PRIVATE ${SDL2_LIBRARIES})

# ── Platform specifics ────────────────────────────────────────────────────────
if(WIN32)
    # Link SDL2main on Windows
    target_link_libraries(nx2emu PRIVATE SDL2::SDL2main)
    # Remove console window in Release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(nx2emu PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(nx2emu PRIVATE dl pthread)
endif()

# ── Optimization flags ────────────────────────────────────────────────────────
target_compile_options(nx2emu PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address>
)

target_link_options(nx2emu PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SDL2: ${SDL2_INCLUDE_DIRS}")
