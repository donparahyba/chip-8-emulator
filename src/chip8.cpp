#include "chip8.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

const uint32_t START_ADDR = 0x200;
const uint32_t FONT_ADDR = 0x050;

uint8_t fontset[80] = {
    0xF0,0x90,0x90,0x90,0xF0, // 0
    0x20,0x60,0x20,0x20,0x70, // 1
    0xF0,0x10,0xF0,0x80,0xF0, // 2
    0xF0,0x10,0xF0,0x10,0xF0, // 3
    0x90,0x90,0xF0,0x10,0x10, // 4
    0xF0,0x80,0xF0,0x10,0xF0, // 5
    0xF0,0x80,0xF0,0x90,0xF0, // 6
    0xF0,0x10,0x20,0x40,0x40, // 7
    0xF0,0x90,0xF0,0x90,0xF0, // 8
    0xF0,0x90,0xF0,0x10,0xF0, // 9
    0xF0,0x90,0xF0,0x90,0x90, // A
    0xE0,0x90,0xE0,0x90,0xE0, // B
    0xF0,0x80,0x80,0x80,0xF0, // C
    0xE0,0x90,0x90,0x90,0xE0, // D
    0xF0,0x80,0xF0,0x80,0xF0, // E
    0xF0,0x80,0xF0,0x80,0x80  // F
};

Chip8::Chip8()
    : rng(std::chrono::system_clock::now().time_since_epoch().count())
    {
        pc = START_ADDR;

        for (int i= 0; i < 80; i++)
            memory[FONT_ADDR + i] = fontset[i];

        randByte = std::uniform_int_distribution<uint8_t>(0, 255);

        for (auto& f : table) f = &Chip8::op_null;
        for (auto& f : table0) f = &Chip8::op_null;
        for (auto& f : table8) f = &Chip8::op_null;
        for (auto& f : tableE) f = &Chip8::op_null;
        for (auto& f : tableF) f = &Chip8::op_null;

        table[0x0] = &Chip8::table0_dispatch;
        table[0x1] = &Chip8::op_1nnn;
        table[0x2] = &Chip8::op_2nnn;
        table[0x3] = &Chip8::op_3xkk;
        table[0x4] = &Chip8::op_4xkk;
        table[0x5] = &Chip8::op_5xy0;
        table[0x6] = &Chip8::op_6xkk;
        table[0x7] = &Chip8::op_7xkk;
        table[0x8] = &Chip8::table8_dispatch;
        table[0x9] = &Chip8::op_9xy0;
        table[0xA] = &Chip8::op_Annn;
        table[0xB] = &Chip8::op_Bnnn;
        table[0xC] = &Chip8::op_Cxkk;
        table[0xD] = &Chip8::op_Dxyn;
        table[0xE] = &Chip8::tableE_dispatch;
        table[0xF] = &Chip8::tableF_dispatch;

        table0[0x0] = &Chip8::op_00E0;
        table0[0xE] = &Chip8::op_00EE;
        
        table8[0x0] = &Chip8::op_8xy0;
        table8[0x1] = &Chip8::op_8xy1;
        table8[0x2] = &Chip8::op_8xy2;
        table8[0x3] = &Chip8::op_8xy3;
        table8[0x4] = &Chip8::op_8xy4;
        table8[0x5] = &Chip8::op_8xy5;
        table8[0x6] = &Chip8::op_8xy6;
        table8[0x7] = &Chip8::op_8xy7;
        table8[0xE] = &Chip8::op_8xyE;

        tableE[0x1] = &Chip8::op_ExA1;
        tableE[0xE] = &Chip8::op_Ex9E;

        tableF[0x07] = &Chip8::op_Fx07;
        tableF[0x0A] = &Chip8::op_Fx0A;
        tableF[0x15] = &Chip8::op_Fx15;
        tableF[0x18] = &Chip8::op_Fx18;
        tableF[0x1E] = &Chip8::op_Fx1E;
        tableF[0x29] = &Chip8::op_Fx29;
        tableF[0x33] = &Chip8::op_Fx33;
        tableF[0x55] = &Chip8::op_Fx55;
        tableF[0x65] = &Chip8::op_Fx65;
    }

void Chip8::LoadROM(const char* filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return;

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&memory[START_ADDR]), size);
}

void Chip8::cycle(){
    opcode = (memory[pc] << 8) | memory[pc + 1];
        pc += 2;

        (this->*table[(opcode & 0xF000) >> 12])();

        if (delayTimer > 0) --delayTimer;
        if (soundTimer > 0) --soundTimer;
    }

void Chip8::table0_dispatch() {(this->*table0[opcode&0x000F])();}
void Chip8::table8_dispatch() {(this->*table8[opcode&0x000F])();}
void Chip8::tableE_dispatch() {(this->*tableE[opcode&0x000F])();}
void Chip8::tableF_dispatch() {(this->*tableF[opcode&0x00FF])();}

// CLS
void Chip8::op_00E0(){
    memset(video, 0, sizeof(video));
}

// JP
void Chip8::op_1nnn(){
    pc = opcode & 0x0FFF;
}

// CALL
void Chip8::op_2nnn(){
    stack[sp++] = pc;
    pc = opcode & 0x0FFF;
}

// RET
void Chip8::op_00EE(){
    pc = stack[--sp];
}

// LD Vx
void Chip8::op_6xkk(){
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t val = opcode & 0x00FF;
    regs[x] = val;
}

// ADD
void Chip8::op_8xy4(){
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint16_t sum = regs[x] + regs[y];
    regs[0xF] = (sum > 255) ? 1 : 0;
    regs[x] = sum & 0xFF;
}

// RND
void Chip8::op_Cxkk(){
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t mask = opcode & 0x00FF;
    regs[x] = randByte(rng) & mask;
}

// DRW
void Chip8::op_Dxyn(){
    uint8_t x = regs[(opcode & 0x0F00) >> 8] % VIDEO_W;
    uint8_t y = regs[(opcode & 0x00F0) >> 4] % VIDEO_H;
    uint8_t h = opcode & 0x000F;

    regs[0xF] = 0;

    for(int row = 0; row < h; row++){
        uint8_t spriteByte = memory[index + row];

        for (int col = 0; col < 8; col++){
            if(spriteByte & (0x80 >> col)){
                uint32_t* pixel = &video[(y + row) * VIDEO_W + (x + col)];

                if(*pixel == 0xFFFFFFFF)
                regs[0xF] = 1;

                *pixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::op_null() {}

void Chip8::op_3xkk() {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;
    if (regs[x] == kk) pc += 2;
}

void Chip8::op_4xkk() {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;
    if (regs[x] != kk) pc += 2;
}

void Chip8::op_5xy0() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    if (regs[x] == regs[y]) pc += 2;
}

void Chip8::op_7xkk() {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;
    regs[x] += kk;
}

void Chip8::op_8xy0() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[x] = regs[y];
}

void Chip8::op_8xy1() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[x] |= regs[y];
}

void Chip8::op_8xy2() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[x] &= regs[y];
}

void Chip8::op_8xy3() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[x] ^= regs[y];
}

void Chip8::op_8xy5() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[0xF] = (regs[x] > regs[y]) ? 1 : 0;
    regs[x] -= regs[y];
}

void Chip8::op_8xy6() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    regs[0xF] = regs[x] & 0x1;
    regs[x] >>= 1;
}

void Chip8::op_8xy7() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    regs[0xF] = (regs[y] > regs[x]) ? 1 : 0;
    regs[x] = regs[y] - regs[x];
}

void Chip8::op_8xyE() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    regs[0xF] = (regs[x] & 0x80) >> 7;
    regs[x] <<= 1;
}

void Chip8::op_9xy0() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    if (regs[x] != regs[y]) pc += 2;
}

void Chip8::op_Annn() {
    index = opcode & 0x0FFF;
}

void Chip8::op_Bnnn() {
    pc = regs[0] + (opcode & 0x0FFF);
}

void Chip8::op_Ex9E() {
    uint8_t x   = (opcode & 0x0F00) >> 8;
    if (keypad[regs[x]]) pc += 2;
}

void Chip8::op_ExA1() {
    uint8_t x   = (opcode & 0x0F00) >> 8;
    if (!keypad[regs[x]]) pc += 2;
}

void Chip8::op_Fx07() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    regs[x] = delayTimer;
}

void Chip8::op_Fx0A() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (int i = 0; i < 16; i++) {
        if (keypad[i]) { regs[x] = i; return; }
    }
    pc -= 2;
}

void Chip8::op_Fx15() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    delayTimer = regs[x];
}

void Chip8::op_Fx18() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    soundTimer = regs[x];
}

void Chip8::op_Fx1E() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    index += regs[x];
}

void Chip8::op_Fx29() {
    uint8_t x     = (opcode & 0x0F00) >> 8;
    uint8_t digit = regs[x];
    index = FONT_ADDR + (5 * digit);
}

void Chip8::op_Fx33() {
    uint8_t x   = (opcode & 0x0F00) >> 8;
    uint8_t val = regs[x];
    memory[index + 2] = val % 10; val /= 10;
    memory[index + 1] = val % 10; val /= 10;
    memory[index]     = val % 10;
}

void Chip8::op_Fx55() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
        memory[index + i] = regs[i];
}

void Chip8::op_Fx65() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
        regs[i] = memory[index + i];
}