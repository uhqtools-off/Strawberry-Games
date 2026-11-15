#include "memory_map.h"
#include <cstring>
#include <stdexcept>
#include <iostream>

bool MemoryMap::init() {
    rom.assign(ROM_SIZE,  0);
    ram.assign(RAM_SIZE,  0);
    vram.assign(VRAM_SIZE, 0);
    mmio.assign(MMIO_SIZE, 0);
    std::cout << "[MEM] Allocated: ROM=" << ROM_SIZE/1024/1024 << "MB"
              << " RAM=" << RAM_SIZE/1024/1024 << "MB"
              << " VRAM=" << VRAM_SIZE/1024/1024 << "MB\n";
    return true;
}

void MemoryMap::destroy() {
    rom.clear();  rom.shrink_to_fit();
    ram.clear();  ram.shrink_to_fit();
    vram.clear(); vram.shrink_to_fit();
    mmio.clear(); mmio.shrink_to_fit();
}

void MemoryMap::reset() {
    std::fill(ram.begin(),  ram.end(),  0);
    std::fill(vram.begin(), vram.end(), 0);
    std::fill(mmio.begin(), mmio.end(), 0);
}

bool MemoryMap::load_rom(const uint8_t* data, size_t size) {
    if (!data || size == 0) return false;
    if (size > ROM_SIZE) {
        std::cerr << "[MEM] ROM too large: " << size << " bytes (max " << ROM_SIZE << ")\n";
        // Load first chunk only
        size = ROM_SIZE;
    }
    std::memcpy(rom.data(), data, size);
    std::cout << "[MEM] ROM loaded: " << size / 1024 << " KB\n";
    return true;
}

uint8_t* MemoryMap::region_ptr(uint32_t addr, size_t len) {
    if (addr >= ROM_BASE && addr + len <= ROM_BASE + ROM_SIZE)
        return rom.data() + (addr - ROM_BASE);
    if (addr >= RAM_BASE && addr + len <= RAM_BASE + RAM_SIZE)
        return ram.data() + (addr - RAM_BASE);
    if (addr >= VRAM_BASE && addr + len <= VRAM_BASE + VRAM_SIZE)
        return vram.data() + (addr - VRAM_BASE);
    if (addr >= MMIO_BASE && addr + len <= MMIO_BASE + MMIO_SIZE)
        return mmio.data() + (addr - MMIO_BASE);
    return nullptr;
}

const uint8_t* MemoryMap::region_ptr(uint32_t addr, size_t len) const {
    return const_cast<MemoryMap*>(this)->region_ptr(addr, len);
}

uint8_t MemoryMap::read8(uint32_t addr) const {
    const uint8_t* p = region_ptr(addr, 1);
    return p ? *p : 0xFF;
}

uint16_t MemoryMap::read16(uint32_t addr) const {
    const uint8_t* p = region_ptr(addr, 2);
    if (!p) return 0xFFFF;
    uint16_t v; std::memcpy(&v, p, 2); return v;
}

uint32_t MemoryMap::read32(uint32_t addr) const {
    const uint8_t* p = region_ptr(addr, 4);
    if (!p) return 0xFFFFFFFF;
    uint32_t v; std::memcpy(&v, p, 4); return v;
}

void MemoryMap::write8(uint32_t addr, uint8_t val) {
    uint8_t* p = region_ptr(addr, 1);
    if (p) *p = val;
}

void MemoryMap::write16(uint32_t addr, uint16_t val) {
    uint8_t* p = region_ptr(addr, 2);
    if (p) std::memcpy(p, &val, 2);
}

void MemoryMap::write32(uint32_t addr, uint32_t val) {
    uint8_t* p = region_ptr(addr, 4);
    if (p) std::memcpy(p, &val, 4);
}
