#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <string>

/*
 * NX2 Memory Map (simplified)
 *
 * 0x00000000 - 0x0FFFFFFF  ROM / cartridge data   (256 MB)
 * 0x10000000 - 0x17FFFFFF  System RAM              (128 MB)
 * 0x18000000 - 0x1FFFFFFF  VRAM / framebuffer      (128 MB)
 * 0x20000000 - 0x200FFFFF  MMIO (input, timers…)   (1 MB)
 * 0x40000000 - 0x7FFFFFFF  Shared memory / stack   (1 GB)
 */

class MemoryMap {
public:
    static constexpr size_t ROM_SIZE    = 256 * 1024 * 1024ULL;  // 256 MB
    static constexpr size_t RAM_SIZE    = 128 * 1024 * 1024ULL;  // 128 MB
    static constexpr size_t VRAM_SIZE   = 128 * 1024 * 1024ULL;  // 128 MB
    static constexpr size_t MMIO_SIZE   = 1   * 1024 * 1024ULL;  // 1 MB

    static constexpr uint32_t ROM_BASE  = 0x00000000;
    static constexpr uint32_t RAM_BASE  = 0x10000000;
    static constexpr uint32_t VRAM_BASE = 0x18000000;
    static constexpr uint32_t MMIO_BASE = 0x20000000;

    // MMIO registers offsets
    static constexpr uint32_t REG_INPUT_BUTTONS = 0x20000000;
    static constexpr uint32_t REG_INPUT_AXIS_LX = 0x20000004;
    static constexpr uint32_t REG_INPUT_AXIS_LY = 0x20000008;
    static constexpr uint32_t REG_INPUT_AXIS_RX = 0x2000000C;
    static constexpr uint32_t REG_INPUT_AXIS_RY = 0x20000010;
    static constexpr uint32_t REG_TIMER_MS       = 0x20000100;

    bool init();
    void destroy();
    void reset();

    // Raw access
    uint8_t  read8 (uint32_t addr) const;
    uint16_t read16(uint32_t addr) const;
    uint32_t read32(uint32_t addr) const;
    void     write8 (uint32_t addr, uint8_t  val);
    void     write16(uint32_t addr, uint16_t val);
    void     write32(uint32_t addr, uint32_t val);

    // ROM loading
    bool load_rom(const uint8_t* data, size_t size);

    // VRAM direct pointer (for renderer)
    uint8_t* vram_ptr() { return vram.data(); }
    const uint8_t* vram_ptr() const { return vram.data(); }

private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
    std::vector<uint8_t> vram;
    std::vector<uint8_t> mmio;

    uint8_t*       region_ptr(uint32_t addr, size_t len = 1);
    const uint8_t* region_ptr(uint32_t addr, size_t len = 1) const;
};
