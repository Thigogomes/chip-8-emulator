#include "include/chip8.h"
#include "include/run.h"

uint16_t opcode;
uint8_t memory[MEM_SIZE];
uint8_t V[V_SIZE];
uint16_t I;
uint16_t pc;
uint8_t display[DISPLAY_SIZE];
uint8_t delay_timer;
uint8_t sound_timer;
uint16_t stack[STACK_SIZE];
uint16_t sp;
uint8_t key[KEY_SIZE];
bool drawFlag;
emulator_state_t state;

const uint8_t chip8_fontset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Initialize the Chip8 machine state
bool chip8_inicialize(char *rom){
    pc = ENTRY_POINT;
    opcode = 0;
    I = 0;
    sp = 0;

    memset(memory, 0, sizeof(uint8_t)*MEM_SIZE);
    memset(V, 0, sizeof(uint8_t)*V_SIZE);
    memset(display, 0, sizeof(uint8_t)*DISPLAY_SIZE);
    memset(stack, 0, sizeof(uint16_t)*STACK_SIZE);
    memset(key, 0, sizeof(uint8_t)*KEY_SIZE);

    memcpy(memory, chip8_fontset, sizeof(chip8_fontset));

    if(!load_ROM(rom)){
        return false;
    }

    delay_timer = 0;
    sound_timer = 0;
    state = RUNNING;
    drawFlag = true;

    return true;
}

// Draw a sprite at position (x, y) with n bytes of sprite data starting at memory location I
void draw_sprite(uint8_t x, uint8_t y, uint8_t n){
    unsigned row = y, col = x;

    // set the collision flag to 0
    V[0xF] = 0;
    for (unsigned byte_index = 0; byte_index < n; byte_index++) {

        uint8_t byte = memory[I + byte_index];

        for (unsigned bit_index = 0; bit_index < 8; bit_index++) {

            uint8_t bit = (byte >> (7 - bit_index)) & 1;

            uint8_t *pixel =
                &display[((y + byte_index) % DISPLAY_ROWS) * DISPLAY_COLS +
                        ((x + bit_index) % DISPLAY_COLS)];

            if (bit && *pixel)
                V[0xF] = 1;

            *pixel ^= bit;
        }
    }

    drawFlag = true;
}

// Load a ROM file into memory starting at address 0x200
bool load_ROM(char *rom){

    // Load ROM into RAM
    FILE *rom_file;
    rom_file = fopen(rom, "rb");

    if(rom_file == NULL){
        fprintf(stderr, "Unable to open ROM: %s\n", rom);
        return false;
    }

    // get ROM size
    fseek(rom_file, 0, SEEK_END);
    const size_t rom_size = ftell(rom_file);
    const size_t max_rom_size = sizeof(memory) - ENTRY_POINT;
    rewind(rom_file);

    if(rom_size > max_rom_size){
        fprintf(stderr, "ROM size exceeds available memory: %zu bytes\n", rom_size);
        fclose(rom_file);
        return false;
    }

    fread(&memory[ENTRY_POINT], 1, rom_size, rom_file);

    fclose(rom_file);
    return true;
}

// Handle unknown opcodes by printing an error message and exiting the program
void unknown_opcode(uint16_t opcode){
    printf("Unknown opcode %04X at PC=%03X\n", opcode, pc);
    exit(EXIT_FAILURE);
}

// Emulate one cycle of the Chip8 CPU
void chip8_emulateCycle(){
    int i;
    uint8_t x, y, n;
    uint8_t kk;
    uint16_t nnn;

    opcode = memory[pc] << 8 | memory[pc+1];

    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    n = opcode & 0x000F;
    kk = opcode & 0x00FF;
    nnn = opcode & 0x0FFF;

    switch(opcode & 0xF000){
        case 0x0000:
            switch(kk){
                case 0x00E0: // CLS (Clear the display)
                    printf("Clear the screen\n");
                    memset(display, 0, sizeof(uint8_t)*DISPLAY_SIZE);
                    drawFlag = true;
                    pc += 2;
                    break;
                case 0x00EE: // RET (Return from a subroutine)
                    printf("Return from subroutine to 0x%x\n", stack[sp - 1]);
                    pc = stack[--sp];
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        case 0x1000: // 1nnn: Jump to location nnn (JP addr)
            printf("Jump to 0x%x\n", nnn);
            pc = nnn;
            break;
        case 0x2000: // 2nnn: Call subroutine at nnn (CALL addr)
            printf("Call subroutine at 0x%x (return to 0x%x)\n", nnn, pc + 2);
            stack[sp++] = pc+2; 
            pc = nnn;
            break;
        case 0x3000: // 3xkk: Skip next instruction if Vx = kk (SE Vx, byte)
            printf("Skip if V[%x] (0x%x) == 0x%x\n", x, V[x], kk);
            pc += (V[x] == kk) ? 4 : 2;
            break;
        case 0x4000: // 4xkk: Skip next instruction if Vx != kk (SNE Vx, byte)
            printf("Skip if V[%x] (0x%x) != 0x%x\n", x, V[x], kk);
            pc += (V[x] != kk) ? 4 : 2;
            break;
        case 0x5000: // 5xy0: Skip next instruction if Vx = Vy (SE Vx, Vy)
            
            switch(n){
                case 0x0:
                    printf("Skip if V[%x] (0x%x) == V[%x] (0x%x)\n", x, V[x], y, V[y]);
                    pc += (V[x] == V[y]) ? 4 : 2;
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        case 0x6000: // 6xkk: Set Vx = kk (LD Vx, byte)
            printf("Set V[%x] = 0x%x\n", x, kk);
            V[x] = kk;
            pc +=2;
            break;
        case 0x7000: // 7xkk: Set Vx = Vx + kk (ADD Vx, byte)
            printf("Add 0x%x to V[%x] (old: 0x%x)\n", kk, x, V[x]);
            V[x] = V[x] + kk;
            pc += 2;
            break;
        case 0x8000:
           switch(n){
                case 0x0: // 8xy0: Set Vx = Vy (LD Vx, Vy)
                    printf("Set V[%x] = V[%x] (0x%x)\n", x, y, V[y]);
                    V[x] = V[y];
                    pc += 2;
                    break;
                case 0x1: // 8xy1 Set Vx = Vx OR Vy (OR Vx, Vy)
                    printf("Set V[%x] = V[%x] OR V[%x] (0x%x | 0x%x)\n", x, x, y, V[x], V[y]);
                    V[x] = V[x] | V[y];
                    pc += 2;
                    break;
                case 0x2: // 8xy2: Set Vx = Vx AND Vy (AND Vx, Vy)
                    printf("Set V[%x] = V[%x] AND V[%x] (0x%x & 0x%x)\n", x, x, y, V[x], V[y]);
                    V[x] = V[x] & V[y];
                    pc += 2;
                    break;
                case 0x3: // 8xy3: Set Vx = Vx XOR Vy (XOR Vx, Vy)
                    printf("Set V[%x] = V[%x] XOR V[%x] (0x%x ^ 0x%x)\n", x, x, y, V[x], V[y]);
                    V[x] = V[x] ^ V[y];
                    pc += 2;
                    break;
                case 0x4: // 8xy4: Set Vx = Vx + Vy, set VF = carry (ADD Vx, Vy)
                    printf("Add V[%x] (0x%x) to V[%x] (0x%x)\n", y, V[y], x, V[x]);
                    V[0xF] = ((int) V[x] + (int) V[y]) > 255 ? 1 : 0;
                    V[x] = V[x] + V[y];
                    pc += 2;
                    break;
                case 0x5: // 8xy5: Set Vx = Vx - Vy, set VF = NOT borrow (SUB Vx, 
                    printf("Set V[%x] = V[%x] - V[%x] (0x%x - 0x%x)\n", x, x, y, V[x], V[y]);
                    V[0xF] = (V[x] >= V[y]) ? 1 : 0;
                    V[x] = V[x] - V[y];
                    pc += 2;
                    break;
                case 0x6: // 8xy6: Set Vx = Vx SHR 1 (SHR Vx {, Vy})
                    printf("Shift right V[%x] (0x%x) by 1\n", x, V[x]);
                    V[0xF] = V[x] & 0x1;
                    V[x] = (V[x] >> 1);
                    pc += 2;
                    break;
                case 0x7: // 8xy7: Set Vx = Vy - Vx, set VF = NOT borrow (SUBN Vx, Vy)
                    printf("Set V[%x] = V[%x] - V[%x] (0x%x - 0x%x)\n", x, y, x, V[y], V[x]);
                    V[0xF] = (V[y] >= V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    pc += 2;
                    break;
                case 0xE: // 8xyE: Set Vx = Vx SHL 1 (SHL Vx {, Vy})
                    printf("Shift left V[%x] (0x%x) by 1\n", x, V[x]);
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] = (V[x] << 1);
                    pc += 2;
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
           }
            break;
        case 0x9000: // 9xy0: Skip next instruction if Vx != Vy (SNE Vx, Vy)
            switch(n){
                case 0x0:
                    printf("Skip if V[%x] (0x%x) != V[%x] (0x%x)\n", x, V[x], y, V[y]);
                    pc += (V[x] != V[y]) ? 4 : 2;
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        case 0xA000: // Annn: Set I = nnn (LD I, addr)
            printf("Set I = 0x%x\n", nnn);
            I = nnn;
            pc += 2;
            break;
        case 0xB000: // Bnnn: Jump to location nnn + V0 (JP V0, addr)
            printf("Jump to 0x%x + V[0] (0x%x)\n", nnn, V[0]);
            pc = nnn + V[0];
            break;
        case 0xC000: // Cxkk: Set Vx = random byte AND kk (RND Vx, byte)
            printf("Set V[%x] = random byte AND 0x%x\n", x, kk);
            V[x] = (rand() % 256) & kk;
            pc += 2;
            break;
        case 0xD000: // Dxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), 
                    // set VF = collision (DRW Vx, Vy, nibble)
            printf("Draw sprite at (V[%x]=0x%x, V[%x]=0x%x), height 0x%x, I=0x%x\n", x, V[x], y, V[y], n, I);
            draw_sprite(V[x], V[y], n);
            pc += 2;
            break;
        case 0xE000:
            switch(kk){
                case 0x9E: // Ex9E: Skip next instruction if key with the value of Vx is pressed (SKP Vx)
                    printf("Skip if key V[%x] (0x%x) is pressed\n", x, V[x]);
                    pc += (key[V[x]] ? 4 : 2);
                    break;
                case 0xA1: // ExA1: Skip next instruction if key with the value of Vx is not pressed (SKNP Vx)
                    printf("Skip if key V[%x] (0x%x) is not pressed\n", x, V[x]);
                    pc += (!key[V[x]] ? 4 : 2);
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        case 0xF000:
            switch(kk){
                case 0x07: // Fx07: Set Vx = delay timer value (LD Vx, DT)
                    printf("Set V[%x] = delay timer (0x%x)\n", x, delay_timer);
                    V[x] = delay_timer;
                    pc += 2;
                    break;
                case 0x0A: // Fx0A: Wait for a key press, store the value of the key in Vx (LD Vx, K)
                    printf("Wait for key press and store in V[%x]\n", x);
                    bool key_pressed = false;

                    for(i = 0; i < KEY_SIZE; i++){
                        if(key[i]){
                            V[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if(key_pressed){pc += 2;}
                    break;
                case 0x15: // Fx15: Set delay timer = Vx (LD DT, Vx)
                    printf("Set delay timer = V[%x] (0x%x)\n", x, V[x]);
                    delay_timer = V[x];
                    pc += 2;
                    break;
                case 0x18: // Fx18: Set sound timer = Vx (LD ST, Vx)
                    printf("Set sound timer = V[%x] (0x%x)\n", x, V[x]);
                    sound_timer = V[x];
                    pc += 2;
                    break;
                case 0x1E: // Fx1E: Set I = I + Vx (ADD I, Vx)
                    printf("Set I = I (0x%x) + V[%x] (0x%x)\n", I, x, V[x]);
                    V[0xF] = (I + V[x] > 0xfff) ? 1 : 0;
                    I = I + V[x];
                    pc += 2;
                    break;
                case 0x29: // Fx29: Set I = location of sprite for digit Vx (LD F, Vx)
                    printf("Set I to sprite address for digit in V[%x] (0x%x)\n", x, V[x]);
                    I = 5 * V[x];
                    pc += 2;
                    break;
                case 0x33: // Fx33: Store BCD representation of Vx in memory locations I, I+1, and I+2 (LD B, Vx)
                    printf("Store BCD of V[%x] (0x%x) at memory[I..I+2], I=0x%x\n", x, V[x], I);
                    memory[I] = V[x] / 100;  // Hundreds digit
                    memory[I+1] = (V[x] / 10) % 10; // Tens digit
                    memory[I+2] = V[x] % 10; // Ones digit
                    pc += 2;
                    break;
                case 0x55: // Fx55: Store registers V0 through Vx in memory starting at location I (LD [I], Vx)
                    printf("Store V[0] through V[%x] in memory starting at I=0x%x\n", x, I);
                    for(i = 0; i <= x; i++){
                        memory[I+i] = V[i];
                    }
                    I += x + 1;
                    pc += 2;
                    break;
                case 0x65: // Fx65: Read registers V0 through Vx from memory starting at location I (LD Vx, [I])
                    printf("Load V[0] through V[%x] from memory starting at I=0x%x\n", x, I);
                    for(i = 0; i <= x; i++){
                        V[i] = memory[I+i];
                    }
                    I += x + 1;
                    pc += 2;
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;

        default:
            unknown_opcode(opcode);
            break;
    }

}