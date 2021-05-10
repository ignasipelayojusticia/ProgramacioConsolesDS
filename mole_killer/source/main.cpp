
#include <nds.h>
#include <stdio.h>

#include <mole.h>
#include <background.h>

#define FRAMES_PER_ANIMATION 3
#define MOLES_BUFFER_SIZE 6

enum {SCREEN_TOP = 0, SCREEN_BOTTOM = 192, SCREEN_LEFT = 0, SCREEN_RIGHT = 256};

typedef struct 
{
	int x;
	int y;

	u16* sprite_gfx_mem;
	u8* frame_gfx;

	int state;
	int anim_frame;
} Mole;

touchPosition touch;
bool playing;

Mole moles[MOLES_BUFFER_SIZE];

void init();
void initMole(Mole *mole, u8* gfx);
void animateMole(Mole *mole);
void checkCollisionWithMole(Mole *mole, touchPosition touch);

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
	playing = true;

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_3_2D);

	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_SPRITE_0x06400000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	int bg3 = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	dmaCopy(backgroundBitmap, bgGetGfxPtr(bg3), 256*192);
	dmaCopy(backgroundPal, BG_PALETTE_SUB, 256*2);

	for(int i = 0; i < MOLES_BUFFER_SIZE; i++)
	{
		moles[i] = {35 * i + 10, 50};
		initMole(&moles[i], (u8*)moleTiles);
	}
	//initMole(&mole, (u8*)moleTiles);
	dmaCopy(molePal, SPRITE_PALETTE_SUB, 512);
}

void initMole(Mole *sprite, u8* gfx)
{
	sprite->sprite_gfx_mem = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);
	sprite->frame_gfx = (u8*)gfx;
}

void animateMole(Mole *sprite)
{
	int frame = sprite->anim_frame + sprite->state * FRAMES_PER_ANIMATION;
    u8* offset = sprite->frame_gfx + frame * 32*32;

    dmaCopy(offset, sprite->sprite_gfx_mem, 32*32);
}


void checkCollisionWithMole(Mole *mole, touchPosition touch)
{
	if(touch.px >= mole->x && touch.px <= mole->x + 32 && touch.py >= mole->y && touch.py <= mole->y + 32)
	{
		mole->x = 256;
		mole->y = 192;

		// Puntuación ++
	}
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
		
	for(int i = 0; i < MOLES_BUFFER_SIZE; i++)
	{
		animateMole(&moles[i]);
		oamSet(&oamSub, i , moles[i].x, moles[i].y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 
			moles[i].sprite_gfx_mem, -1, false, false, false, false, false);
	}

	swiWaitForVBlank();

	oamUpdate(&oamMain);
	oamUpdate(&oamSub);
	
	if(held & KEY_TOUCH)
	{
		touchRead(&touch);

		for(int i = 0; i < MOLES_BUFFER_SIZE; i++)
		{
			checkCollisionWithMole(&moles[i], touch);
		}
		
		iprintf("\x1b[10;0Hx = %04i - %04i\n", touch.rawx, touch.px);
		iprintf("y = %04i - %04i\n", touch.rawy, touch.py);
	}
}