#include "chip8.h"
#include <fstream>
#include <stdexcept>
#include <random>

Chip8::Chip8(const Beeper& beeper) : mBeeper(beeper), I(0x0000), PC(0x0200), SP(0x00), DT(0x00), ST(0x00) {
    for (int i = 0; i < FONT_SET_SIZE; i++) {
        memory[i] = FONT_SET[i];
    }
}

void Chip8::loadRom(const std::string &path) {
    // std::ios_base::ate to seek directly to the end to get the size
    std::ifstream stream(path, std::ios_base::binary | std::ios_base::ate);

    if (!stream.is_open()) {
        throw std::runtime_error("could not open the file " + path);
    }

    std::streampos size = stream.tellg();
    std::unique_ptr<char[]> buffer(new char[size]);

    stream.seekg(std::ios_base::beg);
    stream.read(buffer.get(), size);
    stream.close();

    for (int i = 0; i < size; ++i) {
        memory[PC + i] = buffer[i];
    }
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
                memset(graphics, 0, sizeof(graphics));
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
                V[0xf] = V[x] > V[y] ? 0x1 : 0x0;
                V[x] = (V[x] - V[y]) & 0xff;
            } else if (n == 0x6) { // SHR Vx {, Vy}
                V[x] = V[y];
                V[0xf] = (V[x] & 0x01) == 0x01 ? 0x1 : 0x0;
                V[x] >>= 1;
            } else if (n == 0x7) { // SUBN Vx, Vy
                V[0xf] = V[y] > V[x] ? 0x1 : 0x0;
                V[x] = (V[y] - V[x]) & 0xff;
            } else if (n == 0xe) { // SHL Vx {, Vy}
                V[x] = V[y];
                V[0xf] = (V[x] & 0x10) == 0x10 ? 0x1 : 0x0;
                V[x] <<= 1;
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
            V[0xf] = 0x0;

            uint8_t i, j, sprite;
            uint8_t xPos, yPos;
            uint8_t spritePixel, screenPixel;
            for (i = 0; i < n; ++i) {
                sprite = memory[I + i];

                for (j = 0; j < 8; ++j) {
                    xPos = (V[x] + j) % GRAPHICS_WIDTH;
                    yPos = (V[y] + i) % GRAPHICS_HEIGHT;

                    spritePixel = (sprite & (0x80 >> j)) >> (8 - j - 1);
                    screenPixel = graphics[(yPos * GRAPHICS_WIDTH) + xPos];

                    if (spritePixel == 0x1) {
                        if (screenPixel == 0x1) {
                            V[0xf] = 0x1;
                        }

                        graphics[(yPos * GRAPHICS_WIDTH) + xPos] = screenPixel ^ spritePixel;
                    }
                }
            }
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
                waitForKeyPress(x);
            } else if (nn == 0x15) { // LD DT, Vx
                DT = V[x];
            } else if (nn == 0x18) { // LD ST, Vx
                ST = V[x];
            } else if (nn == 0x1e) { // ADD I, Vx
                I += V[x];
            } else if (nn == 0x29) { // LD F, Vx
                I = 5 * V[x];
            } else if (nn == 0x33) { // LD B, Vx
                uint8_t hundreds = V[x] / 100;
                uint8_t tens = (V[x] / 10) % 10;
                uint8_t ones = V[x] % 10;

                memory[I] = hundreds;
                memory[I + 1] = tens;
                memory[I + 2] = ones;
            } else if (nn == 0x55) { // LD [I], Vx
                for (uint8_t i = 0; i <= x; i++) {
                    memory[I++] = V[i];
                }
            } else if (nn == 0x65) { // LD Vx, [I]
                for (uint8_t i = 0; i <= x; i++) {
                    V[i] = memory[I++];
                }
            }
            break;
        default:
            throw std::runtime_error("invalid operation");
    }

    if (incrementPC) {
        PC += 2;
    }

    decrementTimers();
}

uint16_t Chip8::getCurrentOpcode() {
    return (memory[PC] << 8) | memory[PC + 1];
}

void Chip8::waitForKeyPress(uint8_t x) {
    if (keypad[0x0]) {
        V[x] = 0x0;
    } else if (keypad[0x1]) {
        V[x] = 0x1;
    } else if (keypad[0x2]) {
        V[x] = 0x2;
    } else if (keypad[0x3]) {
        V[x] = 0x3;
    } else if (keypad[0x4]) {
        V[x] = 0x4;
    } else if (keypad[0x5]) {
        V[x] = 0x5;
    } else if (keypad[0x6]) {
        V[x] = 0x6;
    } else if (keypad[0x7]) {
        V[x] = 0x7;
    } else if (keypad[0x8]) {
        V[x] = 0x8;
    } else if (keypad[0x9]) {
        V[x] = 0x9;
    } else if (keypad[0xa]) {
        V[x] = 0xa;
    } else if (keypad[0xb]) {
        V[x] = 0xb;
    } else if (keypad[0xc]) {
        V[x] = 0xc;
    } else if (keypad[0xd]) {
        V[x] = 0xd;
    } else if (keypad[0xe]) {
        V[x] = 0xe;
    } else if (keypad[0xf]) {
        V[x] = 0xf;
    } else {
        PC -= 2;
    }
}

void Chip8::decrementTimers() {
    if (DT > 0) {
        --DT;
    }

    if (ST > 0) {
        --ST;
        mBeeper.play();
    }
}

uint8_t Chip8::randomByte() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint8_t> distribution(0x00, 0xff);
    return distribution(rng);
}

const uint8_t *Chip8::getGraphics() const {
    return graphics;
}

uint8_t *Chip8::getKeys() {
    return keypad;
}