#include "SDL3/SDL.h"
#include "Chip8.h"
#include "cstdlib"

Chip8 chip8;

int main(int argc, char** argv) {
	char* rom = argv[1];
	// Initialize graphics
	SDL_Window* window = SDL_CreateWindow("Elric's Chip-8 Emulator", 640, 320, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if (!initSDL()) exit(EXIT_FAILURE);

	// Initial screen clear
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // background color
	SDL_RenderClear(renderer);

	// Setup input callbacks

	// Initualize chip8
	Chip8* chip8 = chip8Init();
	// Load ROM
	chip8LoadProgram(chip8, rom);

	// Emulation loop
	while (chip8->running) {
		// Emulate one cycle
		chip8EmulateFrame(chip8, renderer);		
	}

	// Cleanup
	chip8Destroy(chip8);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

