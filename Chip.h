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
		bool load(const char *);
		void cycle();
		~Chip8()= default;
		unsigned char input[16];		// 16 button inputs
		unsigned char graphics[64*32];	// display buffer
		bool draw;
	private:
		unsigned char memory[4096];		// 4KB memory
		unsigned char v[16];			// 16 8bit registers, v0-ve and carry register
		unsigned short I;				// 16bit index register
		unsigned short stack[16];		// 16 byte stack
		unsigned short sp;				// stack pointer
		unsigned short pc;				// program counter
		unsigned short opcode;			// current 8bit opcode
		unsigned char clock;			// system timer
		unsigned char sound;			// sound timer
};
#endif
