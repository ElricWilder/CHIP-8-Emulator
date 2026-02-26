#define _CRT_SECURE_NO_WARNINGS
#include "Chip8.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <iostream>

const short MILLISECS_PER_FRAME = 16;
const short INSTRUCTIONS_PER_FRAME = 11;

unsigned char chip8_fontset[80] =
{
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

Chip8* chip8Init(void) {
	// We need to manually allocate memory here
	Chip8* chip8 = static_cast<Chip8*>(malloc(sizeof(Chip8)));

	memset(chip8, 0, sizeof(Chip8));

	// Fontset needs to be loaded to the first portion of memory
	for (int i = 0; i < 80; i++) {
		chip8->memory[i] = chip8_fontset[i];
	}
	chip8->running = 1;// chip8 is running
	chip8->pc = 0x200; // PC starts at 0x200
	chip8->I = 0;	   // Reset index register
	chip8->sp = 0;     // Reset stack pointer
	chip8->draw = 0;   // Reset draw flag
	
	// Used for opcode CXKK
	srand(time(NULL));

	return chip8;
}

void chip8Destroy(Chip8* chip8) {
	free(chip8);
}

void chip8ExecuteOpcode(Chip8* chip8, short opcode) {
	// 35 opcodes

	// Decode opcodes using bitwise operations
	unsigned char X = (opcode & 0x0F00) >> 8;
	unsigned char Y = (opcode & 0x00F0) >> 4;
	unsigned short N = opcode & 0x000F;
	unsigned short KK = opcode & 0x00FF;
	unsigned short NNN = opcode & 0x0FFF;


	// Act on most significant 4 bits of opcode
	switch ((opcode >> 12) & 0x0F) {

	case 0x0: {

		if ((opcode & 0x000F) == 0x0000) {
			// 00E0, clear screen
			std::cout << "opcode 00E0" << std::endl;


			// Set value of all pixels in gfx array to zero
			memset(chip8->gfx, 0, 64 * 32);

			// Could also clear screen via SDL 

		}
		else if ((opcode & 0x00FF) == 0x00EE) {
			// 00EE, return from a subroutine
			std::cout << "opcode 00EE" << std::endl;


			chip8->pc = chip8->stack[--chip8->sp]; // Stack pointer used to jump back to previous place in pc
		}
		else if ((opcode & 0x0F00) == 0x0000) {
			// 0NNN, Execute machine language subroutine at address NNN

			// Used for old machines, not required to implement
		}
		else {
			fprintf(stderr, "Unknown opcode '0x%x'\n", opcode);
			exit(1);
		}
		break;
	}

	case 0x1: {
		// 1NNN, jump to location nnn
		std::cout << "opcode 1NNN" << std::endl;

		chip8->pc = NNN;
		break;
	}

	case 0x2: {
		// 2NNN, call subroutine at nnn
		std::cout << "opcode 2NNN" << std::endl;

		// Put current PC on top of stack
		chip8->stack[chip8->sp] = chip8->pc;

		// Increment stack pointer
		chip8->sp += 1;

		// PC set to NNN
		chip8->pc = NNN;
		break;
	}

	case 0x3: {
		// 3xKK, skip next instruction if Vx == KK
		std::cout << "opcode 3XKK" << std::endl;

		if (chip8->V[X] == KK) {
			chip8->pc += 2;
		}
		break;
	}

	case 0x4: {
		// 4xKK, skip next instruction if Vx != KK
		std::cout << "opcode 4XKK" << std::endl;

		if (chip8->V[X] != KK) {
			chip8->pc += 2;
		}
		break;
	}

	case 0x5: {
		// 5XY0, skip next instruction if Vx == Vy
		std::cout << "opcode 5XY0" << std::endl;

		if (chip8->V[X] == chip8->V[Y]) {
			chip8->pc += 2;
		}
		break;
	}

	case 0x6: {
		// 6XKK, set Vx = KK
		std::cout << "opcode 6XKK" << std::endl;

		chip8->V[X] = KK;
		break;
	}

	case 0x7: {
		// 7XKK, set Vx += KK
		std::cout << "opcode 7XKK" << std::endl;

		chip8->V[X] += KK;
		break;
	}

	case 0x8: {
		if ((opcode & 0x000F) == 0x0) {
			// 8XY0, set VX equal to VY
			std::cout << "opcode 8XY0" << std::endl;

			chip8->V[X] = chip8->V[Y];
		}

		else if ((opcode & 0x000F) == 0x1) {
			// 8XY1, set VX equal to VX bitwise OR VY
			std::cout << "opcode 8XY1" << std::endl;

			chip8->V[X] = (chip8->V[X] | chip8->V[Y]);
		}

		else if ((opcode & 0x000F) == 0x2) {
			// 8XY2, set VX to VX bitwise AND VY
			std::cout << "opcode 8XY2" << std::endl;

			chip8->V[X] = (chip8->V[X] & chip8->V[Y]);
		}

		else if ((opcode & 0x000F) == 0x3) {
			// 8XY3, set VX to VX XOR VY
			std::cout << "opcode 8XY3" << std::endl;

			chip8->V[X] = (chip8->V[X] ^ chip8->V[Y]);
		}

		else if ((opcode & 0x000F) == 0x4) {
			// 8XY4, add VY to VX, set the VF register carry flag if needed
			std::cout << "opcode 8XY4" << std::endl;

			unsigned short result = chip8->V[X] + chip8->V[Y];
			if (result > 255) {
				chip8->V[0xF] = 1;
			}
			else {
				chip8->V[0xF] = 0;
			}
			chip8->V[X] = (result & 0x00FF);
		}

		else if ((opcode & 0x000F) == 0x5) {
			// 8XY5, set VX to VX-VY, set VF flag
			std::cout << "opcode 8XY5" << std::endl;

			if (chip8->V[Y] > chip8->V[X]) {
				chip8->V[0xF] = 0;
			}
			else {
				chip8->V[0xF] = 1;
			}
			chip8->V[X] = chip8->V[X] - chip8->V[Y];
		}

		else if ((opcode & 0x000F) == 0x6) {
			// 8XY6, set VX = VX SHR 1
			std::cout << "opcode 8XY6" << std::endl;

			// Sets VF to least significant bit
			chip8->V[0xF] = chip8->V[X] & 0x1;

			chip8->V[X] >>= 1;
		}

		else if ((opcode & 0x000F) == 0x7) {
			// 8XY7, set VX = VY - VX, set VF flag
			std::cout << "opcode 8XY7" << std::endl;

			if (chip8->V[X] > chip8->V[Y]) {
				chip8->V[0xF] = 0;
			}
			else {
				chip8->V[0xF] = 1;
			}
			chip8->V[X] = chip8->V[Y] - chip8->V[X];
		}

		else if ((opcode & 0x000F) == 0xE) {
			// 8XYE, set VX = VX SHL 1
			std::cout << "opcode 8XYE" << std::endl;

			// Sets VF to most significant bit
			chip8->V[0xF] = (chip8->V[X] & 0x80) >> 7;

			chip8->V[X] <<= 1;
		}

		else {
			fprintf(stderr, "Unknown opcode '0x%x'\n", opcode);
			exit(1);
		}

		break;
	}

	case 0x9: {
		// 9XY0, skip next instruction if VX != VY
		std::cout << "opcode 9XY0" << std::endl;

		if (chip8->V[X] != chip8->V[Y]) {
			chip8->pc += 2;
		}

		break;
	}

	case 0xA: {
		// ANNN, set I = NNN
		std::cout << "opcode ANNN" << std::endl;

		chip8->I = NNN;
		break;
	}

	case 0xB: {
		// BNNN, program counter set to NNN + value of V0
		std::cout << "opcode BNNN" << std::endl;

		chip8->pc = (NNN + chip8->V[0]);
		break;
	}

	case 0xC: {
		// CXKK, generate random num 0-255, AND it with KK, store in VX
		std::cout << "opcode CXKK" << std::endl;

		unsigned char random_num = rand() % 256;
		chip8->V[X] = random_num & KK;
		break;
	}

	case 0xD: {
		// DXYN, display n-byte sprite starting at mem location I at (VX, VY),
		// set VF = collision
		std::cout << "opcode DXYN" << std::endl;

		unsigned char xCoor = chip8->V[X] % 64; // Get X coordinate
		unsigned char yCoor = chip8->V[Y] % 32; // Get Y coordinate
		unsigned char pixel;                    // Variable to store sprite data
		unsigned char orig_X = xCoor;
		chip8->V[0xF] = 0; // Set collision flag to 0

		for (int i = 0; i < N; i++) {           // For N rows
			pixel = chip8->memory[chip8->I + i];
			xCoor = orig_X;                     // Reset X for next row to draw 
			for (int j = 7; j >= 0; j--) {      // For each bit (pixel) in row
				// If pixel in sprite on and pixel on screen on, set flag
				if ((pixel & (1 << j)) && (chip8->gfx[yCoor * 64 + xCoor])) {
					chip8->V[0xF] = 1;
				}

				// XOR display pixel with sprite pixel/bit
				chip8->gfx[yCoor * 64 + xCoor] ^= (pixel & (1 << j));

				// Stop drawing if hit right edge of screen
				if (++xCoor >= 64) {
					break;
				}
			}
			if (++yCoor >= 32) {
				break;
			}
			
		}
		chip8->draw = 1;
		break;
	}

	case 0xE: {
		if ((opcode & 0x00FF) == 0x9E) {
			// EX9E, skip next instruction if key with value of VX is pressed
			std::cout << "opcode EX9E" << std::endl;

			if (chip8->key[chip8->V[X]] == 1) {
				chip8->pc += 2;
			}
		}

		else if ((opcode & 0x00FF) == 0xA1) {
			// EXA1, skip next instruction if key with value of VX is not pressed
			std::cout << "opcode EXA1" << std::endl;

			if (chip8->key[chip8->V[X]] == 0) {
				chip8->pc += 2;
			}
		}

		else {
			fprintf(stderr, "Unknown opcode '0x%x'\n", opcode);
			exit(1);
		}

		break;
	}

	case 0xF: {
		if ((opcode & 0x00FF) == 0x07) {
			// FX07, set VX = delay timer value
			std::cout << "opcode FX07" << std::endl;

			chip8->V[X] = chip8->delay_timer;
		}

		else if ((opcode & 0x00FF) == 0x0A) {
			// FX0A, wait for key press, store value of key in VX
			std::cout << "opcode FX0A" << std::endl;
			bool key_pressed_flag = false;
			for (int i = 0; i < sizeof chip8->key; i++) {
				if (chip8->key[i] == 1) {
					chip8->V[X] = i;
					key_pressed_flag = true;
					break;
				}
			}

			// Wait for key press by decrementing program counter
			if (!key_pressed_flag) {
				chip8->pc -= 2;
			}

		}

		else if ((opcode & 0x00FF) == 0x15) {
			// FX15, set delay timer = VX
			std::cout << "opcode FX15" << std::endl;

			chip8->delay_timer = chip8->V[X];
		}

		else if ((opcode & 0x00FF) == 0x18) {
			// FX18, set sound timer = VX
			std::cout << "opcode FX18" << std::endl;

			chip8->sound_timer = chip8->V[X];
		}

		else if ((opcode & 0x00FF) == 0x1E) {
			// FX1E, set I = I + VX
			std::cout << "opcode FX1E" << std::endl;

			chip8->I += chip8->V[X];
		}

		else if ((opcode & 0x00FF) == 0x29) {
			// FX29, set I = location of sprite for digit VX
			std::cout << "opcode FX29" << std::endl;

			chip8->I = chip8->V[X] * 5;
		}

		else if ((opcode & 0x00FF) == 0x33) {
			// FX33, store BCD representation of VX in memory at I, I + 1, and I + 2
			std::cout << "opcode FX33" << std::endl;

			chip8->memory[chip8->I] = chip8->V[X] / 100;       // Hundreds digit
			chip8->memory[chip8->I + 1] = (chip8->V[X] / 10) % 10; // Tens digit
			chip8->memory[chip8->I + 2] = chip8->V[X] % 10;        // Ones digit
		}

		else if ((opcode & 0x00FF) == 0x55) {
			// FX55, store registers V0 - VX in memory starting at location I
			std::cout << "opcode FX55" << std::endl;

			for (int i = 0; i <= X; i++) {
				chip8->memory[chip8->I + i] = chip8->V[i];
			}
		}

		else if ((opcode & 0x00FF) == 0x65) {
			// FX65, read registers V0 - VX from memory starting at location I
			std::cout << "opcode FX65" << std::endl;

			for (int i = 0; i <= X; i++) {
				chip8->V[i] = chip8->memory[chip8->I + i];
			}
		}

		else {
			fprintf(stderr, "Unknown opcode '0x%x'\n", opcode);
			exit(1);
		}
		break;
	}

	}
	
}

void chip8UpdateTimers(Chip8* chip8) {
	if (chip8->delay_timer > 0) {
		chip8->delay_timer -= 1;
	}

	if (chip8->sound_timer > 0) {
		chip8->sound_timer -= 1;
	}
}

void chip8LoadProgram(Chip8* chip8, const char* program) {
	// Load program bytes from program file
	FILE* f = fopen(program, "rb");
	if (f == NULL) {
		fprintf(stderr, "File error\n");
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);

	// Create buffer
	char* buffer = (char*)malloc(sizeof(char) * fsize);
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		exit(1);
	}

	// Copy file into buffer
	size_t result = fread(buffer, 1, fsize, f);
	if (result != fsize) {
		fprintf(stderr, "Could not read program bytes\n");
		exit(1);
	}

	// Load program into memory starting at address 0x200 (512 bytes)
	for (int i = 0; i < result; ++i) {
		chip8->memory[i + 512] = buffer[i];
	}

	// Close file, free buffer
	fclose(f);
	free(buffer);
}

void chip8EmulateFrame(Chip8* chip8, SDL_Renderer* renderer) {
	handleInput(chip8);

	// Execute a certain number of instructions per frame(700/sec)
	uint32_t startFrameTime = SDL_GetTicks();
	for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
		unsigned short opcode = fetchOpcode(chip8);
		chip8ExecuteOpcode(chip8, opcode);
	}
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - startFrameTime);
	if (timeToWait > 0) { SDL_Delay(timeToWait); }

	// Update screen every 60Hz
	if (chip8->draw == 1) {
		drawScreen(chip8, renderer);
		chip8->draw = 0;
	}

	// Update timers every 60Hz
	chip8UpdateTimers(chip8);
}

bool initSDL(void) {
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 1) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	return true;

}

// Chip-8 keypad	QWERTY
// 123C  			1234
// 456D				QWER
// 789E				ASDF
// A0BF				ZXCV
void handleInput(Chip8* chip8) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		switch (event.type) {

		case SDL_EVENT_QUIT:
			chip8->running = 0;
			return;

		case SDL_EVENT_KEY_DOWN:
			switch (event.key.key) {
			case SDLK_ESCAPE:
				// Escape key exits program
				chip8->running = 0;
				return;

			case SDLK_1: chip8->key[0x1] = 1; break;
			case SDLK_2: chip8->key[0x2] = 1; break;
			case SDLK_3: chip8->key[0x3] = 1; break;
			case SDLK_4: chip8->key[0xC] = 1; break;

			case SDLK_Q: chip8->key[0x4] = 1; break;
			case SDLK_W: chip8->key[0x5] = 1; break;
			case SDLK_E: chip8->key[0x6] = 1; break;
			case SDLK_R: chip8->key[0xD] = 1; break;

			case SDLK_A: chip8->key[0x7] = 1; break;
			case SDLK_S: chip8->key[0x8] = 1; break;
			case SDLK_D: chip8->key[0x9] = 1; break;
			case SDLK_F: chip8->key[0xE] = 1; break;

			case SDLK_Z: chip8->key[0xA] = 1; break;
			case SDLK_X: chip8->key[0x0] = 1; break;
			case SDLK_C: chip8->key[0xB] = 1; break;
			case SDLK_V: chip8->key[0xF] = 1; break;

			default:
				break;
			}
			break;

		case SDL_EVENT_KEY_UP:
			switch (event.key.key) {

			case SDLK_1: chip8->key[0x1] = 0; break;
			case SDLK_2: chip8->key[0x2] = 0; break;
			case SDLK_3: chip8->key[0x3] = 0; break;
			case SDLK_4: chip8->key[0xC] = 0; break;

			case SDLK_Q: chip8->key[0x4] = 0; break;
			case SDLK_W: chip8->key[0x5] = 0; break;
			case SDLK_E: chip8->key[0x6] = 0; break;
			case SDLK_R: chip8->key[0xD] = 0; break;

			case SDLK_A: chip8->key[0x7] = 0; break;
			case SDLK_S: chip8->key[0x8] = 0; break;
			case SDLK_D: chip8->key[0x9] = 0; break;
			case SDLK_F: chip8->key[0xE] = 0; break;

			case SDLK_Z: chip8->key[0xA] = 0; break;
			case SDLK_X: chip8->key[0x0] = 0; break;
			case SDLK_C: chip8->key[0xB] = 0; break;
			case SDLK_V: chip8->key[0xF] = 0; break;

			default:
				break;
			}
			break;
		default:
			break;
		}
	}
};

void drawScreen(Chip8* chip8, SDL_Renderer* renderer) {
	SDL_FRect rect = { 0.0f, 0.0f, 10.0f, 10.0f };

	// Loop through display pixels and draw a rectangle per pixel
	for (int i = 0; i < sizeof(chip8->gfx); i++) {
		// Translate ID to 2D
		// X = i % window width
		// Y = i / window width
		rect.x = (i % 64) * 10;
		rect.y = (i / 64) * 10;

		if (chip8->gfx[i]) {
			// If pixel is on, draw foreground color
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // foreground color
			SDL_RenderFillRect(renderer, &rect);
		}
		else {
			// If pixel is off, draw background color
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // foreground color
			SDL_RenderFillRect(renderer, &rect);
		}
	}

	SDL_RenderPresent(renderer);
}

unsigned short fetchOpcode(Chip8* chip8) {
	// Retrieve and merge both bytes of opcode
	unsigned short opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
	printf("Memory Address: 0x%04X, Opcode: 0x%04X",
		chip8->pc - 2, opcode);
	// Increment program counter
	chip8->pc += 2;

	return opcode;
}