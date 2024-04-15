#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "chip8.h"
#include "platform.h"
#include "beeper.h"

const long long CLOCK_TIME = 1000 / 300;

int main() {
    try {
        Platform platform("Chip8 Emulator", 10);

        Beeper beeper(440, 100);

        Chip8 c8(beeper);
        c8.loadRom("roms/chip8-test-suite/1-chip8-logo.ch8");

        std::chrono::steady_clock::time_point clockPrev = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point clockCurr;
        long long clockDelta;

        while (true) {
            if (!platform.processInput(c8.getKeys(), c8.shouldWaitForKeyPress())) {
                break;
            }

            if (c8.shouldWaitForKeyPress()) {
                c8.setShouldWaitForKeyPress(false);
                c8.setWaitedKeyPress();
            }

            platform.clearScreen();

            c8.execute();
            c8.decrementTimers();

            platform.drawGraphics(c8.getGraphics());

            platform.presentDisplay();

            clockCurr = std::chrono::steady_clock::now();
            clockDelta = std::chrono::duration_cast<std::chrono::milliseconds>(clockCurr - clockPrev).count();

            if (clockDelta < CLOCK_TIME) {
                std::this_thread::sleep_for(std::chrono::milliseconds(CLOCK_TIME - clockDelta));
            }

            clockPrev = clockCurr;
        }

        return 0;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
