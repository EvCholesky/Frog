#include "SimpleMaze.h"
#include "FrogInput.h"

#include <stdio.h>

typedef enum FDIR_tag
{
	FDIR_None	= 0x0,
	FDIR_L		= 0x1,
	FDIR_U		= 0x2,
	FDIR_R		= 0x4,
	FDIR_D		= 0x8,

	FDIR_____   = FDIR_None,
	FDIR_L___   = FDIR_L,
	FDIR__U__   = FDIR_U,
	FDIR_LU__   = FDIR_L | FDIR_U,
	FDIR___R_   = FDIR_R,
	FDIR_L_R_   = FDIR_L | FDIR_R,
	FDIR__UR_   = FDIR_U | FDIR_R,
	FDIR_LUR_   = FDIR_L | FDIR_U | FDIR_R,
	FDIR____D   = FDIR_D,
	FDIR_L__D   = FDIR_L | FDIR_D,
	FDIR__U_D   = FDIR_U | FDIR_D,
	FDIR_LU_D   = FDIR_L | FDIR_U | FDIR_D,
	FDIR___RD   = FDIR_R | FDIR_D,
	FDIR_L_RD   = FDIR_L | FDIR_R | FDIR_D,
	FDIR__URD   = FDIR_U | FDIR_R | FDIR_D,
	FDIR_LURD	= FDIR_L | FDIR_U | FDIR_R | FDIR_D
} FDIR;

#define kDXMaze 6
#define kDYMaze 6

static const int s_xStart = 0;
static const int s_yStart = 0;
static const int s_xEnd = 5;
static const int s_yEnd = 5;

static FDIR s_aFdir[] = 
{
	FDIR____D,	FDIR___RD,	FDIR_L_R_, FDIR_L__D, FDIR_____, FDIR_____,
	FDIR__UR_,	FDIR_LU__,	FDIR___RD, FDIR_LU__, FDIR_____, FDIR_____,
	FDIR___RD,	FDIR_L_R_,	FDIR_LURD, FDIR_L__D, FDIR_____, FDIR_____,
	FDIR__U_D,	FDIR_____,	FDIR__UR_, FDIR_LURD, FDIR_L___, FDIR_____,
	FDIR__URD,	FDIR_L__D,	FDIR___R_, FDIR_LU_D, FDIR_____, FDIR_____,
	FDIR__U__,	FDIR__UR_,	FDIR_L___, FDIR__UR_, FDIR_L_R_, FDIR_L___,
};



static const char * s_pChzRoom =
"WWWWWWWWWWWW" // 0
"W..........W"
"W..........W"
"W..........W"
"W..........W" // 4
"W..........W"
"W..........W"
"W..........W"
"W..........W" // 8
"W..........W"
"W..........W"
"WWWWWWWWWWWW";

const static char * s_pChzRoomA =
"WWWWWWWWWWWW" // 0
"WWW......WWW"
"WWW......WWW"
"W..........W"
"W..........W" // 4
"W..........W"
"W..........W"
"W..........W"
"W..........W" // 8
"WWW......WWW"
"WWW......WWW"
"WWWWWWWWWWWW";

static const char * s_pChzRoomB =
"WWWWWWWWWWWW" // 0
"W..........W"
"W..........W"
"W....WW....W"
"W...WWWW...W" // 4
"W..WW..WW..W"
"W..WW..WW..W"
"W...WWWW...W"
"W....WW....W" // 8
"W..........W"
"W..........W"
"WWWWWWWWWWWW";

static const char * s_pChzRoomStart =
"WWWWWWWWWWWW" // 0
"W..........W"
"W....SS....W"
"W...S..S...W"
"W...S......W" // 4
"W....SS....W"
"W......S...W"
"W...S..S...W"
"W....SS....W" // 8
"W..........W"
"W..........W"
"WWWWWWWWWWWW";

static const char * s_pChzRoomEnd =
"WWWWWWWWWWWW" // 0
"W..........W"
"W...EEEEE..W"
"W...E......W"
"W...EEE....W" // 4
"W...E......W"
"W...E......W"
"W...EEEEE..W"
"W..........W" // 8
"W..........W"
"W..........W"
"WWWWWWWWWWWW";

#define kDXTile 12
#define kDYTile 12

static FDIR FdirFromXyScreen(int xScr, int yScr)
{
	FR_ASSERT(xScr < kDXMaze && yScr < kDYMaze, "bad screen coordinate");
	return s_aFdir[xScr + yScr * kDYMaze];
}

static inline void SetScreenTile(char * aCh, int x, int y, char ch)
{
	aCh[x + y*kDXTile] = ch;
}

static inline void WriteScreenTileHorizLine(char * aCh, int x, int y, char ch, int cCh)
{
	int iChBase = x + y*kDXTile;
	for (int iCh = 0; iCh < cCh; ++iCh)
	{
		aCh[iChBase + iCh] = ch;
	}
}

static FrRoom * PRoomCreate(SimpleMaze * pMaze, int x, int y)
{
	char aCh[kDXTile * kDYTile];

	if (x == s_xStart && y == s_yStart)
	{
		CopyAB(s_pChzRoomStart, aCh, FR_DIM(aCh));
	}
	else if (x == s_xEnd && y == s_yEnd)
	{
		CopyAB(s_pChzRoomEnd, aCh, FR_DIM(aCh));
	}
	else if ((x % 2) == 1 && (y % 2) == 0)
	{
		CopyAB(s_pChzRoomA, aCh, FR_DIM(aCh));
	}
	else if ((x % 2) == 0 && (y % 2) == 1)
	{
		CopyAB(s_pChzRoomB, aCh, FR_DIM(aCh));
	}
	else
	{
		CopyAB(s_pChzRoom, aCh, FR_DIM(aCh));
	}

	FDIR fdir = FdirFromXyScreen(x, y);
	if (fdir & FDIR_U)
	{
		SetScreenTile(aCh, 4, 0, '.');
		SetScreenTile(aCh, 5, 0, '.');
		SetScreenTile(aCh, 6, 0, '.');
		SetScreenTile(aCh, 7, 0, '.');
	}

	if (fdir & FDIR_D)
	{
		SetScreenTile(aCh, 4, 11, '.');
		SetScreenTile(aCh, 5, 11, '.');
		SetScreenTile(aCh, 6, 11, '.');
		SetScreenTile(aCh, 7, 11, '.');
	}

	if (fdir & FDIR_L)
	{
		SetScreenTile(aCh, 0, 4, '.');
		SetScreenTile(aCh, 0, 5, '.');
		SetScreenTile(aCh, 0, 6, '.');
		SetScreenTile(aCh, 0, 7, '.');

		if (y == 2)
		{
			WriteScreenTileHorizLine(aCh, 0, 5, '_', 6);
			WriteScreenTileHorizLine(aCh, 0, 6, '_', 6);
		}
	}

	if (fdir & FDIR_R)
	{
		SetScreenTile(aCh, 11, 4, '.');
		SetScreenTile(aCh, 11, 5, '.');
		SetScreenTile(aCh, 11, 6, '.');
		SetScreenTile(aCh, 11, 7, '.');

		if (y == 2)
		{
			WriteScreenTileHorizLine(aCh, 6, 5, '_', 6);
			WriteScreenTileHorizLine(aCh, 6, 6, '_', 6);
		}
	}

	aCh[14] = 'L';
	aCh[15] = 'L';
	aCh[26] = 'L';
	aCh[27] = 'L';
	FrRoom * pRoom = Frog_PRoomAllocate(&pMaze->m_tworld, kDXTile, kDYTile, 32, 32);
	Frog_SetRoomTiles(pRoom, &pMaze->m_tworld.m_tmap, aCh);
	return pRoom;
}

static void SetCurScreen(SimpleMaze * pMaze, int xNew, int yNew)
{
	pMaze->m_pRoomCur = PRoomCreate(pMaze, xNew, yNew);
	pMaze->m_xScr = xNew;
	pMaze->m_yScr = yNew;

	Frog_SetTransition(&pMaze->m_roomt, ROOMTK_None, NULL, pMaze->m_pRoomCur);
}

static void TranslateCurScreen(SimpleMaze * pMaze, int xNew, int yNew, FrVec2 dPosTransit)
{
	FrRoom * pRoomPrev = pMaze->m_pRoomCur;

	pMaze->m_pRoomCur = PRoomCreate(pMaze, xNew, yNew);
	pMaze->m_xScr = xNew;
	pMaze->m_yScr = yNew;

	if (!pRoomPrev)
	{
		Frog_SetTransition(&pMaze->m_roomt, ROOMTK_None, NULL, pMaze->m_pRoomCur);
	}
	else
	{
		Frog_SetTransition(&pMaze->m_roomt, ROOMTK_Translate, pRoomPrev, pMaze->m_pRoomCur);
		pMaze->m_roomt.m_dPos = dPosTransit;
	}
}

void InitSimpleMaze(SimpleMaze * pMaze)
{
	Frog_InitTileWorld(&pMaze->m_tworld);

	FrColor colWallFg = Frog_ColCreate(0xFFa5c9c3);
	FrColor colWallBg = Frog_ColCreate(0xFF85aaa4);
	FrColor colGrassFg = Frog_ColFromRGB(100, 200, 50); //(0xFF267043); //#437026
	FrColor colGrassBg = Frog_ColCreate(0xFF156f3a); //#3a6f16
	FrColor colItemFg = Frog_ColCreate(0xFFa4d8d1); //#d1d8a4
	FrColor colItemBg = Frog_ColCreate(0xFF383e26); //#383e26
	FrColor colPathFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colPathBg = Frog_ColCreate(0xFF52afbf); //#bfaf52
	FrColor colLavaFg = Frog_ColFromRGB(255, 150, 150);
	FrColor colLavaBg = Frog_ColFromRGB(200, 50, 50);

	FrTileMap * pTmap = &pMaze->m_tworld.m_tmap;
	//Frog_SetTile(&pMaze->m_tmap, 'W', L'▓', colWallFg, colWallBg);
	Frog_SetTile(pTmap, 'W', 'W', colWallFg, colWallBg, FTILE_None);
	Frog_SetTile(pTmap, 'S', 'S', colWallFg, colWallBg, FTILE_None);
	Frog_SetTile(pTmap, 'E', 'E', colWallFg, colWallBg, FTILE_None);
	Frog_SetTile(pTmap, 'L', 'L', colLavaFg, colLavaBg, FTILE_None);
	Frog_SetTile(pTmap, '.', ' ', colGrassFg, colGrassBg, FTILE_None);
	Frog_SetTile(pTmap, '_', ' ', colPathFg, colPathBg, FTILE_None);
	Frog_SetTile(pTmap, 'C', 'C', colItemFg, colItemBg, FTILE_None);
	Frog_SetTile(pTmap, '@', '@', colItemFg, colItemBg, FTILE_None);

	pMaze->m_xScr = s_xStart;
	pMaze->m_yScr = s_yStart;

	pMaze->m_pRoomCur = NULL;
	SetCurScreen(pMaze, pMaze->m_xScr, pMaze->m_yScr);
}

static void UpdateInput(SimpleMaze * pMaze, FrInput * pInput)
{
	Frog_PollInput(pInput);

	if (pMaze->m_roomt.m_roomtk != ROOMTK_None)
		return;

	FrInputEventIterator inevit = Frog_Inevit(pInput->m_pInevfifo);
	FrInputEvent * pInev;
	while (pInev = Frog_PInevNext(&inevit))
	{
		if (pInev->m_edges != EDGES_Press)
			continue;

		FrRoom * pRoomCur = pMaze->m_pRoomCur;
		f32 dXScreen = (f32)pRoomCur->m_dX * pRoomCur->m_dXCharPixel;
		f32 dYScreen = (f32)pRoomCur->m_dY * pRoomCur->m_dYCharPixel;

		FDIR fdir = FdirFromXyScreen(pMaze->m_xScr, pMaze->m_yScr);
		switch (pInev->m_keycode)
		{
		case KEYCODE_ArrowUp:
			if ((fdir & FDIR_U) && pMaze->m_yScr > 0)
			{
				TranslateCurScreen(pMaze, pMaze->m_xScr, pMaze->m_yScr - 1, Frog_Vec2Create(0.0f, -dYScreen));
			}
			break;
		case KEYCODE_ArrowDown:
			if ((fdir & FDIR_D) && pMaze->m_yScr < kDYMaze-1)
			{
				TranslateCurScreen(pMaze, pMaze->m_xScr, pMaze->m_yScr + 1, Frog_Vec2Create(0.0f, dYScreen));
			}
			break;
		case KEYCODE_ArrowLeft:
			if ((fdir & FDIR_L) && pMaze->m_xScr > 0)
			{
				TranslateCurScreen(pMaze, pMaze->m_xScr - 1, pMaze->m_yScr, Frog_Vec2Create(dXScreen, 0.0f));
			}
			break;
		case KEYCODE_ArrowRight:
			if ((fdir & FDIR_R) && pMaze->m_xScr < kDXMaze-1)
			{
				TranslateCurScreen(pMaze, pMaze->m_xScr + 1, pMaze->m_yScr, Frog_Vec2Create(-dXScreen, 0.0f));
			}
			break;
		}
	}
	Frog_ClearInputEvents(pInput->m_pInevfifo);
}

void UpdateSimpleMaze(SimpleMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT)
{
	FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	//static const char * s_pChzQuickFox = "TheQuickBrownFoxJumpsOverTheLazyDog";
	FrVec2 posText = Frog_Vec2Create(20.0f, 20.0f);

	char aCh[32];
	sprintf_s(aCh, FR_DIM(aCh), "(%d, %d)", pMaze->m_xScr, pMaze->m_yScr);
	Frog_DrawTextRaw(pDrac, posText, aCh);

	FrRoomTransition * pRoomt = &pMaze->m_roomt;
	Frog_UpdateTransition(pRoomt, dT);
	if (pRoomt->m_r >= 1.0f)
	{
		Frog_FreeRoom(&pMaze->m_tworld, pRoomt->m_pRoomPrev);
		Frog_SetTransition(pRoomt, ROOMTK_None, NULL, pRoomt->m_pRoom);
	}

	Frog_RenderTransition(pDrac, &pMaze->m_tworld, &pMaze->m_roomt, s_posScreen);

	UpdateInput(pMaze, pInput);
}
