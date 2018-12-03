// main.cpp
// Noah Snelson
// November 28, 2018
// Main file for emulator and interpreter of CHIP-8 architecture and asm. 
// Requires SDL2 installation in /usr/include/SDL2 .

#include "Chip.h"
#include <chrono>
#include <thread>
#include "SDL2/SDL.h"
using std::cout;
using std::endl;

int main(int argc, char * argv[]){
	Chip8 chip;
	if(argc < 2){
		cout << "Please provide ROM filename." << endl;
		exit(1);
	}
	int windowWidth =  1024;
	int windowHeight = 512;
	cout << "Loading ROM file: " << argv[1] << endl;
	chip.load(argv[1]);
	cout << "Finished loading ROM." << endl;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window * window = SDL_CreateWindow("emu8", 400, 200, windowWidth, windowHeight, 0);
	SDL_Renderer * render = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(render, windowWidth, windowHeight);
	SDL_Texture * texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
											SDL_TEXTUREACCESS_STREAMING , 64, 32);

	unsigned int graphicsBuffer[2048];
	unsigned char pixel;
	unsigned char keybindings[16] = { // Emulates numpad setup, but fits laptop keyboard:
		SDLK_x,	SDLK_1,	SDLK_2,			// 1 2 3 4
		SDLK_3,	SDLK_q,	SDLK_w,			// Q W E R
		SDLK_e,	SDLK_a,	SDLK_s,			// A S D F
		SDLK_d,	SDLK_z,	SDLK_c,			// Z X C V
		SDLK_4,	SDLK_r,	SDLK_f,
		SDLK_v
	};
	while(true){
		chip.cycle();
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				SDL_DestroyWindow(window);
				SDL_Quit();
				exit(0);
			}
			if(event.type == SDL_KEYDOWN){
				if (event.key.keysym.sym == SDLK_ESCAPE){
					SDL_DestroyWindow(window);
					SDL_Quit();
					exit(0);
				}
				for(int i = 0; i < 16; i++){
					if(event.key.keysym.sym == keybindings[i]){
						chip.input[i] = 1;
					}
				}
			}
			if(event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == keybindings[i]) {
                        chip.input[i] = 0;
                    }
                }
			}
		}
		if(chip.draw){
			chip.draw = false;
			for(int i = 0; i < 2048; i++){
				pixel = chip.graphics[i];
				graphicsBuffer[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			}
			SDL_UpdateTexture(texture, NULL, graphicsBuffer, 64 * sizeof(unsigned int));
			SDL_RenderClear(render); 
			SDL_RenderCopy(render, texture, NULL, NULL);
			SDL_RenderPresent(render);
		}
		// Slows down emulation to playable level
		std::this_thread::sleep_for(std::chrono::microseconds(1200));		
	}
}
