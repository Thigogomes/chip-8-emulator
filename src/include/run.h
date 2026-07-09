#ifndef __RUN_H__
#define __RUN_H__

#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

// SDL2 window structure
typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
} sdl_t;

#define VOLUME 1000
#define SCALE 15

bool sdl_inicialize(sdl_t *sdl);
void final_cleanup(sdl_t *sdl);
void update_screen(sdl_t sdl);
void get_handle_input();
void update_timers(sdl_t sdl);
void audio_callback(void *userdata, uint8_t *stream, int len);
void choice_rom(char **rom);

#endif