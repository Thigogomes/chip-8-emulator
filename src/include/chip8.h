#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define true 1
#define false 0
#define bool int

#define MEM_SIZE 4096
#define V_SIZE 16
#define DISPLAY_ROWS 32
#define DISPLAY_COLS 64
#define DISPLAY_SIZE (DISPLAY_ROWS * DISPLAY_COLS)
#define STACK_SIZE 16
#define KEY_SIZE 16
#define ENTRY_POINT 0x200

// Emulator states
typedef enum{
    QUIT,
    RUNNING,
    PAUSED,
} emulator_state_t;

extern uint16_t opcode;
extern uint8_t memory[MEM_SIZE];
extern uint8_t V[V_SIZE];
extern uint16_t I;
extern uint16_t pc;
extern uint8_t display[DISPLAY_SIZE];
extern uint8_t delay_timer;
extern uint8_t sound_timer;
extern uint16_t stack[STACK_SIZE];
extern uint16_t sp;
extern uint8_t key[KEY_SIZE];
extern emulator_state_t state;
extern bool drawFlag;

bool chip8_inicialize(char *rom);
void chip8_emulateCycle();
void draw_sprite(uint8_t x, uint8_t y, uint8_t n);
bool load_ROM(char *rom);
void unknown_opcode(uint16_t opcode);

#endif