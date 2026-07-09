#include "include/chip8.h"
#include "include/run.h"

// SDL2 window structure
bool sdl_inicialize(sdl_t *sdl){
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0){
        SDL_Log("Unable to inicialize SDL: %s", SDL_GetError());
        return false;
    }

    sdl->window = SDL_CreateWindow("Chip8 Emulator", 
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, 
                                    DISPLAY_COLS*SCALE,  // Scale width for better visibility
                                    DISPLAY_ROWS*SCALE,  // Scale height for better visibility
                                    0);

    if (!sdl->window) {
        SDL_Log("SDL window creation failed: %s\n", SDL_GetError());
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl->renderer) {
        SDL_Log("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    sdl->want = (SDL_AudioSpec){
        .freq = 44100,
        .format = AUDIO_S16LSB,
        .channels = 1,
        .samples = 512,
        .callback = audio_callback,
    };

    sdl->dev = SDL_OpenAudioDevice(NULL, 0, &sdl->want, &sdl->have, 0);

    if(sdl->dev == 0){
        SDL_Log("Could not get an Audio Device %s\n", SDL_GetError());
        return false;
    }

    if((sdl->want.format != sdl->have.format) || (sdl->want.channels != sdl->have.channels)){
        SDL_Log("Could not get desired Audio Spec\n");
        return false;
    }

    return true; // Success
}

// Audio callback function
void audio_callback(void *userdata, uint8_t *stream, int len){
    (void)userdata; 

    int16_t *audio_data = (int16_t *)stream;

    static uint32_t running_sample_index = 0;
    const int32_t square_wave_period = 44100 / 440;
    const int32_t half_square_wave_period = square_wave_period / 2;

    for (int i = 0; i < len / 2; i++){
        audio_data[i] = ((running_sample_index++ / half_square_wave_period) % 2) ? VOLUME : -VOLUME;
    }
}

// Update the screen with the current display buffer
void update_screen(sdl_t sdl){
    SDL_Rect rect = {.x = 0, .y = 0, .w = SCALE-2, .h = SCALE-2};

    for(uint32_t i = 0; i < DISPLAY_SIZE; i++){
        rect.x = (i % DISPLAY_COLS) * SCALE;
        rect.y = (i / DISPLAY_COLS) * SCALE;

        if(display[i]){
            SDL_SetRenderDrawColor(sdl.renderer, 90, 0, 255, 255); // Purple for pixels that are on
            SDL_RenderFillRect(sdl.renderer, &rect);
        } else {
            SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255); // Black for pixels that are off
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
    }

    SDL_RenderPresent(sdl.renderer);
}

/*
CHIP-8 Keypad Layout  |   QWERTY Keyboard Mapping
----------------------|--------------------------
1  2  3  C            |   1  2  3  4
4  5  6  D            |   Q  W  E  R
7  8  9  E            |   A  S  D  F
A  0  B  F            |   Z  X  C  V
*/

// Handle input events
void get_handle_input(){
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT: // Exit window; End program
                state = QUIT;
                return;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        state = QUIT;
                        break;
                    case SDLK_SPACE:
                        state = (state == PAUSED) ? RUNNING : PAUSED;
                        break;

                    // CHIP-8 Key to QWERTY Mapping
                    case SDLK_1: key[0x1] = true; break;
                    case SDLK_2: key[0x2] = true; break;
                    case SDLK_3: key[0x3] = true; break;
                    case SDLK_4: key[0xC] = true; break;

                    case SDLK_q: key[0x4] = true; break;
                    case SDLK_w: key[0x5] = true; break;
                    case SDLK_e: key[0x6] = true; break;
                    case SDLK_r: key[0xD] = true; break;

                    case SDLK_a: key[0x7] = true; break;
                    case SDLK_s: key[0x8] = true; break;
                    case SDLK_d: key[0x9] = true; break;
                    case SDLK_f: key[0xE] = true; break;

                    case SDLK_z: key[0xA] = true; break;
                    case SDLK_x: key[0x0] = true; break;
                    case SDLK_c: key[0xB] = true; break;
                    case SDLK_v: key[0xF] = true; break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym){
                    // CHIP-8 Key to QWERTY Mapping
                    case SDLK_1: key[0x1] = false; break;
                    case SDLK_2: key[0x2] = false; break;
                    case SDLK_3: key[0x3] = false; break;
                    case SDLK_4: key[0xC] = false; break;

                    case SDLK_q: key[0x4] = false; break;
                    case SDLK_w: key[0x5] = false; break;
                    case SDLK_e: key[0x6] = false; break;
                    case SDLK_r: key[0xD] = false; break;

                    case SDLK_a: key[0x7] = false; break;
                    case SDLK_s: key[0x8] = false; break;
                    case SDLK_d: key[0x9] = false; break;
                    case SDLK_f: key[0xE] = false; break;

                    case SDLK_z: key[0xA] = false; break;
                    case SDLK_x: key[0x0] = false; break;
                    case SDLK_c: key[0xB] = false; break;
                    case SDLK_v: key[0xF] = false; break;
                    default: break;
                } 
                break;
            default:
                break;
        }
    }
}

// Update the delay and sound timers
void update_timers(sdl_t sdl){
    if(delay_timer > 0){delay_timer--;}

    if (sound_timer > 0) {
        sound_timer--;
        SDL_PauseAudioDevice(sdl.dev, 0); // Play sound
    } else {
        SDL_PauseAudioDevice(sdl.dev, 1); // Pause sound
    }
}

// Cleanup and free resources
void final_cleanup(sdl_t *sdl){
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_CloseAudioDevice(sdl->dev);
    SDL_Quit();
}

// Function to allow user to choose a ROM to load
void choice_rom(char **rom){
    printf("Select a ROM to load:\n");
    printf("1. Test ROM\n");
    printf("2. IBM Logo\n");
    printf("3. Tetris\n");
    printf("4. Brick\n");
    printf("Enter your choice (1-4): ");

    int choice;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            *rom = "src/roms/test.ch8";
            break;
        case 2:
            *rom = "src/roms/IBM Logo.ch8";
            break;
        case 3:
            *rom = "src/roms/tetris.ch8";
            break;
        case 4:
            *rom = "src/roms/Brick.ch8";
            break;
        default:
            printf("Invalid choice. Defaulting to Test ROM.\n");
            *rom = "src/roms/test.ch8";
            break;
    }
}

// Main function to run the emulator
int main(int argc, char *argv[]){
    (void)argc;
    (void)argv;

    char *rom = NULL;
    choice_rom(&rom);

    sdl_t sdl = {0};
    if(!sdl_inicialize(&sdl)) exit(EXIT_FAILURE);

    if (!chip8_inicialize(rom)) {
        final_cleanup(&sdl);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    const uint32_t CPU_CYCLES_PER_FRAME = 100/60; // 60Hz

    uint32_t frameStart = SDL_GetTicks();
    const int FPS = 60;
    const int CYCLES_PER_FRAME = 10;

    while (state != QUIT){
        frameStart = SDL_GetTicks();

        get_handle_input();

        if(state == PAUSED){
            SDL_Delay(100); // Sleep for 100ms when paused
            continue; // Skip the rest of the loop
        }

        for (int i = 0; i < CYCLES_PER_FRAME; i++)
            chip8_emulateCycle();

        update_timers(sdl);

        if(drawFlag)
        {
            update_screen(sdl);
            drawFlag = false;
        }

        uint32_t frameTime = SDL_GetTicks() - frameStart;

        if(frameTime < 16)
            SDL_Delay(16 - frameTime);
    }

    final_cleanup(&sdl);
    exit(EXIT_SUCCESS);

    return 0;
}