#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "memory_map.h"

enum class RomFormat {
    UNKNOWN,
    NSP,   // Nintendo Submission Package
    XCI,   // NX Card Image
    NRO,   // NX Relocatable Object (homebrew)
    NCA,   // NX Content Archive
    ROM,   // Generic / legacy ROM
    ISO,   // ISO image
};

struct RomInfo {
    std::string   path;
    std::string   title;
    RomFormat     format  = RomFormat::UNKNOWN;
    size_t        size    = 0;
    uint64_t      title_id = 0;
    std::string   version;
    std::string   lang;
};

class RomLoader {
public:
    bool load(const std::string& path, MemoryMap& mem);
    const RomInfo& info() const { return rom_info; }

private:
    bool load_nsp(const std::string& path, MemoryMap& mem);
    bool load_xci(const std::string& path, MemoryMap& mem);
    bool load_nro(const std::string& path, MemoryMap& mem);
    bool load_generic(const std::string& path, MemoryMap& mem);

    RomFormat detect_format(const std::string& path, const uint8_t* header, size_t header_size);
    bool      read_file(const std::string& path, std::vector<uint8_t>& out);

    RomInfo rom_info;
};
