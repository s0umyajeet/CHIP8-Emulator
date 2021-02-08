#include "Chip8.h"
#include <stdio.h>
#include "stdint.h"
#include "SDL2/SDL.h"

//for sleep
#ifdef _WIN32

#include <Windows.h>
#define sleep(a) Sleep(a)

#else
#include <unistd.h>
#endif

struct Chip8 chip;

// Keypad keymap for SDL
uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ROM file path missing! Please see usage below:\n");
        printf("Usage: ./chip8 [ROM file]\n");
        return 1;
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    //width and height for the SDL window
    int w = 1024;
    int h = 512;

    SDL_Window *window = NULL;

    //Initialize the window
    window = SDL_CreateWindow("Chip8 Emulator",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w, h, SDL_WINDOW_SHOWN);

    //Quit if window creation failed
    if (window == NULL) {
        return 0;
    }

    //Initialize renderer and texture
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    //Quit if renderer creation failed
    if (renderer == NULL) {
        return 0;
    }

    SDL_RenderSetLogicalSize(renderer, w, h);

    //Inialize the texture
    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, 64, 32);

    //Quit if texture creation failed
    if (tex == NULL) {
        return 0;
    }

    //Chip8 graphics -- 64 x 32 pixels
    uint32_t pixels[2048];

reload:

    // Quit if  loading the ROM failed
    if (!load(argv[1])) {
        return 2;
	}

    //Main loop
	while (1) {
        emulateCycle();
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                exit(0);

            //Process key down events
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                if (event.key.keysym.sym == SDLK_F1)
                    goto reload;

                for (int i = 0; i < 16; i++)
                    if (event.key.keysym.sym == keymap[i])
                        chip.keys[i] = 1;
            }

            // Process keyup events
            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i)
                    if (event.key.keysym.sym == keymap[i])
                        chip.keys[i] = 0;
            }
        }

        // if drawFlag set to true
        if (chip.drawFlag) {
            chip.drawFlag = 0;

            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; i++) {
                uint8_t pixel = chip.graphics[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, tex, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // sleep to slow down emulation speed
        usleep(1200);
	}
    return 0;
}
