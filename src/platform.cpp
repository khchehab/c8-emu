#include "platform.h"
#include <stdexcept>
#include <iostream>

Platform::Platform(const std::string &title, int scale) : mScale(scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error(std::string("could not initialize SDL! SDL Error: ") + SDL_GetError());
    }

    mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               GRAPHICS_WIDTH * mScale, GRAPHICS_HEIGHT * mScale, SDL_WINDOW_SHOWN);
    if (mWindow == nullptr) {
        throw std::runtime_error(std::string("could not create window! SDL Error: ") + SDL_GetError());
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == nullptr) {
        throw std::runtime_error(std::string("could not create renderer! SDL Error: ") + SDL_GetError());
    }

    mSetColor = {0x00, 0x00, 0x00};
    mUnsetColor = {0xff, 0xff, 0xff};
}

Platform::~Platform() {
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);

    SDL_Quit();
}

int eventFilterForWait(void *userdata, SDL_Event *e) {
    if (e->type == SDL_QUIT || e->type == SDL_KEYDOWN | e->type == SDL_KEYUP) {
        return 1;
    }
    return 0;
}

int eventFilterAllowAll(void *userdata, SDL_Event *e) {
    return 1;
}

bool Platform::processInput(uint8_t *keys, bool shouldWaitForKeyPress) {
    SDL_Event e;

    if (shouldWaitForKeyPress) {
        SDL_SetEventFilter(eventFilterForWait, nullptr);
    } else {
        SDL_SetEventFilter(eventFilterAllowAll, nullptr);
    }

    bool keyPressed = false;

    while (true) {
        int eventResult;
        if (shouldWaitForKeyPress) {
            eventResult = SDL_WaitEvent((&e));
        } else {
            eventResult = SDL_PollEvent((&e));
        }

        if (!eventResult) {
            break;
        }

        if (e.type == SDL_QUIT) {
            return false;
        }

        /*
         * The keyboard layout, which will be mapped to keys 1 to 4 horizontally and 1 to z vertically,
         * is as follows:
         * | 1 | 2 | 3 | C |    | 1 | 2 | 3 | 4 |
         * | 4 | 5 | 6 | D | -> | Q | W | E | R |
         * | 7 | 8 | 9 | E | -> | A | S | D | F |
         * | A | 0 | B | F |    | Z | X | C | V |
         */
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: return false;
                case SDLK_x: keys[0x0] = true; break;
                case SDLK_1: keys[0x1] = true; break;
                case SDLK_2: keys[0x2] = true; break;
                case SDLK_3: keys[0x3] = true; break;
                case SDLK_q: keys[0x4] = true; break;
                case SDLK_w: keys[0x5] = true; break;
                case SDLK_e: keys[0x6] = true; break;
                case SDLK_a: keys[0x7] = true; break;
                case SDLK_s: keys[0x8] = true; break;
                case SDLK_d: keys[0x9] = true; break;
                case SDLK_z: keys[0xa] = true; break;
                case SDLK_c: keys[0xb] = true; break;
                case SDLK_4: keys[0xc] = true; break;
                case SDLK_r: keys[0xd] = true; break;
                case SDLK_f: keys[0xe] = true; break;
                case SDLK_v: keys[0xf] = true; break;
                default: break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: return false;
                case SDLK_x: keys[0x0] = false; keyPressed = true; break;
                case SDLK_1: keys[0x1] = false; keyPressed = true; break;
                case SDLK_2: keys[0x2] = false; keyPressed = true; break;
                case SDLK_3: keys[0x3] = false; keyPressed = true; break;
                case SDLK_q: keys[0x4] = false; keyPressed = true; break;
                case SDLK_w: keys[0x5] = false; keyPressed = true; break;
                case SDLK_e: keys[0x6] = false; keyPressed = true; break;
                case SDLK_a: keys[0x7] = false; keyPressed = true; break;
                case SDLK_s: keys[0x8] = false; keyPressed = true; break;
                case SDLK_d: keys[0x9] = false; keyPressed = true; break;
                case SDLK_z: keys[0xa] = false; keyPressed = true; break;
                case SDLK_c: keys[0xb] = false; keyPressed = true; break;
                case SDLK_4: keys[0xc] = false; keyPressed = true; break;
                case SDLK_r: keys[0xd] = false; keyPressed = true; break;
                case SDLK_f: keys[0xe] = false; keyPressed = true; break;
                case SDLK_v: keys[0xf] = false; keyPressed = true; break;
                default: break;
            }
        }

        if (shouldWaitForKeyPress && keyPressed) {
            break;
        }
    }

    return true;
}

void Platform::clearScreen() {
    SDL_SetRenderDrawColor(mRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(mRenderer);
}

void Platform::presentDisplay() {
    SDL_RenderPresent(mRenderer);
}

void Platform::drawGraphics(const uint8_t *graphics) {
    int i, j;
    SDL_Rect fillRect;
    for (i = 0; i < GRAPHICS_WIDTH; ++i) {
        for (j = 0; j < GRAPHICS_HEIGHT; ++j) {
            if (graphics[(j * GRAPHICS_WIDTH) + i]) {
                SDL_SetRenderDrawColor(mRenderer, mSetColor.r, mSetColor.g, mSetColor.b, 0xff);
            } else {
                SDL_SetRenderDrawColor(mRenderer, mUnsetColor.r, mUnsetColor.g, mUnsetColor.b, 0xff);
            }

            fillRect = {i * mScale, j * mScale, mScale, mScale};
            SDL_RenderFillRect(mRenderer, &fillRect);
        }
    }
}
