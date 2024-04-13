#ifndef C8_EMU_PLATFORM_H
#define C8_EMU_PLATFORM_H

#include <string>
#include <SDL.h>
#include "constants.h"

class Platform {
public:
    explicit Platform(const std::string &title, int scale);

    ~Platform();

    bool processInput(uint8_t *keys, bool shouldWaitForKeyPress);

    void clearScreen();

    void presentDisplay();

    void drawGraphics(const uint8_t *graphics);

private:
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;

    SDL_Color mSetColor{};
    SDL_Color mUnsetColor{};

    int mScale;
};

#endif //C8_EMU_PLATFORM_H
