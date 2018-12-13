/* chip8.h
 * Noah Snelson
 * November 21, 2018
 * Emulator for CHIP-8 architecture and assembly language.
 */

#include <iostream>
#ifndef CHIP8_HPP_
#define CHIP8_HPP_

class Chip8{
	public:
		Chip8();
		~Chip8()= default;
		bool load(const char *);        // Load ROM into memory
		void cycle();                   // CPU cycle
		bool input[16];         		// 16 button inputs
		unsigned char graphics[64*32];	// display buffer
		bool draw;

	private:
		unsigned char memory[4096];		// 4KB memory
		unsigned char v[16];			// 16 8bit registers, v0-ve and carry register
		unsigned short I;				// 16bit index register
		unsigned short stack[16];		// 32 byte stack
		unsigned short sp;				// stack pointer
		unsigned short pc;				// program counter
		unsigned short opcode;			// current opcode
		unsigned char clock;			// system timer
		unsigned char sound;			// sound timer
};
#endif
