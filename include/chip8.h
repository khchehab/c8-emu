#ifndef C8_EMU_CHIP8_H
#define C8_EMU_CHIP8_H

#include <string>
#include "constants.h"

class Chip8 {
public:
    Chip8();

    void loadRom(const std::string &path);

    void execute();

    const uint8_t *getGraphics() const;

    uint8_t *getKeys();

private:
    uint8_t V[REGISTER_SIZE]{};
    uint8_t memory[MEMORY_SIZE]{};
    uint8_t graphics[GRAPHICS_WIDTH * GRAPHICS_HEIGHT]{};
    uint16_t stack[STACK_SIZE]{};
    uint8_t keypad[KEY_SIZE]{}; // 0 if not pressed, non-0 if pressed

    uint16_t I;
    uint16_t PC;
    uint8_t SP;
    uint8_t DT;
    uint8_t ST;

    uint16_t getCurrentOpcode();

    void waitForKeyPress(uint8_t x);

    void decrementTimers();

    static uint8_t randomByte();
};

#endif //C8_EMU_CHIP8_H
