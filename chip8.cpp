#include "chip8.h"
#include <iostream>
#include <fstream>
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
	clear(graphics);
	clear(stack);
	clear(v);
	clear(memory);
	for(int i = 0; i < 80; i++){ // Load font into memory
		memory[i] = font[i];
	}
}

bool Chip8::load(const char * filename){
	FILE * romFile = fopen(filename, "rb");	// Open rom file
	fseek(romFile, 0, SEEK_END);
	size_t romSize = ftell(romFile);		// Get size in bytes
	rewind(romFile);
	char * buffer = new char[romSize];
	size_t result = fread(buffer, sizeof(char), romSize, romFile); // Read rom file to buffer
	for(int i = 0; i < romSize; i++){		// Copy to memory
		memory[i + 0x200] = (unsigned short) buffer[i];
	}
	fclose(romFile);
	delete [] buffer;
	return true;
}

void Chip8::cycle(){
	opcode = memory[pc] << 8 | memory[pc+1]; // Each opcode is 8 bit
	switch(opcode & 0xF000){	// Parse current opcode
		case 0x0000:

		// ANNN : set current address to NNN
		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		default:
			cout << "Error: unknown opcode " << opcode << endl;
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
}
