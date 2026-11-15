#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>
#include <string>
#include "cpu.h"
#include "memory_map.h"
#include "input.h"
#include "renderer.h"

class Emulator {
public:
    Emulator();
    ~Emulator();

    bool init();
    void run();
    void shutdown();

    MemoryMap& get_memory() { return memory; }
    void set_rom_loaded(bool v) { rom_loaded = v; }

private:
    void handle_events();
    void update();
    void render();
    void update_title(double fps);

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer_sdl = nullptr;

    CPU        cpu;
    MemoryMap  memory;
    InputManager input;
    Renderer   gfx;

    bool running    = false;
    bool rom_loaded = false;

    // timing
    uint64_t last_tick = 0;
    double   fps_acc   = 0.0;
    int      fps_count = 0;
    double   current_fps = 0.0;

    // config
    static constexpr int   SCREEN_W    = 1280;
    static constexpr int   SCREEN_H    = 720;
    static constexpr int   TARGET_FPS  = 60;
    static constexpr double FRAME_TIME = 1000.0 / TARGET_FPS;
};
