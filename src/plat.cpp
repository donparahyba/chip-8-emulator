#include "plat.hpp"

Platform::Platform(const char* title, int winW, int winH, int texW, int texH) {
    SDL_Init(SDL_INIT_VIDEO);
    window   = SDL_CreateWindow(title, winW, winH, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, texW, texH);
}

Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::update(const void* buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Platform::processInput(uint8_t* keys) {
    SDL_Event e;
    bool quit = false;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) quit = true;

        auto handle = [&](SDL_EventType type, uint8_t val) {
            if (e.type != type) return;
            switch (e.key.key) {
                case SDLK_1: keys[1] = val; break;
                case SDLK_2: keys[2] = val; break;
                case SDLK_3: keys[3] = val; break;
                case SDLK_4: keys[0xC] = val; break;
                case SDLK_Q: keys[4] = val; break;
                case SDLK_W: keys[5] = val; break;
                case SDLK_E: keys[6] = val; break;
                case SDLK_R: keys[0xD] = val; break;
                case SDLK_A: keys[7] = val; break;
                case SDLK_S: keys[8] = val; break;
                case SDLK_D: keys[9] = val; break;
                case SDLK_F: keys[0xE] = val; break;
                case SDLK_Z: keys[0xA] = val; break;
                case SDLK_X: keys[0] = val; break;
                case SDLK_C: keys[0xB] = val; break;
                case SDLK_V: keys[0xF] = val; break;
                case SDLK_ESCAPE: quit = true; break;
            }
        };
        handle(SDL_EVENT_KEY_DOWN, 1);
        handle(SDL_EVENT_KEY_UP,   0);
    }
    return quit;
}