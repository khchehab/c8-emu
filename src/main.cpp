#include <iostream>
#include <stdexcept>
#include "chip8.h"
#include "platform.h"
#include "beeper.h"

int main() {
    try {
        Platform platform("Chip8 Emulator", 10);

        Beeper beeper(440, 100);

        Chip8 c8(beeper);
        c8.loadRom("roms/chip8-test-suite/6-keypad.ch8");

        while (true) {
            if (!platform.processInput(c8.getKeys())) {
                break;
            }

            platform.clearScreen();

            c8.execute();
            platform.drawGraphics(c8.getGraphics());

            platform.presentDisplay();
        }

        return 0;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
