#include "rom_loader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

// ── NSP magic: PFS0 ──────────────────────────────────────────────────────────
static constexpr uint8_t NSP_MAGIC[4] = { 'P', 'F', 'S', '0' };
// ── XCI magic: HEAD ──────────────────────────────────────────────────────────
static constexpr uint8_t XCI_MAGIC[4] = { 'H', 'E', 'A', 'D' };
// ── NRO magic ────────────────────────────────────────────────────────────────
static constexpr uint8_t NRO_MAGIC[4] = { 'N', 'R', 'O', '0' };

struct PFS0Header {
    uint8_t  magic[4];
    uint32_t file_count;
    uint32_t string_table_size;
    uint32_t reserved;
};

struct PFS0FileEntry {
    uint64_t data_offset;
    uint64_t data_size;
    uint32_t string_offset;
    uint32_t reserved;
};

struct NROStart {
    uint32_t unused;
    uint32_t mod_offset;
    uint8_t  pad[8];
};

struct NROHeader {
    uint8_t  magic[4];
    uint32_t version;
    uint32_t size;
    uint32_t flags;
    uint32_t text_offset, text_size;
    uint32_t ro_offset,   ro_size;
    uint32_t data_offset, data_size;
    uint32_t bss_size;
    uint8_t  reserved[0x28];
};

// ─────────────────────────────────────────────────────────────────────────────
bool RomLoader::read_file(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        std::cerr << "[ROM] Cannot open: " << path << "\n";
        return false;
    }
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(sz));
    if (!f.read(reinterpret_cast<char*>(out.data()), sz)) {
        std::cerr << "[ROM] Read error\n";
        return false;
    }
    return true;
}

RomFormat RomLoader::detect_format(const std::string& path, const uint8_t* hdr, size_t hdr_size) {
    if (hdr_size < 4) return RomFormat::UNKNOWN;

    if (std::memcmp(hdr, NSP_MAGIC, 4) == 0) return RomFormat::NSP;
    if (std::memcmp(hdr, XCI_MAGIC, 4) == 0) return RomFormat::XCI;
    if (std::memcmp(hdr, NRO_MAGIC, 4) == 0) return RomFormat::NRO;

    // Extension fallback
    std::string ext = path.substr(path.rfind('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "nsp") return RomFormat::NSP;
    if (ext == "xci") return RomFormat::XCI;
    if (ext == "nro") return RomFormat::NRO;
    if (ext == "nca") return RomFormat::NCA;
    if (ext == "iso") return RomFormat::ISO;
    if (ext == "rom") return RomFormat::ROM;

    return RomFormat::UNKNOWN;
}

bool RomLoader::load(const std::string& path, MemoryMap& mem) {
    rom_info.path = path;
    // Extract filename as title
    size_t sep = path.rfind('/');
    if (sep == std::string::npos) sep = path.rfind('\\');
    std::string fname = (sep == std::string::npos) ? path : path.substr(sep+1);
    rom_info.title = fname.substr(0, fname.rfind('.'));

    std::vector<uint8_t> data;
    if (!read_file(path, data)) return false;

    rom_info.size = data.size();
    rom_info.format = detect_format(path, data.data(), data.size());

    std::cout << "[ROM] File: " << fname
              << " | Size: " << (data.size() / 1024) << " KB\n";

    switch (rom_info.format) {
        case RomFormat::NSP: return load_nsp(path, mem);
        case RomFormat::XCI: return load_xci(path, mem);
        case RomFormat::NRO: return load_nro(path, mem);
        default:             return load_generic(path, mem);
    }
}

bool RomLoader::load_nsp(const std::string& path, MemoryMap& mem) {
    std::vector<uint8_t> data;
    if (!read_file(path, data)) return false;

    if (data.size() < sizeof(PFS0Header)) {
        std::cerr << "[NSP] File too small\n";
        return false;
    }

    const PFS0Header* hdr = reinterpret_cast<const PFS0Header*>(data.data());
    if (std::memcmp(hdr->magic, NSP_MAGIC, 4) != 0) {
        std::cerr << "[NSP] Invalid magic — trying generic load\n";
        return load_generic(path, mem);
    }

    uint32_t file_count        = hdr->file_count;
    uint32_t str_table_size    = hdr->string_table_size;
    size_t   entries_offset    = sizeof(PFS0Header);
    size_t   str_table_offset  = entries_offset + file_count * sizeof(PFS0FileEntry);
    size_t   data_base         = str_table_offset + str_table_size;

    std::cout << "[NSP] PFS0: " << file_count << " file(s)\n";

    for (uint32_t i = 0; i < file_count && i < 64; ++i) {
        const PFS0FileEntry* entry = reinterpret_cast<const PFS0FileEntry*>(
            data.data() + entries_offset + i * sizeof(PFS0FileEntry));

        const char* name = reinterpret_cast<const char*>(
            data.data() + str_table_offset + entry->string_offset);

        uint64_t off  = data_base + entry->data_offset;
        uint64_t size = entry->data_size;

        std::cout << "[NSP]   [" << i << "] " << name
                  << " offset=0x" << std::hex << off
                  << " size=" << std::dec << size/1024 << " KB\n";

        // Find the main NCA content file and load it
        std::string sname(name);
        if (sname.find(".nca") != std::string::npos && off + size <= data.size()) {
            std::cout << "[NSP] Loading NCA: " << name << "\n";
            mem.load_rom(data.data() + off, static_cast<size_t>(size));
            rom_info.format = RomFormat::NCA;
            return true;
        }
    }

    // No NCA found — load whole file as generic ROM
    std::cerr << "[NSP] No NCA found, loading raw\n";
    return mem.load_rom(data.data(), data.size());
}

bool RomLoader::load_xci(const std::string& path, MemoryMap& mem) {
    std::vector<uint8_t> data;
    if (!read_file(path, data)) return false;

    // XCI header is at 0x100, game partition at variable offset
    // Simplified: skip 0x200-byte header and load content
    static constexpr size_t XCI_HEADER_SIZE = 0x200;
    if (data.size() <= XCI_HEADER_SIZE) {
        std::cerr << "[XCI] File too small\n";
        return false;
    }

    // Read title ID from XCI header area (offset 0x110, 8 bytes)
    if (data.size() >= 0x118) {
        uint64_t tid = 0;
        std::memcpy(&tid, data.data() + 0x110, 8);
        rom_info.title_id = tid;
        std::ostringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << tid;
        std::cout << "[XCI] Title ID: " << ss.str() << "\n";
    }

    std::cout << "[XCI] Loading cartridge image (" << data.size()/1024/1024 << " MB)\n";
    return mem.load_rom(data.data() + XCI_HEADER_SIZE, data.size() - XCI_HEADER_SIZE);
}

bool RomLoader::load_nro(const std::string& path, MemoryMap& mem) {
    std::vector<uint8_t> data;
    if (!read_file(path, data)) return false;

    if (data.size() < sizeof(NROStart) + sizeof(NROHeader)) {
        std::cerr << "[NRO] File too small\n";
        return false;
    }

    // NRO header is at offset 0x10
    const NROHeader* hdr = reinterpret_cast<const NROHeader*>(data.data() + sizeof(NROStart));

    if (std::memcmp(hdr->magic, NRO_MAGIC, 4) != 0) {
        std::cerr << "[NRO] Invalid magic\n";
        return false;
    }

    std::cout << "[NRO] Homebrew: "
              << " .text=" << hdr->text_size/1024 << "KB"
              << " .ro=" << hdr->ro_size/1024 << "KB"
              << " .data=" << hdr->data_size/1024 << "KB\n";

    // Load NRO at ROM base
    return mem.load_rom(data.data(), data.size());
}

bool RomLoader::load_generic(const std::string& path, MemoryMap& mem) {
    std::vector<uint8_t> data;
    if (!read_file(path, data)) return false;
    std::cout << "[ROM] Generic load: " << data.size()/1024 << " KB\n";
    return mem.load_rom(data.data(), data.size());
}
