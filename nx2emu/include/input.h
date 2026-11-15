#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include <unordered_map>
#include "memory_map.h"

// NX2 button bitmask (matches MMIO register)
enum NXButton : uint32_t {
    NX_A      = 1 << 0,
    NX_B      = 1 << 1,
    NX_X      = 1 << 2,
    NX_Y      = 1 << 3,
    NX_L      = 1 << 4,
    NX_R      = 1 << 5,
    NX_ZL     = 1 << 6,
    NX_ZR     = 1 << 7,
    NX_PLUS   = 1 << 8,
    NX_MINUS  = 1 << 9,
    NX_UP     = 1 << 10,
    NX_DOWN   = 1 << 11,
    NX_LEFT   = 1 << 12,
    NX_RIGHT  = 1 << 13,
    NX_LS     = 1 << 14,
    NX_RS     = 1 << 15,
};

struct AxisState {
    int16_t lx = 0, ly = 0;
    int16_t rx = 0, ry = 0;
};

class InputManager {
public:
    void init();
    void destroy();

    void on_key_down(SDL_Keycode key);
    void on_key_up  (SDL_Keycode key);

    void on_controller_added  (int idx);
    void on_controller_removed(int idx);
    void on_controller_button (const SDL_ControllerButtonEvent& e);
    void on_controller_axis   (const SDL_ControllerAxisEvent& e);

    // Write current state to memory-mapped registers
    void flush_to_memory(MemoryMap& mem);

    uint32_t  button_state() const { return buttons; }
    AxisState axis_state()   const { return axes; }

private:
    uint32_t  buttons = 0;
    AxisState axes{};
    SDL_GameController* controller = nullptr;

    // Default keyboard → NX button mapping
    std::unordered_map<SDL_Keycode, NXButton> key_map = {
        { SDLK_k,      NX_A     },
        { SDLK_j,      NX_B     },
        { SDLK_i,      NX_X     },
        { SDLK_h,      NX_Y     },
        { SDLK_q,      NX_L     },
        { SDLK_e,      NX_R     },
        { SDLK_z,      NX_ZL    },
        { SDLK_c,      NX_ZR    },
        { SDLK_RETURN, NX_PLUS  },
        { SDLK_BACKSPACE, NX_MINUS },
        { SDLK_UP,     NX_UP    },
        { SDLK_DOWN,   NX_DOWN  },
        { SDLK_LEFT,   NX_LEFT  },
        { SDLK_RIGHT,  NX_RIGHT },
    };

    // SDL controller button → NX button
    NXButton sdl_btn_to_nx(uint8_t btn);
};
