#include "emulator.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Emulator::Emulator() = default;
Emulator::~Emulator() = default;

bool Emulator::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
        std::cerr << "[EMU] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow(
        "NX2 Emulator v1.0",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) return false;

    renderer_sdl = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_sdl) return false;

    // Init sub-systems
    if (!gfx.init(renderer_sdl, SCREEN_W, SCREEN_H)) {
        std::cerr << "[EMU] Renderer init failed\n";
        return false;
    }

    if (!memory.init()) {
        std::cerr << "[EMU] Memory init failed\n";
        return false;
    }

    input.init();
    cpu.reset();

    running   = true;
    last_tick = SDL_GetTicks64();

    std::cout << "[EMU] Init OK — " << SCREEN_W << "x" << SCREEN_H << " @ " << TARGET_FPS << "fps\n";
    return true;
}

void Emulator::run() {
    while (running) {
        uint64_t now   = SDL_GetTicks64();
        double   delta = static_cast<double>(now - last_tick);
        last_tick      = now;

        handle_events();
        update();
        render();

        // FPS counter
        fps_acc   += delta;
        fps_count += 1;
        if (fps_acc >= 500.0) {
            current_fps = fps_count / (fps_acc / 1000.0);
            fps_acc     = 0.0;
            fps_count   = 0;
            update_title(current_fps);
        }

        // Frame limiter (fallback when vsync is off)
        double elapsed = static_cast<double>(SDL_GetTicks64() - now);
        if (elapsed < FRAME_TIME)
            SDL_Delay(static_cast<uint32_t>(FRAME_TIME - elapsed));
    }
}

void Emulator::handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                input.on_key_down(e.key.keysym.sym);
                // Hotkeys
                if (e.key.keysym.sym == SDLK_ESCAPE)  running = false;
                if (e.key.keysym.sym == SDLK_F11)
                    SDL_SetWindowFullscreen(window,
                        SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN
                            ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                break;

            case SDL_KEYUP:
                input.on_key_up(e.key.keysym.sym);
                break;

            case SDL_CONTROLLERDEVICEADDED:
                input.on_controller_added(e.cdevice.which);
                std::cout << "[INPUT] Controller connected: " << e.cdevice.which << "\n";
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                input.on_controller_removed(e.cdevice.which);
                std::cout << "[INPUT] Controller disconnected\n";
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                input.on_controller_button(e.cbutton);
                break;

            case SDL_CONTROLLERAXISMOTION:
                input.on_controller_axis(e.caxis);
                break;

            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                    gfx.on_resize(e.window.data1, e.window.data2);
                break;
        }
    }
}

void Emulator::update() {
    if (!rom_loaded) return;

    // Execute CPU cycles per frame (ARM64 stub — ~1000 cycles/frame placeholder)
    constexpr int CYCLES_PER_FRAME = 1000;
    cpu.execute_cycles(CYCLES_PER_FRAME, memory);

    // Push input state to memory-mapped registers
    input.flush_to_memory(memory);
}

void Emulator::render() {
    SDL_SetRenderDrawColor(renderer_sdl, 10, 10, 15, 255);
    SDL_RenderClear(renderer_sdl);

    if (rom_loaded)
        gfx.draw_framebuffer();
    else
        gfx.draw_splash(window);

    SDL_RenderPresent(renderer_sdl);
}

void Emulator::update_title(double fps) {
    std::ostringstream ss;
    ss << "NX2 Emulator v1.0";
    if (rom_loaded) ss << " — " << std::fixed << std::setprecision(1) << fps << " fps";
    SDL_SetWindowTitle(window, ss.str().c_str());
}

void Emulator::shutdown() {
    std::cout << "[EMU] Shutting down...\n";
    gfx.destroy();
    input.destroy();
    memory.destroy();
    if (renderer_sdl) SDL_DestroyRenderer(renderer_sdl);
    if (window)       SDL_DestroyWindow(window);
    SDL_Quit();
}
