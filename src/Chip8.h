#ifndef Chip8_H_
#define Chip8_H_
#include "SDL3/SDL.h"


struct Chip8 {
	// Flag to determine if CHIP-8 is running
	unsigned char running;

	// CHIP-8 has 4k memory
	unsigned char memory[4096];

	// CHIP-8 has 15 8-bit registers, V0-VE, 16th register is 'carry flag'
	unsigned char V[16];

	// Index register I
	unsigned short I;

	// Program counter PC, 0x000 to 0xFFF
	unsigned short pc;

	// Memory map
	// 0x000-0x1FF - Chip 8 interpreter
	// 0x050-0x0A0 - Used for built in 4x5 pixel font set (0-F)
	// 0x200-0xFFF - Program ROM and work RAM

	// Graphics - screen is 64x32 pixels, array stores pixel state as 1 or 0
	unsigned char gfx[64 * 32];

	// Delay and sound timer, count down at 60 Hz until they reach 0
	unsigned char delay_timer;
	unsigned char sound_timer;

	// Stack to keep track of current position before jump, plus stack point SP
	unsigned short stack[16];
	unsigned short sp;

	// Hex based keypad (0x0-0xF), stores state of each key
	unsigned char key[16];

	// Flag determines if screen should be updated
	unsigned char draw;



};

// Chip8 implementation functions
Chip8* chip8Init(void);
void chip8Destroy(Chip8*);
void chip8ExecuteOpcode(Chip8* chip8, short opcode);
void chip8UpdateTimers(Chip8* chip8);

// void chip8ClearKeys(Chip8* chip8);
// void chip8SetKeyDown(Chip8* chip8, CHIP8_KEY key);
// void chip8SetKeyUp(Chip8* chip8, CHIP8_KEY key);

void chip8LoadProgram(Chip8* chip8, const char* program);

void chip8EmulateFrame(Chip8* chip8, SDL_Renderer* renderer);

bool initSDL();

void handleInput(Chip8* chip8);

void drawScreen(Chip8* chip8, SDL_Renderer* renderer);

unsigned short fetchOpcode(Chip8* chip8);

#endif;
