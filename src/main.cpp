#include <iostream>
#include <stdexcept>
#include "chip8.h"
#include "platform.h"

int main() {
    try {
        Chip8 c8;
        c8.loadRom("roms/chip8-test-rom/test_opcode.ch8");

        Platform platform("Chip8 Emulator", 10);

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
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
