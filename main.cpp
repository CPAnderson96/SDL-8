#include <SDL.h>
#include "chip8.h"

using namespace std;

int main(int argc, char **argv)
{
	chip8 play; // game object

	// set up render system and register input
	play.setupGraphics();

	// initialize Chip8 system and load the game into the memory
	play.initialize();

	// emulation loop
	for (;;)
	{
		// emulate one cycle 
		play.emulateCycle();

		// if the draw flag is set, update the screen
		if (play.drawFlag)
		{
			play.drawGraphics();
			play.drawFlag = false;
			SDL_Delay(FRAME_DELAY);
		}

		// store key press state
		play.setKeys();
	}
	play.sdlClose();
	return 0;
}