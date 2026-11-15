# NX2 Emulator — C++ / SDL2

Skeleton d'émulateur Nintendo Switch 2 en C++17 + SDL2.

## Ce que ça fait

| Composant | Statut |
|-----------|--------|
| Fenêtre SDL2 / rendu | ✅ |
| Boucle émulateur 60fps | ✅ |
| Chargeur ROM `.nsp` `.xci` `.nro` `.rom` `.iso` | ✅ |
| Memory map (ROM/RAM/VRAM/MMIO) | ✅ |
| Interpréteur ARM64 (ADD, SUB, LDR, STR, B, BL, B.cond) | ✅ skeleton |
| Input clavier + manette (SDL2 GameController) | ✅ |
| Flush input → registres MMIO | ✅ |
| Renderer framebuffer (texture streaming) | ✅ |
| Splash screen sans ROM | ✅ |
| Parse PFS0 (NSP) + header XCI + NRO | ✅ |

---

## Dépendances

- **CMake** ≥ 3.16
- **SDL2** ≥ 2.0.18

### Linux (Ubuntu/Debian)
```bash
sudo apt install cmake libsdl2-dev
```

### macOS (Homebrew)
```bash
brew install cmake sdl2
```

### Windows (vcpkg)
```powershell
vcpkg install sdl2:x64-windows
```

---

## Compiler

```bash
# Clone / extraire le zip
cd nx2emu

# Build Release
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Lancer sans ROM (splash)
./build/nx2emu

# Lancer avec un jeu
./build/nx2emu /chemin/vers/jeu.nsp
./build/nx2emu /chemin/vers/jeu.xci
./build/nx2emu /chemin/vers/homebrew.nro
```

---

## Touches clavier par défaut

| Touche | Bouton NX |
|--------|-----------|
| K | A |
| J | B |
| I | X |
| H | Y |
| Q | L |
| E | R |
| Z | ZL |
| C | ZR |
| Entrée | + |
| Backspace | − |
| Flèches | D-Pad |
| WASD | Stick gauche |
| F11 | Plein écran |
| Échap | Quitter |

---

## Architecture

```
nx2emu/
├── CMakeLists.txt
├── include/
│   ├── emulator.h      ← boucle principale
│   ├── cpu.h           ← ARM64 interpreter
│   ├── memory_map.h    ← ROM/RAM/VRAM/MMIO
│   ├── input.h         ← clavier + manette
│   ├── renderer.h      ← SDL2 texture streaming
│   └── rom_loader.h    ← parser NSP/XCI/NRO/ROM
└── src/
    ├── main.cpp
    ├── emulator.cpp
    ├── cpu.cpp
    ├── memory_map.cpp
    ├── input.cpp
    ├── renderer.cpp
    └── rom_loader.cpp
```

## Étapes suivantes (pour un vrai emu)

1. **JIT ARM64** — remplacer l'interpréteur par un recompilateur (dynarmic, ou custom LLVM)
2. **GPU** — implémenter Maxwell/Ampere NX via Vulkan (très long)
3. **Firmware** — charger les modules système Nintendo (keys requis)
4. **Audio** — mixer SDL2 + DSP emulation
5. **Titlekeys** — déchiffrement NSP/XCI (prod.keys / title.keys)
