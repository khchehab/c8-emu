#ifndef C8_EMU_CHIP8_H
#define C8_EMU_CHIP8_H

#include <string>
#include <chrono>
#include "constants.h"
#include "beeper.h"

class Chip8 {
public:
    explicit Chip8(const Beeper &beeper);

    void loadRom(const std::string &path);

    void execute();

    void decrementTimers();

    const bool *getGraphics() const;

    uint8_t *getKeys();

    void setWaitedKeyPress();

    bool shouldWaitForKeyPress() const { return mShouldWaitForKeyPress; }

    void setShouldWaitForKeyPress(bool shouldWaitForKeyPress) { mShouldWaitForKeyPress = shouldWaitForKeyPress; }

private:
    Beeper mBeeper;

    uint8_t V[REGISTER_SIZE]{};
    uint8_t memory[MEMORY_SIZE]{};
    bool graphics[GRAPHICS_WIDTH * GRAPHICS_HEIGHT]{};
    uint16_t stack[STACK_SIZE]{};
    uint8_t keypad[KEY_SIZE]{}; // 0 if not pressed, non-0 if pressed

    uint16_t I;
    uint16_t PC;
    uint8_t SP;
    uint8_t DT;
    uint8_t ST;

    bool mShouldWaitForKeyPress;

    std::chrono::steady_clock::time_point mTimerPrev;

    uint16_t getCurrentOpcode();

    static uint8_t randomByte();

    void drawSprite(uint8_t x, uint8_t y, uint8_t n);
};

#endif //C8_EMU_CHIP8_H
