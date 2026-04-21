#pragma once
#include <cstdint>
#include <random>

const int KEY_COUNT = 16;
const int MEM_SIZE = 4096;
const int REG_COUNT = 16;
const int STACK_LEVELS = 16;
const int VIDEO_W = 64;
const int VIDEO_H = 32;

class Chip8 {
public:
    Chip8();
    void LoadROM(const char* filename);
    void cycle();

    uint8_t keypad[KEY_COUNT]{};
    uint32_t video[VIDEO_W * VIDEO_H]{};

private:
    uint8_t memory[MEM_SIZE]{};
    uint8_t regs[REG_COUNT]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[STACK_LEVELS]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint16_t opcode{};

    std::default_random_engine rng;
    std::uniform_int_distribution<uint8_t> randByte;

    typedef void (Chip8::*Fn)();
    Fn table[0x10];
    Fn table0[0x10];
    Fn table8[0x10];
    Fn tableE[0x10];
    Fn tableF[0x66];
    
    void op_null(); 
    void op_00E0(); // CLS
    void op_00EE(); // RET
    void op_1nnn(); // JP
    void op_2nnn(); // CALL
    void op_3xkk(); // SE
    void op_4xkk(); // SNE
    void op_5xy0(); // SE Vx Vy
    void op_6xkk(); // LD
    void op_7xkk(); // ADD
    void op_8xy0(); void op_8xy1(); void op_8xy2(); void op_8xy3();
    void op_8xy4(); void op_8xy5(); void op_8xy6(); void op_8xy7();
    void op_8xyE();
    void op_9xy0(); // SNE Vx Vy
    void op_Annn(); // LD I
    void op_Bnnn(); // JP V0+addr
    void op_Cxkk(); // RND
    void op_Dxyn(); // DRW
    void op_Ex9E(); void op_ExA1(); // SKP, SKNP
    void op_Fx07(); void op_Fx0A(); void op_Fx15(); void op_Fx18();
    void op_Fx1E(); void op_Fx29(); void op_Fx33();
    void op_Fx55(); void op_Fx65();

    void table0_dispatch();
    void table8_dispatch();
    void tableE_dispatch();
    void tableF_dispatch();
};
