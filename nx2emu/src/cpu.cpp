#include "cpu.h"
#include <iostream>
#include <iomanip>

CPU::CPU() { reset(); }

void CPU::reset() {
    regs.fill(0);
    sp     = RESET_SP;
    pc     = RESET_PC;
    flags  = {};
    halted = false;
}

int CPU::execute_cycles(int cycles, MemoryMap& mem) {
    int executed = 0;
    while (!halted && executed < cycles) {
        executed += execute_one(mem);
    }
    return executed;
}

int CPU::execute_one(MemoryMap& mem) {
    // Fetch 32-bit ARM64 instruction
    uint32_t insn = mem.read32(static_cast<uint32_t>(pc));
    pc += 4;

    // ARM64 top-level decode via bits[28:25]
    uint8_t op0 = (insn >> 25) & 0xF;

    switch (op0) {
        case 0b0000: // Reserved / UDF
            halted = true;
            std::cerr << "[CPU] UDF at PC=" << std::hex << (pc-4) << "\n";
            return 1;

        case 0b1000:
        case 0b1001: // Data processing — immediate
            exec_data_processing(insn, mem);
            return 1;

        case 0b1010:
        case 0b1011: // Branch / system
            exec_branch(insn, mem);
            return 1;

        case 0b0100:
        case 0b0110:
        case 0b1100:
        case 0b1110: // Load / store
            exec_load_store(insn, mem);
            return 1;

        case 0b0101:
        case 0b1101: // Data processing — register
            exec_data_processing(insn, mem);
            return 1;

        default:
            // Unimplemented — skip
            return 1;
    }
}

// ─── DATA PROCESSING (stub — ADD, SUB, MOV, AND, ORR, EOR) ───────────────────
void CPU::exec_data_processing(uint32_t insn, MemoryMap&) {
    uint8_t  rd  = insn & 0x1F;
    uint8_t  rn  = (insn >> 5) & 0x1F;
    uint32_t imm = (insn >> 10) & 0xFFF;
    uint8_t  opc = (insn >> 29) & 0x3;
    bool     sf  = (insn >> 31) & 1; // 0=32-bit, 1=64-bit

    uint64_t a = (rn < 31) ? regs[rn] : sp;
    uint64_t result = a;

    switch (opc) {
        case 0b00: result = a + imm; break; // ADD
        case 0b01: result = a + imm; flags.C = (result < a); break; // ADDS
        case 0b10: result = a - imm; break; // SUB
        case 0b11: result = a - imm; // SUBS — set flags
            flags.N = (int64_t)result < 0;
            flags.Z = result == 0;
            flags.C = a >= (uint64_t)imm;
            break;
    }

    if (!sf) result &= 0xFFFFFFFF;
    if (rd < 31) regs[rd] = result;
}

// ─── LOAD / STORE (stub — LDR, STR 64-bit unsigned offset) ───────────────────
void CPU::exec_load_store(uint32_t insn, MemoryMap& mem) {
    uint8_t  rt    = insn & 0x1F;
    uint8_t  rn    = (insn >> 5) & 0x1F;
    uint16_t imm12 = (insn >> 10) & 0xFFF;
    uint8_t  size  = (insn >> 30) & 0x3;  // 0=byte 1=half 2=word 3=dword
    bool     load  = (insn >> 22) & 1;

    uint64_t base = (rn < 31) ? regs[rn] : sp;
    uint64_t addr = base + (imm12 << size);

    if (load) {
        uint64_t val = 0;
        switch (size) {
            case 0: val = mem.read8 (static_cast<uint32_t>(addr)); break;
            case 1: val = mem.read16(static_cast<uint32_t>(addr)); break;
            case 2: val = mem.read32(static_cast<uint32_t>(addr)); break;
            case 3: // 64-bit: two 32-bit reads
                val = (uint64_t)mem.read32(static_cast<uint32_t>(addr)) |
                     ((uint64_t)mem.read32(static_cast<uint32_t>(addr+4)) << 32);
                break;
        }
        if (rt < 31) regs[rt] = val;
    } else {
        uint64_t val = (rt < 31) ? regs[rt] : 0;
        switch (size) {
            case 0: mem.write8 (static_cast<uint32_t>(addr), (uint8_t)val);  break;
            case 1: mem.write16(static_cast<uint32_t>(addr), (uint16_t)val); break;
            case 2: mem.write32(static_cast<uint32_t>(addr), (uint32_t)val); break;
            case 3:
                mem.write32(static_cast<uint32_t>(addr),   (uint32_t)val);
                mem.write32(static_cast<uint32_t>(addr+4), (uint32_t)(val>>32));
                break;
        }
    }
}

// ─── BRANCH (B, BL, B.cond, RET) ─────────────────────────────────────────────
void CPU::exec_branch(uint32_t insn, MemoryMap&) {
    uint8_t op = (insn >> 29) & 0x7;

    if (op == 0b000 || op == 0b100) {
        // B / BL — 26-bit signed offset
        int32_t off = (insn & 0x3FFFFFF) << 2;
        if (off & (1 << 27)) off |= ~((1 << 28) - 1); // sign-extend
        if (op == 0b100) regs[30] = pc; // BL saves LR
        pc = (pc - 4) + (int64_t)off;
        return;
    }

    if ((insn >> 25) == 0b1101011) {
        // BR / BLR / RET
        uint8_t rn = (insn >> 5) & 0x1F;
        uint64_t target = (rn < 31) ? regs[rn] : sp;
        uint8_t opc2 = (insn >> 21) & 0x3;
        if (opc2 == 0b01) regs[30] = pc; // BLR
        pc = target;
        return;
    }

    // B.cond — 19-bit signed offset
    if (((insn >> 24) & 0xFF) == 0b01010100) {
        uint8_t  cond = insn & 0xF;
        int32_t  off  = ((insn >> 5) & 0x7FFFF) << 2;
        if (off & (1 << 20)) off |= ~((1 << 21) - 1);

        bool taken = false;
        switch (cond >> 1) {
            case 0: taken = flags.Z;                              break; // EQ/NE
            case 1: taken = flags.C;                              break; // CS/CC
            case 2: taken = flags.N;                              break; // MI/PL
            case 3: taken = flags.V;                              break; // VS/VC
            case 4: taken = flags.C && !flags.Z;                  break; // HI/LS
            case 5: taken = flags.N == flags.V;                   break; // GE/LT
            case 6: taken = !flags.Z && flags.N == flags.V;       break; // GT/LE
            case 7: taken = true;                                  break; // AL
        }
        if (cond & 1) taken = !taken; // invert for NE, CC, PL, VC, LS, LT, LE

        if (taken)
            pc = (pc - 4) + (int64_t)off;
    }
}

// ─── SYSTEM (NOP, HLT, MSR/MRS stub) ─────────────────────────────────────────
void CPU::exec_system(uint32_t insn, MemoryMap&) {
    uint32_t op = (insn >> 5) & 0xFFFF;
    if (op == 0x015F) return;        // NOP
    if (op == 0x4000) { halted = true; return; } // HLT
    // MSR/MRS — ignore for now
}
