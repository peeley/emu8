#include "chip8.h"
#include <random>
#include <iostream>
#include <chrono>
#include <thread>
using std::cout;
using std::endl;
using std::size_t;


// Helper function to clear registers, memory, stack, etc
template<typename T>
void clear(T array){
	size_t length = sizeof(array)/sizeof(array[0]);
	for(size_t i = 0; i < length; i++ ){
		array[i] = 0;
	}
}

unsigned char font[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8(){
	pc = 0x200;		// ROM instructions start at 0x200
	opcode = 0;		// Initialize all values to 0
	I = 0;
	sp = 0;
	draw = false;
	clock = 0;
	sound = 0;
	clear(graphics);
	clear(stack);
	clear(v);
	clear(memory);
	clear(input);
	for(int i = 0; i < 80; i++){ // Load font into memory
		memory[i] = font[i];
	}
}

bool Chip8::load(const char * filename){
	FILE * romFile = fopen(filename, "rb");	// Open rom file
	fseek(romFile, 0, SEEK_END);
	size_t romSize = ftell(romFile);		// Get size in bytes
	rewind(romFile);
	char * buffer = (char *) malloc(sizeof(char) * romSize);
	size_t result = fread(buffer, sizeof(char), romSize, romFile); // Read rom file to buffer
	for(int i = 0; i < romSize; i++){		// Copy to memory
		memory[i + 0x200] = (unsigned short) buffer[i];
	}
	fclose(romFile);
	free(buffer);
	return true;
}
int idx = 0;
void Chip8::cycle(){
	opcode = memory[pc] << 8 | memory[pc+1]; // Each opcode is 8 bit
	cout << std::hex << opcode << endl;
	++idx;
	switch(opcode & 0xF000){	// Parse current opcode
		
		// 00E_ 
		case 0x0000:
			switch(opcode & 0x000F){
				// 00E0 : Clear screen
				case 0x0000:
					for(int i = 0; i < 2048; i++)
						graphics[i] = 0;
					draw = true;
					pc += 2;
					break;
				// 00EE : return
				case 0x000E:
					--sp;
					pc = stack[sp];
					pc += 2;
					break;
				default:
					cout << "Error: unknown opcode " << opcode << endl;
			}
			break;
		// 1NNN : jump to address NNN
		case 0x1000:
			pc = opcode & 0x0FFF;
			break;
		// 2NNN : calls NNN
		case 0x2000:
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;
		// 3XNN : skip next if VX == NN
		case 0x3000:
			{
			unsigned short targetRegister = (opcode & 0x0F00) >> 8;
			if(v[targetRegister] == opcode & 0x00FF)
				pc += 4;
			else
				pc += 2;
			}
			break;
		// 4XNN : skip if VX != NN
		case 0x4000:
			{
			unsigned short targetRegister = (opcode & 0x0F00) >> 8;
			if(v[targetRegister] != opcode & 0x00FF)
				pc += 4;
			else
				pc += 2;
			}
			break;
		// 5XY0 : skips next if VX = VY
		case 0x5000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short yReg = (opcode & 0x00F0) >> 4;
			if(v[xReg] == v[yReg])
				pc += 4;
			else
				pc += 2;
			}
			break;
		// 6XNN : set VX to NN
		case 0x6000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short num = (opcode & 0x00FF);
			v[xReg] = num;
			pc += 2;
			}
			break;
		// 7XNN : add NN to VX
		case 0x7000:
			{
			unsigned short targetReg = (opcode & 0x0F00) >> 8;
			unsigned short num = (opcode & 0x00FF);
			v[targetReg] += num;
			pc += 2;
			}
			break;
		// 8XY_
		case 0x8000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short yReg = (opcode & 0x00F0) >> 4;
			switch(opcode & 0x000F){
				// 8XY0 : set VX = VY
				case 0x0000:
					v[xReg] = v[yReg];
					pc += 2;
					break;
				// 8XY1 : sets VX = VX | VY
				case 0x0001:
					v[xReg] |= v[yReg];
					pc += 2;
					break;
				// 8XY2 : sets VX = VX & VY
				case 0x0002:
					v[xReg] &= v[yReg];
					pc += 2;
					break;
				// 8XY3 : sets VX = VX xor VY
				case 0x0003:
					v[xReg] ^= v[yReg];
					pc += 2;
					break;
				// 8XY4 : add VY to VX, using carry flag
				case 0x0004:
					v[xReg] += v[yReg];
					if(v[yReg] > (0xFF - v[xReg]))
						v[0xF] = 1;	// set carry register to 1
					else
						v[0xF] = 0;
					pc += 2;
					break;
				// 8XY5 : VX - VY, using carry flag for borrow
				case 0x0005:
					if(v[yReg] > v[xReg])
						v[0xF] = 1;	// borrowed
					else
						v[0xF] = 0;
					v[xReg] -= v[yReg];
					pc += 2;
					break;
				// 8XY6 : shifts VX right by one, stores least significant bit in v[f]
				case 0x0006:
					v[0xF] = v[xReg] & 0x1;
					v[xReg] >>= 1;
					pc += 2;
					break;
				// 8XY7 : sets VX = VY - VX, using borrow
				case 0x0007:
					if(v[xReg] > v[yReg])
						v[0xF] = 1;
					else
						v[0xF] = 0;
					v[xReg] = v[yReg] - v[xReg];
					pc += 2;
					break;
				// 8XYE : stores most significant bit of VX in v[f], VX << 1
				case 0x000E:
					v[0xF] = v[xReg] >> 7;
					v[xReg] <<= 1;
					pc += 2;
					break;
				default:
					cout << "Error: unknown opcode " << opcode << endl;
				}
			}
			break;
		// 9XY0 : skip next if VX != VY
		case 0x9000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short yReg = (opcode & 0x00F0) >> 4;
			if(v[xReg] != v[yReg])
				pc += 4;
			else
				pc += 2;
			}
			break;
		// ANNN : set I to NNN
		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		// BNNN : jump to address NNN + V0
		case 0xB000:
			pc = (opcode & 0x0FFF) + v[0];
			break;
		// CXNN : set VX to result of & on random number
		case 0xC000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short n = opcode & 0x00FF;
			v[xReg] = (rand() % (0xFF + 1)) & n;
			pc += 2;
			}
			break;
		// DXYN : draw sprite at (VX, VY) of size 8xN.
		// each row of 8px is read from address I,
		// VF is set to 1 if any pixels are flipped
		// when sprite is drawn, 0 otherwise
		case 0xD000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			unsigned short yReg = (opcode & 0x00F0) >> 4;
			unsigned short height = opcode & 0x000F;
			v[0xF] = 0;
			unsigned short px;
			for(int y = 0; y < height; y++){
				px = memory[I + y];
				for(int x = 0; x < 8; x++){
					if((px & (0x80 >> x) != 0)){
						int pixelLoc = v[xReg] + x + ((v[yReg] + y) * 64);
						if(graphics[pixelLoc] == 1)
							v[0xF] = 1;
						graphics[pixelLoc] ^= 1;
					}
				}
			}
			draw = true;
			pc += 2;
			}
			break;
		// EX__
		case 0xE000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			switch(opcode & 0x00FF){
				// EX9E : skips next instruction if input[VX] is pressed, != 0
				case 0x009E:
					if(input[xReg] != 0)
						pc += 4;
					else
						pc += 2;
					break;
				// EXA1 : skips if input[VX] isn't pressed, == 0
				case 0x00A1:
					if(input[xReg] == 0)
						pc += 4;
					else
						pc += 2;
					break;
				default:
					cout << "Error: unknown opcode " << opcode << endl;
			}
			}
			break;
		// FX__
		case 0xF000:
			{
			unsigned short xReg = (opcode & 0x0F00) >> 8;
			switch(opcode & 0x00FF){
				// FX07 : sets VX to clock
				case 0x0007:
					v[xReg] = clock;
					pc += 2;
					break;
				// FX0A : stores next input into vx
				case 0x000A:
					{
					bool pressed = false;
					for(int i = 0; i < 16; i++ ){
						if(input[i] != 0){
							v[xReg] = 1;
							pressed = true;
						}
					}
					if(!pressed)
						return;
					pc += 2;
					}
					break;
				// FX15 : sets clock to VX
				case 0x0015:
					clock = v[xReg];
					pc += 2;
					break;
				// FX18 : sets sound timer to VX
				case 0x0018:
					sound = v[xReg];
					pc += 2;
					break;
				// FX1E : add VX to I
				case 0x001E:
					I += v[xReg];
					pc += 2;
					break;
				// FX29 : set I to location of sprite of character in VX
				case 0x0029:
					I = v[xReg] * 0x5;
					pc += 2;
					break;
				// FX33 : stores binary coded decimal of VX with
				// three most significant bits at I, the middle
				// digit at I + 1, least significant bit at I+2.
				case 0x0033:
					memory[I] = v[xReg] / 100;
					memory[I+1] = (v[xReg] / 10) % 10;
					memory[I+2] = (v[xReg] % 100) % 10;
					pc += 2;
					break;
				// FX55 : store V0-VX in memory starting at I
				case 0x0055:
					// TODO : different interpreter version ?
					for(int n = 0; n < xReg; n++){
						memory[I+n] = v[n];
					}
					pc += 2;
					break;
				// FX65 : fill V0-VX with vals from memory starting at I
				case 0x0065:
					for(int n = 0; n < xReg; n++){
						v[n] = memory[I+n];
					}
					pc += 2;
					break;
				default:
					cout << "Error: unknown opcode " << opcode << endl;
			}
			}
			break;
		default:
			cout << "Error: unknown opcode " << opcode << endl;
			exit(1);
	}
	// deal with clock and sound - TODO
	if(clock > 0)
		--clock;
	if(sound > 0){
		if(sound == 1){
			cout << "pretend you heard a beep" << endl;
			--sound;
		}
	}
}

int main(int argc, char * argv[]){
	Chip8 chip;
	if(argc < 2){
		cout << "Please provide ROM filename." << endl;
		exit(1);
	}
	cout << "Loading ROM file: " << argv[1] << endl;
	chip.load(argv[1]);
	cout << "Finished loading ROM." << endl;
	while(true){
		chip.cycle();
		std::this_thread::sleep_for(std::chrono::microseconds(1200));		
	}
}
