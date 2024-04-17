#include "platform.h"
#include <iostream>
#include <stdexcept>

Platform::Platform(const std::string &title, int scale) : mScale(scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error(std::string("could not initialize SDL! SDL Error: ") + SDL_GetError());
    }

    int screenWidth = GRAPHICS_WIDTH * mScale;
    int screenHeight = GRAPHICS_HEIGHT * mScale;

    if (DEBUG) {
        screenWidth += 400;
        screenHeight += 400;
    }

    mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (mWindow == nullptr) {
        throw std::runtime_error(std::string("could not create window! SDL Error: ") + SDL_GetError());
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == nullptr) {
        throw std::runtime_error(std::string("could not create renderer! SDL Error: ") + SDL_GetError());
    }

    if (!SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "linear")) {
        std::cout << "[warning]: could not set the render scale quality to linear" << std::endl;
    }

    mTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GRAPHICS_WIDTH,
                                 GRAPHICS_HEIGHT);
    if (mTexture == nullptr) {
        throw std::runtime_error(std::string("could not create texture! SDL Error: ") + SDL_GetError());
    }

    mSurface = SDL_CreateRGBSurfaceWithFormat(0, GRAPHICS_WIDTH, GRAPHICS_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);
    if (mSurface == nullptr) {
        throw std::runtime_error(std::string("could not create rgb surface! SDL Error: ") + SDL_GetError());
    }

    mSetColor = SDL_MapRGBA(mSurface->format, 0x00, 0x00, 0x00, 0x0ff);
    mUnsetColor = SDL_MapRGBA(mSurface->format, 0xff, 0xff, 0xff, 0xff);
}

Platform::~Platform() {
    SDL_FreeSurface(mSurface);
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);

    SDL_Quit();
}

int eventFilterForWait(void *userdata, SDL_Event *e) {
    if (e->type == SDL_QUIT || e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
        return 1;
    }
    return 0;
}

int eventFilterAllowAll(void *userdata, SDL_Event *e) {
    return 1;
}

int Platform::consumeEvent(SDL_Event *e, bool shouldWaitForKeyPress) {
    if (shouldWaitForKeyPress) {
        return SDL_WaitEvent(e);
    } else {
        return SDL_PollEvent(e);
    }
}

bool Platform::processInput(uint8_t *keys, bool shouldWaitForKeyPress) {
    SDL_Event e;

    if (shouldWaitForKeyPress) {
        SDL_SetEventFilter(eventFilterForWait, nullptr);
    } else {
        SDL_SetEventFilter(eventFilterAllowAll, nullptr);
    }

    bool keyPressed = false;

    while (Platform::consumeEvent(&e, shouldWaitForKeyPress)) {
        if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)) {
            return false;
        }

        if (e.type != SDL_KEYDOWN && e.type != SDL_KEYUP) {
            break;
        }

        /*
         * The keyboard layout, which will be mapped to keys 1 to 4 horizontally and 1 to z vertically,
         * is as follows:
         * | 1 | 2 | 3 | C |    | 1 | 2 | 3 | 4 |
         * | 4 | 5 | 6 | D | -> | Q | W | E | R |
         * | 7 | 8 | 9 | E | -> | A | S | D | F |
         * | A | 0 | B | F |    | Z | X | C | V |
         */

        bool keyState = true;
        if (e.type == SDL_KEYUP) {
            keyState = false;
            if (shouldWaitForKeyPress) {
                keyPressed = true;
            }
        }

        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            case SDLK_x:
                keys[0x0] = keyState;
                break;
            case SDLK_1:
                keys[0x1] = keyState;
                break;
            case SDLK_2:
                keys[0x2] = keyState;
                break;
            case SDLK_3:
                keys[0x3] = keyState;
                break;
            case SDLK_q:
                keys[0x4] = keyState;
                break;
            case SDLK_w:
                keys[0x5] = keyState;
                break;
            case SDLK_e:
                keys[0x6] = keyState;
                break;
            case SDLK_a:
                keys[0x7] = keyState;
                break;
            case SDLK_s:
                keys[0x8] = keyState;
                break;
            case SDLK_d:
                keys[0x9] = keyState;
                break;
            case SDLK_z:
                keys[0xa] = keyState;
                break;
            case SDLK_c:
                keys[0xb] = keyState;
                break;
            case SDLK_4:
                keys[0xc] = keyState;
                break;
            case SDLK_r:
                keys[0xd] = keyState;
                break;
            case SDLK_f:
                keys[0xe] = keyState;
                break;
            case SDLK_v:
                keys[0xf] = keyState;
                break;
            default:
                break;
        }

        if (keyPressed) {
            break;
        }
    }

    return true;
}

void Platform::clearScreen() {
    SDL_SetRenderDrawColor(mRenderer, 0xe2, 0x34, 0x56, 0xff);
    SDL_RenderClear(mRenderer);
}

void Platform::presentDisplay() {
    SDL_Rect destRect = {0, 0, GRAPHICS_WIDTH * mScale, GRAPHICS_HEIGHT * mScale};

    SDL_RenderCopy(mRenderer, mTexture, nullptr, &destRect);
    SDL_RenderPresent(mRenderer);
}

void Platform::drawGraphics(const bool *graphics) {
    SDL_LockSurface(mSurface);

    int i, j, index;
    Uint32 *pixels = (Uint32 *) mSurface->pixels;
    for (i = 0; i < GRAPHICS_WIDTH; ++i) {
        for (j = 0; j < GRAPHICS_HEIGHT; ++j) {
            index = (j * GRAPHICS_WIDTH) + i;
            pixels[index] = graphics[index] ? mSetColor : mUnsetColor;
        }
    }

    SDL_UnlockSurface(mSurface);

    SDL_UpdateTexture(mTexture, nullptr, mSurface->pixels, mSurface->pitch);
}
