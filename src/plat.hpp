#pragma once
#include <SDL3/SDL.h>
#include<cstdint>

class Platform{
public:
    Platform(const char* title, int winW, int winH, int texW, int texH);
    ~Platform();
    void update(const void* buffer, int pitch);
    bool processInput(uint8_t* keys);

private:
SDL_Window* window{};
SDL_Renderer* renderer{};
SDL_Texture* texture{};
};