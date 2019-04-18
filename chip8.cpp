#include "chip8.h"

using namespace std;

void chip8::loadRom()
{
	file.open(game, ios::in | ios::binary | ios::ate);
	if(file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, ios::beg);
		file.read(memblock, size);
		file.close();
		cout << "ROM load successful!" << endl;
	}
	else
	{
		cout << "ROM load failed!" << endl;
	}

	for (int i = 0; i < size; ++i)
	{
		memory[i + 512] = memblock[i];
	}
}

//Clears System Memory
void chip8::clearMemory(unsigned char(&memory)[4096], const int &maxMem)
{
	for (int i = 0; i < maxMem; ++i)
		memory[i] = 0;
}

//Loads Fontset
void chip8::loadFonts(unsigned char(&memory)[4096], unsigned char(&chip8_fontset)[80])
{
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];
}

//Clear Stack
void chip8::clearStack(unsigned short(&stack)[16], const int &stackSize)
{
	for (int i = 0; i < stackSize; ++i)
	{
		stack[i] = 0;
	}
}

//Clear Registers
void chip8::clearRegisters(unsigned char(&V)[16], const int &regSize)
{
	for (int i = 0; i < regSize; ++i)
	{
		V[i] = 0;
	}
}




void chip8::clearScreen(unsigned char(&gfx)[screenSize], const int &screenSize)
{
	for (int i = 0; i < screenSize; ++i)
	{
		gfx[i] = 0;
	}
}



void chip8::initialize()
{
	pc = 0x200; // pc starts at 0x200
	opcode = 0; // reset opcode
	index = 0; //reset index register
	sp = 0; // reset stack pointer
  
	// Clear display	
	clearScreen(gfx, screenSize);
	SDL_SetRenderDrawColor(gameRender, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(gameRender);
	SDL_RenderPresent(gameRender);
	//Set Drawing Color to White
	SDL_SetRenderDrawColor(gameRender, 0xFF, 0xFF, 0xFF, 0xFF);
	// Clear stack
	clearStack(stack, 16);
	// Clear registers V0-VF
	clearRegisters(V, 16);
	// Clear memory
	clearMemory(memory, 4096);

	// load fontset
	for(int i = 0; i < 80; ++i)
	{
	memory[i] = chip8_fontset[i];
	}

	loadRom();

    
	// Reset timers
	delay_timer = 0;
	sound_timer = 0;

	//Clear Screen Once
	drawFlag = true;

	//Seed Random
	srand(time_t(NULL));




}
 
void chip8::emulateCycle()
{
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
 
	// Decode opcode
	switch(opcode & 0xF000)
	{    
	case 0x0000:
		switch(opcode & 0x000F)
		{
			case 0x0000: // 0x00E0: Clears the screen        
				clearScreen(gfx, screenSize);
				drawFlag = true;
				pc += 2;
				break;

			case 0x000E: // 0x00EE: Returns from subroutine          
				--sp;
				pc = stack[sp];
				pc += 2;
				break;

			// 00FB through 00FF are SuperChip only
			
			default:
				printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);  
				pc += 2;
				break;
		}
	break;
	
	case 0x1000: // 1NNN: jumps to address NNN
		pc = opcode & 0x0FFF;
		break;
	
	case 0x2000: // 2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	
	case 0x3000: // 3XKK: Skip next instruction if Vx = kk.
		if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
		{
			pc += 2;
		}
		pc += 2;
		break;
	
	case 0x4000: // 4XKK: Skip next instruction if Vx != kk.
		if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
		{
			pc += 2;
		}
		pc += 2;
		break;
	
	case 0x5000: // 5XY0: Skip next instruction if Vx = Vy.
		if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
		{
			pc += 2;
		}
		break;
	
	case 0x6000: // 6XKK: set Vx = kk
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;
	
	case 0x7000: // 7XKK: set Vx = Vx + kk
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;
	
	case 0x8000:
		switch(opcode & 0x000F)
		{
			case 0x0000: // 0x8XY0: set Vx = Vy      
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x0001: // 0x8XY1: Set Vx = Vx OR Vy          
				V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;
			
			case 0x0002: // 0x8XY2: Set Vx = Vx AND Vy          
				V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;
			
			case 0x0003: // 0x8XY3: set Vx = Vx XOR Vy      
				V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
				pc += 2;
				break;

			case 0x0004:       
				if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
				pc += 2;          
				break;
			
			case 0x0005: // 0x8XY5: Set Vx = Vx - Vy, set VF = NOT borrow          
				if((V[(opcode & 0x0F00) >> 8]) > V[(opcode & 0x00F0) >> 4])
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
				pc += 2;      
				break;
			
			case 0x0006: // Set Vx = Vx SHR 1       
				if((V[(opcode & 0x0F00) >> 8] & 0x000F) == 1)
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] *= 2;
				pc += 2;      
			break;
			
			case 0x0007: // Set Vx = Vy - Vx, set VF = NOT borrow      
				if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
				pc += 2;       
				break;
			
			case 0x000E: // Set Vx = Vx SHL 1       
				if((V[(opcode & 0x0F00) >> 8] & 0xF000) == 1)
					V[0xF] = 1; //carry
				else
					V[0xF] = 0;
				V[(opcode & 0x0F00) >> 8] *= 2;
				pc += 2;      
				break;
			
			default:
				printf ("Unknown opcode [0x8000]: 0x%X\n", opcode);
				pc += 2;
				break;
		}
		break;
	
	case 0x9000: // Skip next instruction if Vx != Vy.
		if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
		{
			pc += 2;
		}
		pc += 2;
		break;
	
	case 0xA000: // ANNN: Sets I to the address NNN
		// Execute opcode
		index = opcode & 0x0FFF;
		pc += 2;
		break;
	
	case 0xB000: // BNNN: jump to nnn + V0
		pc = ((opcode & 0x0FFF) + V[0]);
		pc += 2;
		break;
 
	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = ((rand() % 0xFF) & (opcode & 0x00FF));
		pc += 2;
		break;
	
	case 0xD000:		   
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;
 
		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[index + yline];
			for(int xline = 0; xline < 8; xline++)
			{
				if((pixel & (0x80 >> xline)) != 0)
				{
					if(gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1; 
					}                       
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}
		drawFlag = true;
		pc += 2;
	}
	break;
	
	case 0xE000:
		switch(opcode & 0x00FF)
		{
		// EX9E: Skips the next instruction 
		// if the key stored in VX is pressed
			case 0x009E:
				if(key[V[(opcode & 0x0F00) >> 8]] != 0)
					pc += 4;
				else
					pc += 2;
		// EXA1: Skips the next instruction 
		// if the key stored in VX is not pressed
			case 0x00A1:
				if(key[V[(opcode & 0x0F00) >> 8]] == 0)
					pc += 4;
				else
					pc += 2;
			default:
				printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);  
				pc += 2;
				break;
		}
	break;
	
	case 0xF000:
		switch(opcode & 0x00FF)
		{
			
			
			case 0x0007: // Set Vx = delay timer value.
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				pc += 2;
				break;
			
			case 0x000A: // Wait for a key press, store the value of the key in Vx.
			{
				bool keyPress = false;
				for(int i = 0; i < 16; ++i)
				{
					if(key[index] != 0)
					{
						V[(opcode & 0x0F00) >> 8] = index;
						keyPress = true;
					}
				}
				if(!keyPress)
				{
					return;
				}
				pc += 2;
				break;
			}
			
			case 0x0015:
				delay_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			case 0x0018:
				sound_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			case 0x001E:
				if(index + V[(opcode & 0x0F00) >> 8] > 0xFFF)
					V[0xF] = 1;
				else
					V[0xF] = 0;
				index += V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			case 0x0029:
				index = V[(opcode & 0x0F00) >> 8] * 0x5;
				pc += 2;
				break;
			
			case 0x0033: // FX33: stores BCD of VX at index, index+1, and index+2
				memory[index]     = V[(opcode & 0x0F00) >> 8] / 100;
				memory[index + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[index + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
				pc += 2;
				break;
			
			default:
				printf("Unknown Opcode [0xF000]: 0x%X\n", opcode);
				pc += 2;
				break;
		}
		break;
	}  
 
	// Update timers
	if(delay_timer > 0)
		--delay_timer;
 
	if(sound_timer > 0)
	{
		--sound_timer;
	}  
	if (sound_timer == 1)
		cout << "BEEP!\a" << endl;
}

bool chip8::sdlInit()
{
	bool succ = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		succ = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			printf("Warning: Linear texture filtering not enabled!");

		//Create window
		gameWindow = SDL_CreateWindow("SDL-CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * X_SCALE) / 2, (SCREEN_HEIGHT * Y_SCALE) / 2, SDL_WINDOW_SHOWN);
		if (gameWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			succ = false;
		}
		else
		{
			//Create renderer for window
			gameRender = SDL_CreateRenderer(gameWindow, -1, SDL_RENDERER_ACCELERATED);
			SDL_RenderSetScale(gameRender, X_SCALE, Y_SCALE);

			if (gameRender == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				succ = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gameRender, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					succ = false;
				}
			}
		}
	}
	return succ;
}

bool chip8::loadFlags()
{
	bool success = true;
	return success;
}

void chip8::sdlClose()
{
	//Destroy window
	SDL_DestroyRenderer(gameRender);
	SDL_DestroyWindow(gameWindow);
	gameWindow = NULL;
	gameRender = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* chip8::loadTexture(string path)
{
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gameRender, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

void chip8::drawGraphics()
{
	//Blank the screen with black before composing the frame
	SDL_SetRenderDrawColor(gameRender, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(gameRender);

	//Compose the frame with white
	SDL_SetRenderDrawColor(gameRender, 0xFF, 0xFF, 0xFF, 0xFF);
	int rowNum;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			rowNum = y * 64;
			if (gfx[x + rowNum] != 0)
				SDL_RenderDrawPoint(gameRender, x, y);
		}
	}

	drawFlag = false;

	SDL_RenderPresent(gameRender);
	drawFlag = false;
}

void chip8::setupGraphics()
{
	//Start up SDL and create a window
	if (sdlInit())
		cout << "SDL Successfully Initialized!" << endl;
}

void chip8::setKeys()
{
	//Handle events on queue
	while (SDL_PollEvent(&e) != 0)
	{
		//User requests quit
		if (e.type == SDL_QUIT)
			quit = true;
		//User presses a key
		else if (e.type == SDL_KEYDOWN)
		{
			//Select surfaces based on key press
			switch (e.key.keysym.sym)
			{
			case SDLK_x:
				std::cout << "Key 'x' is pressed." << endl;
				key[0x0] = 1;
				break;

			case SDLK_1:
				std::cout << "Key '1' is pressed." << endl;
				key[0x1] = 1;
				break;

			case SDLK_2:
				std::cout << "Key '2' is pressed." << endl;
				key[0x2] = 1;
				break;

			case SDLK_3:
				std::cout << "Key '3' is pressed." << endl;
				key[0x3] = 1;
				break;

			case SDLK_q:
				std::cout << "Key 'q' is pressed." << endl;
				key[0x4] = 1;
				break;

			case SDLK_w:
				std::cout << "Key 'w' is pressed." << endl;
				key[0x5] = 1;
				break;

			case SDLK_e:
				std::cout << "Key 'e' is pressed." << endl;
				key[0x6] = 1;
				break;

			case SDLK_a:
				std::cout << "Key 'a' is pressed." << endl;
				key[0x7] = 1;
				break;

			case SDLK_s:
				std::cout << "Key 's' is pressed." << endl;
				key[0x8] = 1;
				break;

			case SDLK_d:
				std::cout << "Key 'd' is pressed." << endl;
				key[0x9] = 1;
				break;

			case SDLK_z:
				std::cout << "Key 'z' is pressed." << endl;
				key[0xA] = 1;
				break;

			case SDLK_c:
				std::cout << "Key 'c' is pressed." << endl;
				key[0xB] = 1;
				break;

			case SDLK_4:
				std::cout << "Key '4' is pressed." << endl;
				key[0xC] = 1;
				break;

			case SDLK_r:
				std::cout << "Key 'r' is pressed." << endl;
				key[0xD] = 1;
				break;

			case SDLK_f:
				std::cout << "Key 'f' is pressed." << endl;
				key[0xE] = 1;
				break;

			case SDLK_v:
				std::cout << "Key 'v' is pressed." << endl;
				key[0xF] = 1;
				break;

				//DEBUG:
			case SDLK_j:
				std::cout << "Key 'j' is pressed." << endl;
				break;
			}
		}
		if (e.type == SDL_KEYUP)
		{
			//Select surfaces based on key press
			switch (e.key.keysym.sym)
			{
			case SDLK_x:
				std::cout << "Key 'x' is released." << endl;
				key[0x0] = 0;
				break;

			case SDLK_1:
				std::cout << "Key '1' is released." << endl;
				key[0x1] = 0;
				break;

			case SDLK_2:
				std::cout << "Key '2' is released." << endl;
				key[0x2] = 0;
				break;

			case SDLK_3:
				std::cout << "Key '3' is released." << endl;
				key[0x3] = 0;
				break;

			case SDLK_q:
				std::cout << "Key 'q' is released." << endl;
				key[0x4] = 0;
				break;

			case SDLK_w:
				std::cout << "Key 'w' is released." << endl;
				key[0x5] = 0;
				break;

			case SDLK_e:
				std::cout << "Key 'e' is released." << endl;
				key[0x6] = 0;
				break;

			case SDLK_a:
				std::cout << "Key 'a' is released." << endl;
				key[0x7] = 0;
				break;

			case SDLK_s:
				std::cout << "Key 's' is released." << endl;
				key[0x8] = 0;
				break;

			case SDLK_d:
				std::cout << "Key 'd' is released." << endl;
				key[0x9] = 0;
				break;

			case SDLK_z:
				std::cout << "Key 'z' is released." << endl;
				key[0xA] = 0;
				break;

			case SDLK_c:
				std::cout << "Key 'c' is released." << endl;
				key[0xB] = 0;
				break;

			case SDLK_4:
				std::cout << "Key '4' is released." << endl;
				key[0xC] = 0;
				break;

			case SDLK_r:
				std::cout << "Key 'r' is released." << endl;
				key[0xD] = 0;
				break;

			case SDLK_f:
				std::cout << "Key 'f' is released." << endl;
				key[0xE] = 0;
				break;

			case SDLK_v:
				std::cout << "Key 'v' is released." << endl;
				key[0xF] = 0;
				break;
			}
		}
	}
}