# CHIP-8 Emulator

A CHIP-8 emulator written in **C** using **SDL2** for graphics, audio, and keyboard input.

The project aims to provide a simple and readable implementation of the CHIP-8 virtual machine, making it suitable for learning emulator development and computer architecture concepts.

---

# About CHIP-8

CHIP-8 is an interpreted programming language created by Joseph Weisbecker in the mid-1970s for the COSMAC VIP and other 8-bit computers.

Instead of running native machine code, CHIP-8 programs (commonly called **ROMs**) execute on a virtual machine with:

- 35 standard opcodes
- 4 KB of memory
- 16 general-purpose 8-bit registers
- 16-bit index register
- 16-level stack
- 64×32 monochrome display
- 16-key hexadecimal keypad
- Delay and sound timers running at 60 Hz

Because of its small instruction set and straightforward architecture, CHIP-8 is one of the most popular starting points for emulator development.

---

# Features

- Complete implementation of the 35 standard CHIP-8 instructions
- 64×32 monochrome display
- SDL2-based rendering
- Configurable display scaling (default: **15×**, resulting in a **960×480** window)
- 16-key hexadecimal keypad mapped to a QWERTY keyboard
- Delay and sound timer emulation
- Square wave audio (440 Hz)
- Pause/Resume support
- ROM loading from disk
- CPU running at approximately **600 instructions per second**
  - 60 FPS
  - 10 CPU cycles per frame

---

# Project Structure

```
chip-8-emulator/
├── README.md
└── src/
    ├── chip8.c          # CPU implementation, opcode execution, ROM loading
    ├── run.c            # SDL2 initialization, rendering, input, timers, main loop
    ├── include/
    │   ├── chip8.h      # CHIP-8 definitions and CPU declarations
    │   └── run.h        # SDL2 structures and renderer declarations
    └── roms/
        ├── Brick.ch8
        ├── IBM Logo.ch8
        ├── test.ch8
        └── tetris.ch8
```

---

# Dependencies

- GCC or Clang
- SDL2 Development Libraries

## Installing SDL2

### Ubuntu / Debian

```bash
sudo apt install libsdl2-dev
```

### Arch Linux

```bash
sudo pacman -S sdl2
```

### macOS

```bash
brew install sdl2
```

### Windows (MSYS2)

```bash
pacman -S mingw-w64-x86_64-SDL2
```

---

# Building

Compile the emulator using:

```bash
gcc src/*.c -o src/output/chip8.exe -lmingw32 -lSDL2main -lSDL2
```

---

# Running

.\src\output\chip8.exe

```c
char *rom = "src/roms/Brick.ch8";
```

Build the project and execute:

```bash
./chip8
```

---

# Controls

## CHIP-8 Keypad Mapping

| CHIP-8 | Keyboard |
|:------:|:--------:|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

## Emulator Controls

| Key | Action |
|------|--------|
| **ESC** | Exit emulator |
| **SPACE** | Pause / Resume execution |

---

# Emulator Architecture

## Memory Layout

| Address | Purpose |
|----------|---------|
| `0x000–0x1FF` | Interpreter area and font sprites |
| `0x200–0xFFF` | Program ROM and working memory |

Programs are loaded starting at **0x200**, following the original CHIP-8 specification.

---

## CPU State

| Component | Description |
|------------|-------------|
| `memory[4096]` | 4 KB RAM |
| `V[16]` | General-purpose registers (V0–VF) |
| `I` | Index register |
| `pc` | Program counter |
| `stack[16]` | Call stack |
| `sp` | Stack pointer |
| `delay_timer` | 60 Hz countdown timer |
| `sound_timer` | 60 Hz sound timer |
| `key[16]` | Keyboard state |
| `display[64][32]` | Framebuffer |

---

# Instruction Set

The emulator implements all standard CHIP-8 instructions.

| Opcode Group | Functionality |
|---------------|--------------|
| `00E0` | Clear display |
| `00EE` | Return from subroutine |
| `1nnn` | Jump |
| `2nnn` | Call subroutine |
| `3xkk–5xy0` | Conditional skips |
| `6xkk–8xyE` | Register operations |
| `9xy0` | Skip if registers differ |
| `Annn` | Load index register |
| `Bnnn` | Jump with offset |
| `Cxkk` | Random number |
| `Dxyn` | Draw sprite |
| `Ex9E`, `ExA1` | Keyboard input |
| `Fx07–Fx65` | Timers, memory operations, BCD conversion, key wait |

Sprites are rendered using XOR drawing, and collision detection is reported through register **VF**, exactly as defined by the CHIP-8 specification.

---

# Graphics

Rendering is performed using SDL2.

Each CHIP-8 pixel is scaled to a square of **15×15** pixels (minus a small border for visual separation), producing a final resolution of **960×480**.

| State | Color |
|--------|-------|
| ON | Purple (`RGB 90, 0, 255`) |
| OFF | Black (`RGB 0, 0, 0`) |

---

# Audio

The emulator generates a **440 Hz square wave** while the `sound_timer` is non-zero, matching the original CHIP-8 buzzer behavior.

---

# Example ROMs

The repository includes a few ROMs for testing:

- IBM Logo
- Brick
- Tetris
- Test ROM

These can be selected by changing the ROM path in `run.c`.

