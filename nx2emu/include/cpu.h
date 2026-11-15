#pragma once
#include <cstdint>
#include <array>
#include "memory_map.h"

/*
 * ARM64 (AArch64) CPU stub
 * Registers: X0–X30 (64-bit general purpose), SP, PC, NZCV flags
 * This is a skeleton — instruction decoding is stubbed.
 * A full ARM64 JIT would go in cpu_jit.cpp.
 */

struct NZCV {
    bool N = false; // Negative
    bool Z = false; // Zero
    bool C = false; // Carry
    bool V = false; // Overflow
};

class CPU {
public:
    CPU();
    void reset();

    // Execute up to `cycles` instructions
    int execute_cycles(int cycles, MemoryMap& mem);

    // Register access
    uint64_t get_reg(int idx) const { return (idx < 31) ? regs[idx] : 0; }
    void     set_reg(int idx, uint64_t val) { if (idx < 31) regs[idx] = val; }

    uint64_t get_pc()  const { return pc; }
    uint64_t get_sp()  const { return sp; }
    NZCV     get_flags() const { return flags; }

    bool is_halted() const { return halted; }

private:
    // Execute one instruction, return cycles consumed
    int execute_one(MemoryMap& mem);

    // Instruction decode helpers
    void exec_data_processing(uint32_t insn, MemoryMap& mem);
    void exec_load_store     (uint32_t insn, MemoryMap& mem);
    void exec_branch         (uint32_t insn, MemoryMap& mem);
    void exec_system         (uint32_t insn, MemoryMap& mem);

    // General-purpose registers X0–X30
    std::array<uint64_t, 31> regs{};
    uint64_t sp  = 0;
    uint64_t pc  = 0;
    NZCV     flags{};
    bool     halted = false;

    static constexpr uint64_t RESET_PC = 0x00000000; // ROM base
    static constexpr uint64_t RESET_SP = 0x17FFFFFF; // top of RAM
};
