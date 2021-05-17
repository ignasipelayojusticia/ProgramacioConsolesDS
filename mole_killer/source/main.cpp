
#include <nds.h>
#include <stdio.h>

#include <score.h>
#include <mole.h>
#include <background.h>
#include <scoreScreen.h>

#define MOLES_BUFFER_SIZE 6
#define SCORE_DIGITS 2
#define SCORE_X_INITIAL_POSITION 170
#define SCORE_Y_INITIAL_POSITION 93

enum {SCREEN_TOP = 0, SCREEN_BOTTOM = 192, SCREEN_LEFT = 0, SCREEN_RIGHT = 256};

typedef struct 
{
	int x;
	int y;

	u16* sprite_gfx_mem;
	u8* frame_gfx;

	int state;
	int anim_frame;

	float timeToSpawn;
	float timeOnScreen;
} Mole;

typedef struct 
{
	int x;
	int y;

	u16* sprite_gfx_mem;
	u8* frame_gfx;

	int anim_frame;
} Score;


touchPosition touch;
bool playing;
int score;
int hundreds;

Mole moles[MOLES_BUFFER_SIZE];
Score scores[SCORE_DIGITS];

void init();
void initScore(const int& posX, Score *score, u8* gfx);
void animateScore(const int& value, Score *score);
void initMole(const int& id, Mole *mole, u8* gfx);
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
	score = 0;

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_3_2D);

	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_SPRITE_0x06400000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	int mainBackground = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	dmaCopy(scoreScreenBitmap, bgGetGfxPtr(mainBackground), 256*192);
	dmaCopy(scoreScreenPal, BG_PALETTE, 256*2);

	int subBackground = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	dmaCopy(backgroundBitmap, bgGetGfxPtr(subBackground), 256*192);
	dmaCopy(backgroundPal, BG_PALETTE_SUB, 256*2);

	for(int i = 0; i < SCORE_DIGITS; i++)
	{
		initScore(SCORE_X_INITIAL_POSITION + i * 32, &scores[i], (u8*)scoreTiles);
	}
	dmaCopy(scorePal, SPRITE_PALETTE, 512);

	for(int i = 0; i < MOLES_BUFFER_SIZE; i++)
	{
		initMole(i, &moles[i], (u8*)moleTiles);
	}
	dmaCopy(molePal, SPRITE_PALETTE_SUB, 512);
}

void initScore(const int& posX, Score *score, u8* gfx)
{
	score->x = posX;
	score->y = SCORE_Y_INITIAL_POSITION;

	score->anim_frame = 0;

	score->sprite_gfx_mem = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	score->frame_gfx = (u8*)gfx;
}

void animateScore(const int& value, Score *score)
{
	u8* offset = score->frame_gfx + value * 32*32;

	dmaCopy(offset, score->sprite_gfx_mem, 32*32);
}

void initMole(const int& id, Mole *mole, u8* gfx)
{
	mole->x = (26 + 85 * id) % 256;
	mole->y = 60 + 80 * (id / 3);

	mole->sprite_gfx_mem = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);
	mole->frame_gfx = (u8*)gfx;

	mole->timeToSpawn = 100 + (rand() & 0xFF);
	mole->timeOnScreen = -1;
}

void animateMole(Mole *mole)
{
	mole->anim_frame = 0;

	mole->timeToSpawn--;
	if(mole->timeToSpawn <= 0 && mole->timeOnScreen <= 0)
	{
		mole->timeOnScreen = 45 + (rand() & 30);
	}
	else if(mole->timeToSpawn <= 0)
	{
		mole->timeOnScreen--;
		if(mole->timeOnScreen <= 0)
		{
			mole->timeToSpawn = 100 + (rand() & 0xAF);
		}
		else
		{
			mole->anim_frame = 1;
		}
	}

	u8* offset = mole->frame_gfx + mole->anim_frame * 32*32;
	dmaCopy(offset, mole->sprite_gfx_mem, 32*32);
}


void checkCollisionWithMole(Mole *mole, touchPosition touch)
{
	if(mole->anim_frame == 1 && touch.px >= mole->x && touch.px <= mole->x + 32 && 
		touch.py >= mole->y && touch.py <= mole->y + 32)
	{
		mole->timeOnScreen = 0;
		mole->timeToSpawn = 100 + (rand() & 0xAF);

		score++;
		if(score >= 100)
		{
			//gameover
		}
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
	
	animateScore(score/10, &scores[0]);
	animateScore(score - (score/10) * 10, &scores[1]);

	for(int i = 0; i < SCORE_DIGITS; i++)
	{
		oamSet(&oamMain, i , scores[i].x, scores[i].y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 
			scores[i].sprite_gfx_mem, -1, false, false, false, false, false);
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