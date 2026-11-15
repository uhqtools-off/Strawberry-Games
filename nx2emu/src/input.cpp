#include "input.h"
#include <iostream>

void InputManager::init() {
    // Open first available controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "[INPUT] Controller: " << SDL_GameControllerName(controller) << "\n";
                break;
            }
        }
    }
}

void InputManager::destroy() {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}

void InputManager::on_key_down(SDL_Keycode key) {
    auto it = key_map.find(key);
    if (it != key_map.end()) buttons |= it->second;

    // WASD → left stick simulation
    if (key == SDLK_w) axes.ly = -32767;
    if (key == SDLK_s) axes.ly =  32767;
    if (key == SDLK_a) axes.lx = -32767;
    if (key == SDLK_d) axes.lx =  32767;
}

void InputManager::on_key_up(SDL_Keycode key) {
    auto it = key_map.find(key);
    if (it != key_map.end()) buttons &= ~it->second;

    if (key == SDLK_w || key == SDLK_s) axes.ly = 0;
    if (key == SDLK_a || key == SDLK_d) axes.lx = 0;
}

void InputManager::on_controller_added(int idx) {
    if (!controller && SDL_IsGameController(idx)) {
        controller = SDL_GameControllerOpen(idx);
        std::cout << "[INPUT] Controller opened: " << SDL_GameControllerName(controller) << "\n";
    }
}

void InputManager::on_controller_removed(int idx) {
    if (controller && SDL_GameControllerGetJoystick(controller) == SDL_JoystickFromInstanceID(idx)) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
        std::cout << "[INPUT] Controller closed\n";
    }
}

void InputManager::on_controller_button(const SDL_ControllerButtonEvent& e) {
    NXButton nx = sdl_btn_to_nx(e.button);
    if (nx == 0) return;
    if (e.type == SDL_CONTROLLERBUTTONDOWN) buttons |=  nx;
    else                                    buttons &= ~nx;
}

void InputManager::on_controller_axis(const SDL_ControllerAxisEvent& e) {
    static constexpr int16_t DEAD = 4000;
    int16_t val = (abs(e.value) < DEAD) ? 0 : e.value;
    switch (e.axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:  axes.lx = val; break;
        case SDL_CONTROLLER_AXIS_LEFTY:  axes.ly = val; break;
        case SDL_CONTROLLER_AXIS_RIGHTX: axes.rx = val; break;
        case SDL_CONTROLLER_AXIS_RIGHTY: axes.ry = val; break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            if (val > 8000) buttons |= NX_ZL; else buttons &= ~NX_ZL; break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            if (val > 8000) buttons |= NX_ZR; else buttons &= ~NX_ZR; break;
    }
}

void InputManager::flush_to_memory(MemoryMap& mem) {
    mem.write32(MemoryMap::REG_INPUT_BUTTONS, buttons);
    mem.write16(MemoryMap::REG_INPUT_AXIS_LX, (uint16_t)axes.lx);
    mem.write16(MemoryMap::REG_INPUT_AXIS_LY, (uint16_t)axes.ly);
    mem.write16(MemoryMap::REG_INPUT_AXIS_RX, (uint16_t)axes.rx);
    mem.write16(MemoryMap::REG_INPUT_AXIS_RY, (uint16_t)axes.ry);
    mem.write32(MemoryMap::REG_TIMER_MS, SDL_GetTicks());
}

NXButton InputManager::sdl_btn_to_nx(uint8_t btn) {
    switch (btn) {
        case SDL_CONTROLLER_BUTTON_A:             return NX_B;     // SDL A = NX B
        case SDL_CONTROLLER_BUTTON_B:             return NX_A;
        case SDL_CONTROLLER_BUTTON_X:             return NX_Y;
        case SDL_CONTROLLER_BUTTON_Y:             return NX_X;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return NX_L;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return NX_R;
        case SDL_CONTROLLER_BUTTON_START:         return NX_PLUS;
        case SDL_CONTROLLER_BUTTON_BACK:          return NX_MINUS;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:     return NX_LS;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:    return NX_RS;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:       return NX_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     return NX_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     return NX_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    return NX_RIGHT;
        default: return static_cast<NXButton>(0);
    }
}
