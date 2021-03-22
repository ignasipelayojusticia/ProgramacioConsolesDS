#include <nds.h>
#include <stdio.h>

touchPosition touch;
bool playing;

void init();
void quit();
void step();

int main(void) 
{
	init();

	while(playing) 
	{
		step();
	}

	quit();

	return 0;
}

void init()
{
	consoleDemoInit(); 
	playing = true;
}

void quit()
{

}

void step()
{
	scanKeys();
	int held = keysHeld();

	if(held & KEY_START)
	{
		playing = false;
		return;
	}

	if(held & KEY_TOUCH)
	{
		touchRead(&touch);

		iprintf("\x1b[10;0Hx = %04i - %04i\n", touch.rawx, touch.px);
		iprintf("y = %04i - %04i\n", touch.rawy, touch.py);
	}

	swiWaitForVBlank();
}