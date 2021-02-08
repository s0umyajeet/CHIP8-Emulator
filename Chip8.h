/*
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        CHIP8 EMULATION - A simple Chip8 emulator written in C
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        About Chip8
        ~~~~~~~~~~~
        Chip-8 is a simple, interpreted, programming language which was first used on
        some do-it-yourself computer systems in the late 1970s and early 1980s.
        The COSMAC VIP, DREAM 6800, and ETI 660 computers are a few examples.
        These computers typically were designed to use a television as a display,
        had between 1 and 4K of RAM, and used a 16-key hexadecimal keypad for input.
        The interpreter took up only 512 bytes of memory, and programs,
        which were entered into the computer in hexadecimal, were even smaller.

        (quote) http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
        More on: https://en.wikipedia.org/wiki/CHIP-8

        Author
        ~~~~~~
        Soumyajeet Mandal, https://github.com/s0umyajeet
        Sunday, 8 February 2021

        Note
        ~~~~
        All references and diagrams taken from:
        ``` Cowgod's Chip-8 Technical Reference v1.0 ```
        http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

*/


#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>


struct Chip8
{
    //The Chip8 is capable of accessing upto 4KB of RAM
    //from location 0x000 (0) to oxFFF (4095)

    uint8_t memory[4096];

    //        Memory Map:
    //    +---------------+= 0xFFF (4095) End of Chip-8 RAM
    //    |               |
    //    |               |
    //    |               |
    //    |               |
    //    |               |
    //    | 0x200 to 0xFFF|
    //    |     Chip-8    |
    //    | Program / Data|
    //    |     Space     |
    //    |               |
    //    |               |
    //    |               |
    //    +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
    //    |               |
    //    |               |
    //    |               |
    //    +---------------+= 0x200 (512) Start of most Chip-8 programs
    //    | 0x000 to 0x1FF|
    //    | Reserved for  |
    //    |  interpreter  |
    //    +---------------+= 0x000 (0) Start of Chip-8 RAM


    //Chip8 has 16 general purpose 8-bit registers referred to as Vx
    //where x is a hexadecimal digit (0 through F)
    uint8_t V[16];

    //Chip8 has a stack capable of storing 16 16-bit values
    //Hence it allows upto 16 levels of nested subroutines
    uint16_t stack[16];

    //It points to the topmost level of the stack
    uint16_t sp;

    //16 bit program counter to store the currently
    //executing address
    uint16_t pc;

    //Used to store the opcode after each fetch cycle
    uint16_t opcode;

    //16 bit Index register generally used to store memory addresses
    //so only the lowest (rightmost) 12 bits are usually used
    uint16_t I; //Index register

    //Chip-8 provides 2 timers, a delay timer and a sound timer
    //Delay timer is active whenever this register is non zero
    uint8_t soundTimer;

    //Sound timer is active whenever this regitster is non zero
    uint8_t delayTimer;

    //Chip8 has a 64 x 32 pixel monochrome display of this format:
    //  -------------------
    //  | (0,0)	   (63,0) |
    //  |                 |
    //  | (0,31)  (63,31) |
    //  -------------------
    //
    uint8_t graphics[64 * 32];


    //The computers which originally used the Chip-8
    //had a 16-key hexadecimal keypad with the following layout:
    /*  -----------------
        | 1 | 2 | 3 | C |
        -----------------
        | 4 | 5	| 6 | D |
        -----------------
        | 7 | 8	| 9 | E |
        -----------------
        | A | 0 | B | F |
        -----------------
    */
    uint8_t keys[16];

    //flag to control drawing to the screen
    int8_t drawFlag;
};

void init();
void emulateCycle();
int8_t load(const char *file_path);

#endif // CHIP8_H
