#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include "emulator.h"
#include "rom_loader.h"

int main(int argc, char* argv[]) {
    std::cout << "[NX2] Emulator v1.0 — starting...\n";

    Emulator emu;
    if (!emu.init()) {
        std::cerr << "[NX2] Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    if (argc > 1) {
        std::string path = argv[1];
        RomLoader loader;
        if (!loader.load(path, emu.get_memory())) {
            std::cerr << "[NX2] Failed to load ROM: " << path << "\n";
            emu.shutdown();
            return 1;
        }
        std::cout << "[NX2] ROM loaded: " << path << "\n";
        emu.set_rom_loaded(true);
    } else {
        std::cout << "[NX2] No ROM specified. Usage: nx2emu <game.nsp|game.xci|game.rom>\n";
    }

    emu.run();
    emu.shutdown();
    return 0;
}
