/*
#include <nds.h>
#include <stdio.h>

#include <mole.h>

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
Mole mole;

void init();
void initMole(Mole *mole, u8* gfx);

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

	mole = {0,0};

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	initMole(&mole, (u8*)moleTiles);

	dmaCopy(molePal, SPRITE_PALETTE, 512);
}

void initMole(Mole *mole, u8* gfx)
{
	mole->sprite_gfx_mem = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);
	mole->frame_gfx = gfx;
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

	int frame = mole.anim_frame + mole.state * 3;
	u8* offset = mole.frame_gfx + frame * 32*32;
	dmaCopy(offset, mole.sprite_gfx_mem, 32*32);

	oamSet(&oamMain, 0, mole.x, mole.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, mole.sprite_gfx_mem, -1, false, false, false, false, false);
	oamSet(&oamSub, 0, mole.x, mole.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, mole.sprite_gfx_mem, -1, false, false, false, false, false);

	swiWaitForVBlank();

	oamUpdate(&oamMain);
	oamUpdate(&oamSub);

	
	if(held & KEY_TOUCH)
	{
		touchRead(&touch);

		iprintf("\x1b[10;0Hx = %04i - %04i\n", touch.rawx, touch.px);
		iprintf("y = %04i - %04i\n", touch.rawy, touch.py);
	}
}
*/

#include <nds.h>

#include <mole.h>

#define FRAMES_PER_ANIMATION 3

//---------------------------------------------------------------------
// The man sprite
// he needs a single pointer to sprite memory
// and a reference to his frame graphics so they
// can be loaded as needed
//---------------------------------------------------------------------
typedef struct 
{
	int x;
	int y;

	u16* sprite_gfx_mem;
	u8*  frame_gfx;

	int state;
	int anim_frame;
}Man;

//---------------------------------------------------------------------
// The womman sprite
// she needs an array of pointers to sprite memory since all 
// her frames are to be loaded.
// she also needs to keep track of which sprite memory pointer is in use
//---------------------------------------------------------------------
typedef struct
{
	int x;
	int y;

	u16* sprite_gfx_mem[12];
	int gfx_frame;

	int state;
	int anim_frame;


}Woman;

//---------------------------------------------------------------------
// The state of the sprite (which way it is walking)
//---------------------------------------------------------------------
enum SpriteState {W_UP = 0, W_RIGHT = 1, W_DOWN = 2, W_LEFT = 3};

//---------------------------------------------------------------------
// Screen dimentions
//---------------------------------------------------------------------
enum {SCREEN_TOP = 0, SCREEN_BOTTOM = 192, SCREEN_LEFT = 0, SCREEN_RIGHT = 256};

//---------------------------------------------------------------------
// Animating a man requires us to copy in a new frame of data each time
//---------------------------------------------------------------------
void animateMan(Man *sprite)
{
	int frame = sprite->anim_frame + sprite->state * FRAMES_PER_ANIMATION;

	u8* offset = sprite->frame_gfx + frame * 32*32;

	dmaCopy(offset, sprite->sprite_gfx_mem, 32*32);
}

//---------------------------------------------------------------------
// Initializing a man requires little work, allocate room for one frame
// and set the frame gfx pointer
//---------------------------------------------------------------------
void initMan(Man *sprite, u8* gfx)
{
	sprite->sprite_gfx_mem = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	
	sprite->frame_gfx = (u8*)gfx;
}

//---------------------------------------------------------------------
// Animating a woman only requires us to alter which sprite memory pointer
// she is using
//---------------------------------------------------------------------
void animateWoman(Woman *sprite)
{
	sprite->gfx_frame = sprite->anim_frame + sprite->state * FRAMES_PER_ANIMATION;
}

//---------------------------------------------------------------------
// Initializing a woman requires us to load all of her graphics frames 
// into memory
//---------------------------------------------------------------------
void initWoman(Woman *sprite, u8* gfx)
{
	int i;

	for(i = 0; i < 12; i++)
	{
		sprite->sprite_gfx_mem[i] = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);
		dmaCopy(gfx, sprite->sprite_gfx_mem[i], 32*32);
		gfx += 32*32;
	}
}


//---------------------------------------------------------------------
// main
//---------------------------------------------------------------------
int main(void) 
{
	Man man = {0,0};
	Woman woman = {0,0};

	//-----------------------------------------------------------------
	// Initialize the graphics engines
	//-----------------------------------------------------------------
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	//-----------------------------------------------------------------
	// Initialize the two sprites
	//-----------------------------------------------------------------
	initMan(&man, (u8*)moleTiles);
	initWoman(&woman, (u8*)moleTiles);

	dmaCopy(molePal, SPRITE_PALETTE, 512);
	dmaCopy(molePal, SPRITE_PALETTE_SUB, 512);

	//-----------------------------------------------------------------
	// main loop
	//-----------------------------------------------------------------
	while(1) 
	{
		scanKeys();

		int keys = keysHeld();
		
		if(keys & KEY_START) break;

		if(keys)
		{
			if(keys & KEY_UP)
			{
				if(man.y >= SCREEN_TOP) man.y--;
				if(woman.y >= SCREEN_TOP) woman.y--;

				man.state = woman.state = W_UP;
			}
			if(keys & KEY_LEFT)
			{
				if(man.x >= SCREEN_LEFT) man.x--;
				if(woman.x >= SCREEN_LEFT) woman.x--;

				man.state = woman.state = W_LEFT;
			}
			if(keys & KEY_RIGHT)
			{
				if(man.x <= SCREEN_RIGHT) man.x++;
				if(woman.x <= SCREEN_RIGHT) woman.x++;

				man.state = woman.state = W_RIGHT;
			}
			if(keys & KEY_DOWN)
			{
				if(man.y <= SCREEN_BOTTOM) man.y++;
				if(woman.y <= SCREEN_BOTTOM) woman.y++;

				man.state = woman.state = W_DOWN;
			}
		
			man.anim_frame++;
			woman.anim_frame++;

			if(man.anim_frame >= FRAMES_PER_ANIMATION) man.anim_frame = 0;
			if(woman.anim_frame >= FRAMES_PER_ANIMATION) woman.anim_frame = 0;

		}

		animateMan(&man);
		animateWoman(&woman);

		//-----------------------------------------------------------------
		// Set oam attributes, notice the only difference is in the sprite 
		// graphics memory pointer argument.  The man only has one pointer
		// while the women has an array of pointers
		//-----------------------------------------------------------------
		oamSet(&oamMain, 0, man.x, man.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 
			man.sprite_gfx_mem, -1, false, false, false, false, false);
		
		oamSet(&oamSub, 0, woman.x, woman.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 
			woman.sprite_gfx_mem[woman.gfx_frame], -1, false, false, false, false, false);

		swiWaitForVBlank();

		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}

	return 0;
}
