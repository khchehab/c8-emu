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

    void drawGraphics(const bool *graphics);

private:
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;
    SDL_Texture *mTexture;
    SDL_Surface *mSurface;

    Uint32 mSetColor;
    Uint32 mUnsetColor;

    int mScale;

    static int consumeEvent(SDL_Event *e, bool shouldWaitForKeyPress);
};

#endif //C8_EMU_PLATFORM_H
