#include "EntityMaze.h"
#include "FrogInput.h"

#include <stdio.h>
#include <stdlib.h>

void OnEnterRoom(EntityMaze * pMaze, GameRoom * pGroom);
void OnExitRoom(EntityMaze * pMaze, GameRoom * pGroom);

void FreeEntity(EntityMaze * pMaze, GameEntity * pGent);

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
enum { kCEntityCellMax = 20 };

static const int s_xStart = 0;
static const int s_yStart = 0;
static const int s_xEnd = 5;
static const int s_yEnd = 5;
static const int s_xScrLock = 3;
static const int s_yScrLock = 4;
static const int s_xScrKey = 2;
static const int s_yScrKey = 4;

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

static const char * s_pChzRoomA =
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

static const char * s_pChzLocked =
"WWWWWWWWWWWW" // 0
"W..........W"
"W..........W"
"W..........W"
"W..........W" // 4
"W..........W"
"W..........W"
"W..WW.WWW..W"
"W..W....W..W" // 8
"W..W....W..W"
"W..W....W..W"
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
#define kXyCharPixel 32

FDIR static FdirFromXyScreen(int xScr, int yScr)
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

static FrRoom * PRoomCreate(EntityMaze * pMaze, int x, int y)
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
	else if (x == s_xScrLock && y == s_yScrLock)
	{
		CopyAB(s_pChzLocked, aCh, FR_DIM(aCh));
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

	FrRoom * pRoom = Frog_PRoomAllocate(&pMaze->m_tworld, kDXTile, kDYTile, kXyCharPixel, kXyCharPixel);
	Frog_SetRoomTiles(pRoom, &pMaze->m_tworld.m_tmap, aCh);
	return pRoom;
}

static void SetCurRoom(EntityMaze * pMaze, int xScrNew, int yScrNew)
{
	int xyRoom = xScrNew + yScrNew * kDXMaze;
	ROOMID roomid = pMaze->m_mpXyRoomid[xyRoom];
	GameRoom * pGroom = &pMaze->m_aGroom[roomid];

	pMaze->m_pGroomCur = pGroom;
	pMaze->m_xScr = xScrNew;
	pMaze->m_yScr = yScrNew;

	Frog_SetTransition(&pMaze->m_roomt, ROOMTK_None, NULL, pGroom->m_pRoom);

	OnEnterRoom(pMaze, pGroom);
}

static void TranslateCurScreen(EntityMaze * pMaze, int xScrNew, int yScrNew, FrVec2 dPosTransit)
{
	GameRoom * pGroomPrev = pMaze->m_pGroomCur;

	int xyRoomNew = xScrNew + yScrNew * kDXMaze;
	ROOMID roomidNew = pMaze->m_mpXyRoomid[xyRoomNew];
	GameRoom * pGroomNew  = &pMaze->m_aGroom[roomidNew];

	pMaze->m_pGroomCur = pGroomNew;
	pMaze->m_xScr = xScrNew;
	pMaze->m_yScr = yScrNew;
	OnEnterRoom(pMaze, pGroomNew);

	if (!pGroomPrev)
	{
		Frog_SetTransition(&pMaze->m_roomt, ROOMTK_None, NULL, pGroomNew->m_pRoom);
	}
	else
	{
		Frog_SetTransition(&pMaze->m_roomt, ROOMTK_Translate, pGroomPrev->m_pRoom, pGroomNew->m_pRoom);
		pMaze->m_roomt.m_dPos = dPosTransit;
	}
}

bool FIsCellOpen(EntityMaze * pMaze, int xCellAvatar, int yCellAvatar, int dXScreen, int dYScreen)
{
	if (dXScreen != 0 || dYScreen != 0)
	{
		// BB - Check other rooms
		return true;
	}

	FrRoom * pRoom = pMaze->m_pGroomCur->m_pRoom;
	FTILE ftile = (FTILE)Frog_FtileFromCell(pRoom, xCellAvatar, yCellAvatar);
	return (ftile & FTILE_Collide) == 0;
}

void HandleBumpAvatar(EntityMaze * pMaze, GameEntity * pGentSrc, ENTID entidDest, bool * pFAllowMove)
{
	GameEntity * pGentDest = &pMaze->m_mpEntidEnt[entidDest];

	switch (pGentDest->m_entk)
	{
	case ENTK_Key:
		++pMaze->m_aCIik[IIK_Key];
		Frog_PostNote(&pMaze->m_noteq, NOTEK_Normal, "Collected Key");	
		FreeEntity(pMaze, pGentDest);
		break;
	case ENTK_Coin:
		++pMaze->m_aCIik[IIK_Coin];
		Frog_PostNote(&pMaze->m_noteq, NOTEK_Normal, "Collected Coin");	
		FreeEntity(pMaze, pGentDest);
		break;
	case ENTK_Lock:
		if (pMaze->m_aCIik[IIK_Key] > 0)
		{
			pMaze->m_aCIik[IIK_Key];
			Frog_PostNote(&pMaze->m_noteq, NOTEK_Normal, "Unlocked Door");	
			FreeEntity(pMaze, pGentDest);
		}
		else
		{
			Frog_PostNote(&pMaze->m_noteq, NOTEK_Normal, "Door is Locked");	
			*pFAllowMove = false;
		}
		break;
	}
}

bool FTryMoveEntity(EntityMaze * pMaze, GameEntity * pGent, int xCellNew, int yCellNew, GameRoom * pGroomPrev, GameRoom * pGroomNext)
{
	ENTID aEntid[kCEntityCellMax];
	FrCellContents cellc;
	Frog_InitCellContents(aEntid, FR_DIM(aEntid), &cellc);
	Frog_FindCellContents(&pMaze->m_tworld, pGroomNext->m_pRoom, xCellNew, yCellNew, &cellc);

	bool fAllowMove = (cellc.m_tile.m_ftile & FTILE_Collide) == 0;
	
	ENTID * pEntidMax = &cellc.m_aEntid[cellc.m_cEntid];
	for (ENTID * pEntidDest = cellc.m_aEntid; pEntidDest != pEntidMax; ++pEntidDest)
	{
		switch (pGent->m_entk)
		{
		case ENTK_Avatar:
			HandleBumpAvatar(pMaze, pGent, *pEntidDest, &fAllowMove);
			break;
		case ENTK_Monster:
			{
				GameEntity * pGentDest = &pMaze->m_mpEntidEnt[*pEntidDest];
				if (pGentDest->m_entk == ENTK_Avatar)
				{
					if (pMaze->m_aCIik[IIK_Coin])
					{
						pMaze->m_aCIik[IIK_Coin] = 0;
						Frog_PostNote(&pMaze->m_noteq, NOTEK_Normal, "Monster stole coins");	
					}
				}
			} break;
		}
	}
	return fAllowMove;
}

static void MoveAvatar(EntityMaze * pMaze, GameEntity * pGent, int dX, int dY)
{
	GameRoom * pGroom = pMaze->m_pGroomCur;
	FrRoom * pRoom = pGroom->m_pRoom;

	int xNew = pGent->m_x + dX;
	int yNew = pGent->m_y + dY;
	int dXScreen = 0;
	int dYScreen = 0;

	f32 dXPixelScr = (f32)pRoom->m_dX * pRoom->m_dXCharPixel;
	f32 dYPixelScr = (f32)pRoom->m_dY * pRoom->m_dYCharPixel;
	FDIR fdir = FdirFromXyScreen(pMaze->m_xScr, pMaze->m_yScr);
	if (xNew < 0)
	{
		xNew = pGent->m_x;
		if ((fdir & FDIR_L) && pMaze->m_xScr > 0)
		{
			dXScreen = -1;
			xNew = pRoom->m_dX - 1;
		}
	}
	else if (xNew >= pRoom->m_dX)
	{
		xNew = pGent->m_x;
		if ((fdir & FDIR_R) && pMaze->m_xScr < kDXMaze-1)
		{
			dXScreen = 1;
			xNew = 0;
		}
	}

	if (yNew < 0)
	{
		yNew = pGent->m_y;
		if ((fdir & FDIR_D) && pMaze->m_yScr < kDYMaze-1)
		{
			dYScreen = 1;
			yNew = pRoom->m_dY - 1;
		}
	}
	else if (yNew >= pRoom->m_dY)
	{
		yNew = pGent->m_y;
		if ((fdir & FDIR_U) && pMaze->m_yScr > 0)
		{
			dYScreen = -1;
			yNew = 0;
		}
	}

	GameRoom * pGroomPrev = pMaze->m_pGroomCur;
	GameRoom * pGroomNext  = pGroomPrev;
	if (dXScreen != 0 || dYScreen != 0)
	{
		int xyRoomNext = (pMaze->m_xScr + dXScreen) + (pMaze->m_yScr + dYScreen) * kDXMaze;
		ROOMID roomidNext = pMaze->m_mpXyRoomid[xyRoomNext];
		pGroomNext  = &pMaze->m_aGroom[roomidNext];
	}

	if (FTryMoveEntity(pMaze, pGent, xNew, yNew, pGroomPrev, pGroomNext))
	{
		GameRoom * pGroomCur = pMaze->m_pGroomCur;
		pGent->m_x = xNew;
		pGent->m_y = yNew;

		if (dXScreen != 0 || dYScreen != 0)
		{
			// remove the player from the screen we're leaving
			//GameEntity * pGentAvatar = pMaze->m_pGentAvatar;
			FrEntity * pEntAvatar = Frog_PEnt(&pMaze->m_tworld, pGent->m_entid);

			int xRoomNew = pMaze->m_xScr + dXScreen;
			int yRoomNew = pMaze->m_yScr + dYScreen;
			int xyRoom = xRoomNew + yRoomNew * kDXMaze;

			Frog_RemoveFromRoom(pGroomCur->m_pRoom, pEntAvatar);
			Frog_AddToRoom(pGroomNext->m_pRoom, pEntAvatar, EUPO_Avatar);


			Frog_PostNote(&pMaze->m_noteq, NOTEK_LowPriority, "Entered room (%d, %d)", xRoomNew, yRoomNew);	
			TranslateCurScreen(
				pMaze, 
				pMaze->m_xScr + dXScreen, 
				pMaze->m_yScr + dYScreen, 
				Frog_Vec2Create(-dXScreen * dYPixelScr, dYScreen * dYPixelScr));
		}
	}
}

static void OnEnterRoom(EntityMaze * pMaze, GameRoom * pGroom)
{
}

static void OnExitRoom(EntityMaze * pMaze, GameRoom * pGroom)
{
}

static GameEntity * PGentAllocate(EntityMaze * pMaze, ENTK entk, int x, int y, char iTile)
{
	ENTID entid = Frog_EntidAllocate(&pMaze->m_tworld);
	GameEntity * pGent = &pMaze->m_mpEntidEnt[entid];

	pGent->m_entid = entid;
	pGent->m_entk = entk;
	pGent->m_iTile = iTile;

	pGent->m_x = x;
	pGent->m_y = y;

	return pGent;
}

static void FreeEntity(EntityMaze * pMaze, GameEntity * pGent)
{
	if (pGent->m_entid != ENTID_Nil)
	{
		Frog_FreeEntity(&pMaze->m_tworld, pGent->m_entid);
		pGent->m_entid = ENTID_Nil;
	}
}

void InitEntityMaze(EntityMaze * pMaze)
{
	int cBXyMap = sizeof(ROOMID) * kDXMaze * kDYMaze;
	pMaze->m_mpXyRoomid = (ROOMID *)malloc(cBXyMap);

	for (int iRoom = 0; iRoom < kDXMaze * kDYMaze; ++iRoom)
	{
		pMaze->m_mpXyRoomid[iRoom] = ROOMID_Nil;
	}

	ZeroAB(pMaze->m_aGroom, sizeof(pMaze->m_aGroom)); 
	ZeroAB(pMaze->m_aCIik, sizeof(pMaze->m_aCIik));

	Frog_InitNoteQueue(&pMaze->m_noteq, 16);
	Frog_InitTileWorld(&pMaze->m_tworld);

	FrColor colWallFg = Frog_ColCreate(0xFFa5c9c3);
	FrColor colWallBg = Frog_ColCreate(0xFF85aaa4);
	FrColor colGrassFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colGrassBg = Frog_ColCreate(0xFF156f3a); //#3a6f16
	FrColor colItemFg = Frog_ColCreate(0xFFa4d8d1); //#d1d8a4
	FrColor colItemBg = Frog_ColCreate(0xFF383e26); //#383e26
	FrColor colPathFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colPathBg = Frog_ColCreate(0xFF52afbf); //#bfaf52
	FrColor colMonstFg = Frog_ColCreate(0xFFa4d8FF);

	FrTileMap * pTmap = &pMaze->m_tworld.m_tmap;
	//Frog_SetTile(&pMaze->m_tmap, 'W', L'▓', colWallFg, colWallBg);
	Frog_SetTile(pTmap, 'W', 'W', colWallFg, colWallBg, -1, FTILE_Collide);
	Frog_SetTile(pTmap, 'S', 'S', colWallFg, colWallBg, -1, FTILE_Collide);
	Frog_SetTile(pTmap, 'E', 'E', colWallFg, colWallBg, -1, FTILE_Collide);
	Frog_SetTile(pTmap, '.', ' ', colGrassFg, colGrassBg, -1, FTILE_None);
	Frog_SetTile(pTmap, '_', ' ', colPathFg, colPathBg, -1, FTILE_None);
	Frog_SetTile(pTmap, 'C', 'o', colItemFg, colItemBg, -1, FTILE_None);
	Frog_SetTile(pTmap, '#', '#', colItemFg, colItemBg, -1, FTILE_None);	// BB - should be using collid
	Frog_SetTile(pTmap, 'K', 'K', colItemFg, colItemBg, -1, FTILE_None);
	Frog_SetTile(pTmap, '@', '@', colItemFg, colItemBg, -1, FTILE_None);
	Frog_SetTile(pTmap, '*', '*', colMonstFg, colItemBg, -1, FTILE_Collide);

	pMaze->m_xScr = s_xStart;
	pMaze->m_yScr = s_yStart;

	FrTileWorld * pTworld = &pMaze->m_tworld;
	for (int yScr = 0; yScr < kDYMaze; ++yScr)
	{
		for (int xScr = 0; xScr < kDYMaze; ++xScr)
		{
			FrRoom * pRoom = PRoomCreate(pMaze, xScr, yScr);
			
			int xyRoom = xScr + yScr * kDXMaze;
			pMaze->m_mpXyRoomid[xyRoom] = (ROOMID)pRoom->m_roomid;

			GameRoom * pGroom = &pMaze->m_aGroom[pRoom->m_roomid];
			pGroom->m_pRoom = pRoom;
			pGroom->m_cEntid = 0;

			GameEntity * pGentCoin = PGentAllocate(pMaze, ENTK_Coin, 7, 5, 'C');
			Frog_AddToRoom(pRoom, Frog_PEnt(pTworld, pGentCoin->m_entid), EUPO_Item);

			GameEntity * pGentMonst = PGentAllocate(pMaze, ENTK_Monster, 2, 2, '*');
			Frog_AddToRoom(pRoom, Frog_PEnt(pTworld, pGentMonst->m_entid), EUPO_Monster);

			if ((xScr & 0x1) == 0)
			{
				pGentCoin = PGentAllocate(pMaze, ENTK_Coin, 9, 5, 'C');
				Frog_AddToRoom(pRoom, Frog_PEnt(pTworld, pGentCoin->m_entid), EUPO_Item);
			}

			if (xScr == s_xScrLock && yScr == s_yScrLock)
			{
				GameEntity * pGentLock = PGentAllocate(pMaze, ENTK_Lock, 5, 4, '#');
				Frog_AddToRoom(pRoom, Frog_PEnt(pTworld, pGentLock->m_entid), EUPO_Item);
			}

			if (xScr == s_xScrKey && yScr == s_yScrKey)
			{
				GameEntity * pGentKey = PGentAllocate(pMaze, ENTK_Key, 5, 5, 'K');
				Frog_AddToRoom(pRoom, Frog_PEnt(pTworld, pGentKey->m_entid), EUPO_Item);
			}
		}
	}

	pMaze->m_pGroomCur = NULL;
	SetCurRoom(pMaze, pMaze->m_xScr, pMaze->m_yScr);

	pMaze->m_pGentAvatar = PGentAllocate(pMaze, ENTK_Avatar, 3, 3, '@');
	Frog_AddToRoom(pMaze->m_pGroomCur->m_pRoom, Frog_PEnt(pTworld, pMaze->m_pGentAvatar->m_entid), EUPO_Avatar);
}

static void UpdateInput(EntityMaze * pMaze, FrInput * pInput)
{
	Frog_PollInput(pInput);

	FrInputEventIterator inevit = Frog_Inevit(pInput->m_pInevfifo);
	FrInputEvent * pInev;
	while (pInev = Frog_PInevNext(&inevit))
	{
		if (pInev->m_edges != EDGES_Press)
			continue;

		switch (pInev->m_keycode)
		{
		case KEYCODE_ArrowUp:		MoveAvatar(pMaze, pMaze->m_pGentAvatar, 0, 1);	break;
		case KEYCODE_ArrowDown:		MoveAvatar(pMaze, pMaze->m_pGentAvatar, 0, -1);	break;
		case KEYCODE_ArrowLeft:		MoveAvatar(pMaze, pMaze->m_pGentAvatar, -1, 0);	break;
		case KEYCODE_ArrowRight:	MoveAvatar(pMaze, pMaze->m_pGentAvatar, 1, 0);	break;
		}
	}
	Frog_ClearInputEvents(pInput->m_pInevfifo);
}

void UpdateRoomEntities(EntityMaze * pMaze, FrRoom * pRoom, float dT)
{
	static float s_tTotal = 0.0f;
	float tPrev = s_tTotal;
	s_tTotal += dT;

	static const float s_dTMove = 0.8f;
	bool fShouldMove = (int)(tPrev / s_dTMove) != (int)(s_tTotal / s_dTMove);

	FrEntity ** ppEntMax = &pRoom->m_apEnt[pRoom->m_cpEnt];
	for (FrEntity ** ppEnt = pRoom->m_apEnt; ppEnt != ppEntMax; ++ppEnt)
	{
		FrEntity * pEnt = *ppEnt;
		if (!pEnt)
			continue;

		ENTID entid = Frog_EntidFromEnt(&pMaze->m_tworld, pEnt);
		GameEntity * pGent = &pMaze->m_mpEntidEnt[entid];

		switch (pGent->m_entk)
		{
		case ENTK_Monster:
			{
				if (fShouldMove)
				{
					GameEntity * pGentAvatar = pMaze->m_pGentAvatar;
					int dX = pGentAvatar->m_x - pGent->m_x;
					int dY = pGentAvatar->m_y - pGent->m_y;

					if (abs(dX) > abs(dY))
					{
						// move x towards avatar
						int moveX = 1;
						if (dX < 0)
							moveX = -1;
						MoveAvatar(pMaze, pGent, moveX, 0);
					}
					else if (dY != 0)
					{
						int moveY = 1;
						if (dY < 0)
							moveY = -1;
						MoveAvatar(pMaze, pGent, 0, moveY);
					}
				}
			} break;
		}

		Frog_UpdateEntity(&pMaze->m_tworld, pRoom, entid, pGent->m_x, pGent->m_y, pGent->m_iTile);
	}
}

void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT)
{
	FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	FrVec2 posText = Frog_Vec2Create(10.0f, 300.0f);
	FrVec2 posNoteQueue = Frog_Vec2Create(10.0f, 250.0f);

	char aCh[32];
	sprintf_s(aCh, FR_DIM(aCh), "$%d, %d keys", pMaze->m_aCIik[IIK_Coin], pMaze->m_aCIik[IIK_Key]);
	Frog_DrawTextRaw(pDrac, posText, aCh);

	Frog_RenderNoteQueue(pDrac, &pMaze->m_noteq, posNoteQueue, dT);

	FrRoomTransition * pRoomt = &pMaze->m_roomt;
	GameRoom * pGroomPrev = NULL;
	if (pRoomt->m_roomtk != ROOMTK_None && pRoomt->m_pRoomPrev)
	{
		pGroomPrev = &pMaze->m_aGroom[pRoomt->m_pRoomPrev->m_roomid];
	}

	Frog_UpdateTransition(&pMaze->m_roomt, dT);
	if (pRoomt->m_r >= 1.0f)
	{
		if (pGroomPrev)
		{
			OnExitRoom(pMaze, pGroomPrev);
		}
		Frog_SetTransition(pRoomt, ROOMTK_None, NULL, pRoomt->m_pRoom);
	}

	Frog_RenderTransition(pDrac, &pMaze->m_tworld, pRoomt, s_posScreen);

	UpdateInput(pMaze, pInput);

	if (pMaze->m_roomt.m_pRoomPrev)
	{
		FrRoom * pRoomPrev = pMaze->m_roomt.m_pRoomPrev;
		Frog_SortEntityUpdateList(pRoomPrev);

		UpdateRoomEntities(pMaze, pRoomPrev, dT);
	}

	GameRoom * pGroomCur = pMaze->m_pGroomCur;
	Frog_SortEntityUpdateList(pGroomCur->m_pRoom);

	UpdateRoomEntities(pMaze, pGroomCur->m_pRoom, dT);
}
