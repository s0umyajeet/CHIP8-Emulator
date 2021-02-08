#include "Chip8.h"
#include <stdio.h>
#include <stdlib.h>

//---------------------defines-------------------

#define VX (chip.V[(chip.opcode & 0x0F00) >> 8])
#define VY (chip.V[(chip.opcode & 0x00F0) >> 4])
#define X ((chip.opcode & 0x0F00) >> 8)
#define Y ((chip.opcode & 0x00F0) >> 4)
#define NNN (chip.opcode & 0x0FFF)
#define NN (chip.opcode & 0x00FF)

//-----------------------------------------------

extern struct Chip8 chip;



/*
    Chip8 programs may refer to a group of sprites representing the hexadecimal digits 0 through F
    These sprites are 5 bytes long, or 8x5 pixels
    An example of how each character's hexadecimal values are calculated:

         -------------------------
        | "0"	  Binary	Hex  |
         -------------------------
        | ****   11110000   0xF0 |
        | *  *   10010000   0x90 |
        | *  *   10010000   0x90 |
        | *  *   10010000   0x90 |
        | ****   11110000   0xF0 |
        --------------------------
*/

//Notice the hex column above matches the first five values (for 0):
unsigned char chip8_fontset[80] =
{
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

void init()
{
    //0x200 (512) is where most Chip8 programs start
    chip.pc = 0x200;

    //clear all other registers
    chip.sp = 0;
    chip.opcode = 0;
    chip.I = 0;

    //clear the display
    for (int i = 0; i < 2048; i++) {
        chip.graphics[i] = 0;
    }

    //clear stack keys and V
    for (int i = 0; i < 16; i++) {
        chip.stack[i] = 0;
        chip.V[i] = 0;
        chip.keys[i] = 0;
    }

    //clear the memory
    for (int i = 0; i < 4096; i++) {
        chip.memory[i] = 0;
    }

    //load the fontset into memory
    //It should be stored in the interpreter area of Chip-8 memory (0x000 to 0x1FF)
    //(first 512 bytes)
    for (int i = 0; i < 80; i++) {
        chip.memory[i] = chip8_fontset[i];
    }

    chip.soundTimer = 0;
    chip.delayTimer = 0;

    // Seed random number generator function
    srand(time(NULL));
}

int8_t load(const char *file_path) {
    init();

    FILE *rom = fopen(file_path, "rb");
    if (rom == NULL) {
        return 0;
    }

    //get the ROM size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    //allocate memory for the buffer
    char *rom_buffer = (char *)malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL) {
        return 0;
    }

    //copy ROM into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);

    if (result != rom_size) {
        return 0;
    }

    //Copy buffer into the memory
    //0x000 to 0x1FF reserved for the interpreter hence - 512
    if ((4096 - 512) > rom_size) {
        for (int i = 0; i < rom_size; i++)
            chip.memory[i + 512] = (uint8_t)rom_buffer[i];
    } else {
        return 0;
    }

    fclose(rom);
    free(rom_buffer);

    return 1;
}

void emulateCycle()
{
    //Fetch instruction opcode
    //Chip8 opcode is of two bytes
    //shifting first 8 bytes and ORing with the next 8 bytes
    //to create the complete 16 bit opcode
    chip.opcode = chip.memory[chip.pc] << 8 | chip.memory[chip.pc + 1];

    //decode and execute opcode
    switch (chip.opcode & 0xF000) {
        case 0x0000:
            switch (chip.opcode & 0x00FF) {
                case 0x00E0:
                    //clear screen (00E0)
                    for (int i = 0; i < 2048; i++)
                        chip.graphics[i] = 0;

                    chip.drawFlag = 1;
                    chip.pc += 2;
                break;
                case 0x00EE:
                    //return from a subroutine (00EE)
                    chip.sp--;
                    chip.pc = chip.stack[chip.sp];
                    chip.pc += 2;
                    break;

                default:
                    exit(3);
            }
            break;
        case 0x1000:
            //Jump to address NNN
            chip.pc = NNN;
            break;
        case 0x2000:
            //call subroutine at NNN
            chip.stack[chip.sp] = chip.pc;
            chip.sp++;
            chip.pc = NNN;
            break;
        case 0x3000:
            //Skip next instruction if VX is equal to NN
            if (VX == NN) chip.pc += 4;
            else chip.pc += 2;
            break;
        case 0x4000:
            if (VX != NN) chip.pc += 4;
            else chip.pc += 2;
            break;
        case 0x5000:
            if (VX == VY) chip.pc += 4;
            else chip.pc += 2;
            break;
        case 0x6000:
            VX = NN;
            chip.pc += 2;
            break;
        case 0x7000:
            VX += NN;
            chip.pc += 2;
            break;
        case 0x8000:
            switch (chip.opcode & 0x000F)
            {
                case 0x0000:
                    VX = VY;
                    chip.pc += 2;
                    break;
                case 0x0001:
                    VX |= VY;
                    chip.pc += 2;
                    break;
                case 0x0002:
                    VX &= VY;
                    chip.pc += 2;
                    break;
                case 0x0003:
                    VX ^= VY;
                    chip.pc += 2;
                    break;
                case 0x0004:
                    VX += VY;
                    if (VY + VY > (0xFF - VX)) chip.V[0xF] = 1;
                    else chip.V[0xF] = 0;
                    chip.pc += 2;
                    break;
                case 0x0005:
                    //this means there is a borrow
                    if (VY > VX) chip.V[0xF] = 0;
                    else chip.V[0xF] = 1;
                    VX -= VY;
                    chip.pc += 2;
                    break;
                case 0x0006:
                    // 8XY6 - Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    chip.V[(0xF)] = VX & 0x1;
                    VX >>= 1;
                    chip.pc += 2;
                    break;
                case 0x0007:
                    //this means there is a borrow
                    if (VX > VY) chip.V[0xF] = 0;
                    else chip.V[0xF] = 1;
                    VX = VY - VX;
                    chip.pc += 2;
                    break;
                case 0x000E:
                    // 8XYE - Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    chip.V[0xF] = VX >> 7;
                    VX <<= 1;
                    chip.pc += 2;
                    break;
                default:

                    exit(3);
                }
                break;
        case 0x9000:
            //Skips the next instruction if VX doesn't equal VY.
            //(Usually the next instruction is a jump to skip a code block)
            if (VX != VY) chip.pc += 4;
            else chip.pc += 2;
            break;
        case 0xA000:
            //Sets I to the address NNN.
            chip.I = NNN;
            chip.pc += 2;
            break;
        case 0xB000:
            //Jumps to the address NNN plus V0.
            chip.pc = NNN + chip.V[0x0];
            break;
        case 0xC000:
            //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            VX = (rand() % (0xFF + 1)) & NN;
            chip.pc += 2;
            break;
        case 0xD000:
        {
            unsigned short x = VX;
            unsigned short y = VY;
            unsigned short height = chip.opcode & 0x000F;
            unsigned short pixel;

            chip.V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = chip.memory[chip.I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (chip.graphics[(x + xline + ((y + yline) * 64))] == 1) {
                            chip.V[0xF] = 1;
                        }
                        chip.graphics[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            chip.drawFlag = 1;
            chip.pc += 2;
        }
        break;

        case 0xE000:
            switch (chip.opcode & 0x00FF)
            {
                case 0x009E:
                    if (chip.keys[VX] != 0) chip.pc += 4;
                    else chip.pc += 2;
                    break;
                case 0x00A1:
                    if (chip.keys[VX] == 0) chip.pc += 4;
                    else chip.pc += 2;
                    break;
                default:
                    break;
            }
            break;
        case 0xF000:
            switch (chip.opcode & 0x00FF)
            {
                case 0x0007:
                    // FX07 Sets VX to the value of the delay timer.
                    VX = chip.delayTimer;
                    chip.pc += 2;
                    break;
                case 0x000A:
                {
                    // FX0A A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                    uint8_t key_pressed = 0;

                    for (int i = 0; i < 16; ++i) {
                        if (chip.keys[i] != 0) {
                            VX = i;
                            key_pressed = 1;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if (!key_pressed)
                        return;

                    chip.pc += 2;
                }
                break;
                case 0x0015:
                    //FX15 Sets the delay timer to VX.
                    chip.delayTimer = VX;
                    chip.pc += 2;
                    break;
                case 0x0018:
                    //FX18 Sets the sound timer to VX.
                    chip.soundTimer = VX;
                    chip.pc += 2;
                    break;
                case 0x001E:
                    //FX1E Adds VX to I. VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to 0 when there isn't
                    chip.V[0xF] = (chip.I + VX) > 0xFFF;
                    chip.I += VX;
                    chip.pc += 2;
                    break;
                case 0x0029:
                    //FX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    chip.I = VX * 0x5;
                    chip.pc += 2;
                    break;
                case 0x0033:
                    //FX33 Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
                    //the middle digit at I plus 1, and the least significant digit at I plus 2.
                    //(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I,
                    //the tens digit at location I+1, and the ones digit at location I+2.)
                    chip.memory[chip.I] = VX / 100;
                    chip.memory[chip.I + 1] = (VX / 10) % 10;
                    chip.memory[chip.I + 2] = VX % 10;
                    chip.pc += 2;
                    break;
                case 0x0055:
                    //FX55 Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
                    for (int i = 0; i <= X; i++) {
                        chip.memory[chip.I + i] = chip.V[i];
                    }

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    //I += ((chip.opcode & 0x0F00) >> 8) + 1;
                    chip.pc += 2;
                    break;
                case 0x0065:
                    // FX65 Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                    for (int i = 0; i <= X; i++) {
                        chip.V[i] = chip.memory[chip.I + i];
                    }

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    //I += ((chip.opcode & 0x0F00) >> 8) + 1;
                    chip.pc += 2;
                    break;

                default:
                    break;
                }
                break;

        default:
            exit(3);
    }

    // Update timers
    if (chip.delayTimer > 0)
        chip.delayTimer--;

    if (chip.soundTimer > 0)
        chip.soundTimer--;
}
