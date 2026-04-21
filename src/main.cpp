#include "chip8.hpp"
#include "plat.hpp"
#include <chrono>

int main(int argc, char** argv) {
    if (argc != 4) {
        return 1;
    }

    int scale  = std::stoi(argv[1]);
    int delay  = std::stoi(argv[2]);
    const char* rom = argv[3];

    Platform platform("CHIP-8", VIDEO_W * scale, VIDEO_H * scale, VIDEO_W, VIDEO_H);
    Chip8 chip8;
    chip8.LoadROM(rom);

    int pitch = sizeof(chip8.video[0]) * 64;
    auto last = std::chrono::high_resolution_clock::now();

    while (true) {
        if (platform.processInput(chip8.keypad)) break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(now - last).count();

        if (dt > delay) {
            last = now;
            chip8.cycle();
            platform.update(chip8.video, pitch);
        }
    }
}