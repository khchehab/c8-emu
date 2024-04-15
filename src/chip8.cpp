#include "chip8.h"
#include <fstream>
#include <stdexcept>
#include <random>

const long long TIMERS_TIME_PER_CYCLE = 1000 / 60;

Chip8::Chip8(const Beeper &beeper) : mBeeper(beeper), I(0x000), PC(0x0200), SP(0x00), DT(0x00), ST(0x00),
                                     mShouldWaitForKeyPress(false) {
    for (int i = 0; i < FONT_SET_SIZE; i++) {
        memory[i] = FONT_SET[i];
    }

    mTimerPrev = std::chrono::steady_clock::now();
}

void Chip8::loadRom(const std::string &path) {
    // std::ios_base::ate to seek directly to the end to get the size
    std::ifstream stream(path, std::ios_base::binary | std::ios_base::ate);

    if (!stream.is_open()) {
        throw std::runtime_error("could not open the file " + path);
    }

    std::streampos size = stream.tellg();
    if (size > MEMORY_SIZE - 0x200) {
        throw std::runtime_error("the rom will not fit in memory");
    }

    stream.seekg(std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&memory[0x200]), size);
    stream.close();
}

void Chip8::execute() {
    uint16_t opcode = getCurrentOpcode();

    uint8_t opcodeFamily = (opcode & 0xf000) >> 12;
    uint16_t nnn = opcode & 0x0fff;
    uint8_t nn = opcode & 0x00ff;
    uint8_t n = opcode & 0x000f;
    uint8_t x = (opcode & 0x0f00) >> 8;
    uint8_t y = (opcode & 0x00f0) >> 4;

    bool incrementPC = true;

    switch (opcodeFamily) {
        case 0x0:
            if (nnn == 0x0e0) { // CLS
                memset(graphics, false, sizeof(graphics));
            } else if (nnn == 0x0ee) { // RET
                PC = stack[--SP];
            }
            break;
        case 0x1: // JP addr
            PC = nnn;
            incrementPC = false;
            break;
        case 0x2: // CALL addr
            stack[SP++] = PC;
            PC = nnn;
            incrementPC = false;
            break;
        case 0x3: // SE Vx, byte
            if (V[x] == nn) {
                PC += 2;
            }
            break;
        case 0x4: // SNE Vx, byte
            if (V[x] != nn) {
                PC += 2;
            }
            break;
        case 0x5: // SE Vx, Vy
            if (V[x] == V[y]) {
                PC += 2;
            }
            break;
        case 0x6: // LD Vx, byte
            V[x] = nn;
            break;
        case 0x7: // ADD Vx, byte
            V[x] += nn;
            break;
        case 0x8:
            if (n == 0x0) { // LD Vx, Vy
                V[x] = V[y];
            } else if (n == 0x1) { // OR Vx, Vy
                V[x] |= V[y];
                V[0xf] = 0x0;
            } else if (n == 0x2) { // AND Vx, Vy
                V[x] &= V[y];
                V[0xf] = 0x0;
            } else if (n == 0x3) { // XOR Vx, Vy
                V[x] ^= V[y];
                V[0xf] = 0x0;
            } else if (n == 0x4) { // ADD Vx, Vy
                uint16_t res = V[x] + V[y];
                V[x] = res & 0xff;
                V[0xf] = res > 255 ? 0x1 : 0x0;
            } else if (n == 0x5) { // SUB Vx, Vy
                uint8_t carry = V[x] >= V[y] ? 0x1 : 0x0;
                V[x] = V[x] - V[y];
                V[0xf] = carry;
            } else if (n == 0x6) { // SHR Vx {, Vy}
                V[x] = V[y];
                uint8_t carry = (V[x] & 0x01) == 0x01 ? 0x1 : 0x0;
                V[x] >>= 1;
                V[0xf] = carry;
            } else if (n == 0x7) { // SUBN Vx, Vy
                uint8_t carry = V[y] >= V[x] ? 0x1 : 0x0;
                V[x] = V[y] - V[x];
                V[0xf] = carry;
            } else if (n == 0xe) { // SHL Vx {, Vy}
                V[x] = V[y];
                uint8_t carry = (V[x] & 0x80) == 0x80 ? 0x1 : 0x0;
                V[x] <<= 1;
                V[0xf] = carry;
            }
            break;
        case 0x9: // SNE Vx, Vy
            if (V[x] != V[y]) {
                PC += 2;
            }
            break;
        case 0xa: // LD I, addr
            I = nnn;
            break;
        case 0xb: // JP V0, addr
            PC = V[0x0] + nnn;
            incrementPC = false;
            break;
        case 0xc: // RND Vx, byte
            V[x] = randomByte() & nn;
            break;
        case 0xd: // DRW Vx, Vy, nibble
            drawSprite(x, y, n);
            break;
        case 0xe: {
            uint8_t key = V[x];
            if (nn == 0x9e) { // SKP Vx
                if (keypad[key]) {
                    PC += 2;
                }
            } else if (nn == 0xa1) { // SKNP Vx
                if (!keypad[key]) {
                    PC += 2;
                }
            }
        }
            break;
        case 0xf:
            if (nn == 0x07) { // LD Vx, DT
                V[x] = DT;
            } else if (nn == 0x0a) { // LD Vx, K
                mShouldWaitForKeyPress = true;
            } else if (nn == 0x15) { // LD DT, Vx
                DT = V[x];
            } else if (nn == 0x18) { // LD ST, Vx
                ST = V[x];
            } else if (nn == 0x1e) { // ADD I, Vx
                I = (I + V[x]) & 0x0fff;
            } else if (nn == 0x29) { // LD F, Vx
                I = (5 * V[x]) & 0x0fff;
            } else if (nn == 0x33) { // LD B, Vx
                uint8_t hundreds = V[x] / 100;
                uint8_t tens = (V[x] / 10) % 10;
                uint8_t ones = V[x] % 10;

                memory[I & 0x0fff] = hundreds;
                memory[(I + 1) & 0x0fff] = tens;
                memory[(I + 2) & 0x0fff] = ones;
            } else if (nn == 0x55) { // LD [I], Vx
                for (uint8_t i = 0; i <= x; i++) {
                    memory[I & 0x0fff] = V[i];
                    I = (I + 1) & 0x0fff;
                }
            } else if (nn == 0x65) { // LD Vx, [I]
                for (uint8_t i = 0; i <= x; i++) {
                    V[i] = memory[I & 0x0fff];
                    I = (I + 1) & 0x0fff;
                }
            }
            break;
        default:
            throw std::runtime_error("invalid operation");
    }

    if (incrementPC) {
        PC += 2;
    }
}

uint16_t Chip8::getCurrentOpcode() {
    return (memory[PC] << 8) | memory[PC + 1];
}

void Chip8::setWaitedKeyPress() {
    // when doing wait key press, PC has already been incremented
    // should get the opcode of the previous execute
    uint16_t previousOpcode = (memory[PC - 2] << 8) | memory[PC - 1];
    uint8_t x = (previousOpcode & 0x0f00) >> 8;

    for (uint8_t i = 0; i < KEY_SIZE; ++i) {
        if (keypad[i]) {
            V[x] = i;
            break;
        }
    }
}

void Chip8::decrementTimers() {
    if (DT == 0 && ST == 0) {
        return;
    }

    auto timerCurr = std::chrono::steady_clock::now();
    auto timerElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timerCurr - mTimerPrev).count();

    if (timerElapsed >= TIMERS_TIME_PER_CYCLE) {
        mTimerPrev = timerCurr;

        if (DT > 0) {
            --DT;
        }

        if (ST > 0) {
            --ST;
            mBeeper.play();
        }
    }
}

uint8_t Chip8::randomByte() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint8_t> distribution(0x00, 0xff);
    return distribution(rng);
}

void Chip8::drawSprite(uint8_t x, uint8_t y, uint8_t n) {
    uint8_t xPos = V[x] % GRAPHICS_WIDTH;
    uint8_t yPos = V[y] % GRAPHICS_HEIGHT;

    V[0xf] = 0x0;

    uint8_t i, j, sprite, spritePixel, mask, bits;
    uint16_t index;
    bool screenPixel;
    for (i = 0; i < n; ++i) {
        if (yPos + i > GRAPHICS_HEIGHT) {
            continue;
        }

        sprite = memory[(I + i) & 0x0fff];

        for (j = 0; j < 8; ++j) {
            if (xPos + j > GRAPHICS_WIDTH) {
                continue;
            }

            index = ((yPos + i) * GRAPHICS_WIDTH) + (xPos + j);

            spritePixel = (sprite & (0x01 << (8 - j - 1))) >> (8 - j - 1);
            screenPixel = graphics[index];

            if (spritePixel == 0x1) {
                if (screenPixel) {
                    V[0xf] = 0x1;
                }

                graphics[index] = screenPixel ^ spritePixel;
            }
        }
    }
}

const bool *Chip8::getGraphics() const {
    return graphics;
}

uint8_t *Chip8::getKeys() {
    return keypad;
}
