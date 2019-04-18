#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int X_SCALE = 12;
const int Y_SCALE = 12;
const int FRAME_DELAY = 10;

//Emulation Constants
const int screenSize = 64 * 32;


const string game = "PONG";

class chip8
{
public:
	void loadRom();
	void clearMemory(unsigned char(&memory)[4096], const int &maxMem);
	void clearRegisters(unsigned char(&V)[16], const int &regSize);
	void loadFonts(unsigned char(&memory)[4096], unsigned char(&chip8_fontset)[80]);
	void clearStack(unsigned short(&stack)[16], const int &stackSize);
	void initialize();
	void emulateCycle();
	void sdlClose();
	void setupGraphics();
	void drawGraphics();
	bool sdlInit();
	bool loadFlags();
	SDL_Texture* loadTexture(string path);
	void setKeys();
	void clearScreen(unsigned char(&gfx)[screenSize], const int &screenSize);
	

	bool quit = false;
	bool drawFlag;
	int frameRate;
	SDL_Renderer* gameRender = NULL;
	SDL_Event e;

private:
	unsigned short opcode; //used to store current opcode
	unsigned char memory[4096]; //4k of memory in this bad boy
	unsigned char V[16]; //the V registers
	unsigned short index; // index register
	unsigned short pc; //program counter
	unsigned char gfx[screenSize]; //graphics 
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short stack[16]; //cpu's stack
	unsigned short sp; //stack point
	unsigned char key[16]; //Hex based keypad

	// file streaming stuff
	ifstream file;
	streampos size;
	char* memblock;

	SDL_Window* gameWindow;

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
};