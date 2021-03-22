#include <nds.h>
#include <stdio.h>

int main(void) 
{
	touchPosition touch;

	consoleDemoInit(); 

	int area = 0;

	while(1) 
	{
		scanKeys();
		int held = keysHeld();

		if(held & KEY_START) break;

		if(held & KEY_TOUCH)
		{
			touchRead(&touch);

			iprintf("\x1b[10;0Hx = %04i - %04i\n", touch.rawx, touch.px);
			iprintf("y = %04i - %04i\n", touch.rawy, touch.py);


		}

		swiWaitForVBlank();
	}

	return 0;
}