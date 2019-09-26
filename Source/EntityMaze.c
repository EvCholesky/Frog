#include "EntityMaze.h"
#include "FrogInput.h"

#include <stdio.h>
#include <stdlib.h>

void OnEnterRoom(EntityMaze * pMaze, Room * pRoom);
void OnExitRoom(EntityMaze * pMaze, Room * pRoom);
void AddEntityUpdate(EntityMaze * pMaze, Entity * pEnt, EUPO eupo);
void RemoveEntityUpdate(EntityMaze * pMaze, Entity * pEnt);

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
static const int s_xScrLock = 3;
static const int s_yScrLock = 4;

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

static FrScreen * PScreenCreate(EntityMaze * pMaze, int x, int y)
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

	FrScreen * pScr = Frog_AllocateScreen(&pMaze->m_tworld, kDXTile, kDYTile);
	Frog_MapScreen(pScr, &pMaze->m_tworld.m_tmap, aCh);
	return pScr;
}

static void SetCurRoom(EntityMaze * pMaze, int xScrNew, int yScrNew)
{
	int iRoom = xScrNew + yScrNew * kDXMaze;
	Room * pRoom  = &pMaze->m_aRoom[iRoom];

	pMaze->m_pRoomCur = pRoom;
	pMaze->m_xScr = xScrNew;
	pMaze->m_yScr = yScrNew;

	Frog_SetTransition(&pMaze->m_scrtr, SCRTRK_None, NULL, pRoom->m_pScr);

	OnEnterRoom(pMaze, pRoom);
}

static void TranslateCurScreen(EntityMaze * pMaze, int xScrNew, int yScrNew, FrVec2 dPosTransit)
{
	Room * pRoomPrev = pMaze->m_pRoomCur;

	int iRoomNew = xScrNew + yScrNew * kDXMaze;
	Room * pRoomNew  = &pMaze->m_aRoom[iRoomNew];

	pMaze->m_pRoomCur = pRoomNew;
	pMaze->m_xScr = xScrNew;
	pMaze->m_yScr = yScrNew;
	OnEnterRoom(pMaze, pRoomNew);

	if (!pRoomPrev)
	{
		Frog_SetTransition(&pMaze->m_scrtr, SCRTRK_None, NULL, pRoomNew->m_pScr);
	}
	else
	{
		Frog_SetTransition(&pMaze->m_scrtr, SCRTRK_Translate, pRoomPrev->m_pScr, pRoomNew->m_pScr);
		pMaze->m_scrtr.m_dPos = dPosTransit;
	}
}

bool FIsCellOpen(EntityMaze * pMaze, int xCellAvatar, int yCellAvatar, int dXScreen, int dYScreen)
{
	if (dXScreen != 0 || dYScreen != 0)
	{
		// BB - Check other rooms
		return true;
	}

	FrScreen * pScr = pMaze->m_pRoomCur->m_pScr;
	FTILE ftile = Frog_FtileFromCell(pScr, xCellAvatar, yCellAvatar);
	return (ftile & FTILE_Collide) == 0;
}

static void MoveAvatar(EntityMaze * pMaze, int dX, int dY)
{
	Room * pRoom = pMaze->m_pRoomCur;
	FrScreen * pScr = pRoom->m_pScr;
	Entity * pEnt = pMaze->m_pEntAvatar;

	int xNew = pEnt->m_x + dX;
	int yNew = pEnt->m_y + dY;
	int dXScreen = 0;
	int dYScreen = 0;

	f32 dXPixelScr = (f32)pScr->m_dX * pScr->m_dXCharPixel;
	f32 dYPixelScr = (f32)pScr->m_dY * pScr->m_dYCharPixel;
	FDIR fdir = FdirFromXyScreen(pMaze->m_xScr, pMaze->m_yScr);
	if (xNew < 0)
	{
		xNew = pEnt->m_x;
		if ((fdir & FDIR_L) && pMaze->m_xScr > 0)
		{
			dXScreen = -1;
			xNew = pScr->m_dX - 1;
		}
	}
	else if (xNew >= pScr->m_dX)
	{
		xNew = pEnt->m_x;
		if ((fdir & FDIR_R) && pMaze->m_xScr < kDXMaze-1)
		{
			dXScreen = 1;
			xNew = 0;
		}
	}

	if (yNew < 0)
	{
		yNew = pEnt->m_y;
		if ((fdir & FDIR_D) && pMaze->m_yScr < kDYMaze-1)
		{
			dYScreen = 1;
			yNew = pScr->m_dY - 1;
		}
	}
	else if (yNew >= pScr->m_dY)
	{
		yNew = pEnt->m_y;
		if ((fdir & FDIR_U) && pMaze->m_yScr > 0)
		{
			dYScreen = -1;
			yNew = 0;
		}
	}

	if (FIsCellOpen(pMaze, xNew, yNew, dXScreen, dYScreen))
	{
		Room * pRoomCur = pMaze->m_pRoomCur;
		pEnt->m_x = xNew;
		pEnt->m_y = yNew;

		if (dXScreen != 0 || dYScreen != 0)
		{
			// remove the player from the screen we're leaving
			Entity * pEntAvatar = pMaze->m_pEntAvatar;
			Frog_RemoveEntity(&pMaze->m_tworld, pRoomCur->m_pScr, pEntAvatar->m_tentid);

			TranslateCurScreen(
				pMaze, 
				pMaze->m_xScr + dXScreen, 
				pMaze->m_yScr + dYScreen, 
				Frog_Vec2Create(-dXScreen * dYPixelScr, dYScreen * dYPixelScr));
		}
	}
}

void AddEntityToRoom(Room * pRoom, Entity * pEnt)
{
	if (!FR_FVERIFY(pEnt->m_iRoom < 0, "Entity already in room") || 
		!FR_FVERIFY(pRoom->m_cTentid < FR_DIM(pRoom->m_aTentid), "Too many entities in room"))
		return;

	pRoom->m_aTentid[pRoom->m_cTentid++] = pEnt->m_tentid;
	pEnt->m_iRoom = pRoom->m_iRoom;
}

void RemoveEntityFromRoom(Room * pRoom, Entity * pEnt)
{
	if (!FR_FVERIFY(pEnt->m_iRoom != pRoom->m_iRoom, "Trying to remove entity from wrong room"))
		return;

	Tentid tentidEnt = pEnt->m_tentid;
	for (int iTentid = 0; iTentid < pRoom->m_cTentid; ++iTentid)
	{
		if (pRoom->m_aTentid[iTentid] == tentidEnt)
		{
			pEnt->m_iRoom = -1;
			--pRoom->m_cTentid;
			pRoom->m_aTentid[iTentid] = pRoom->m_aTentid[pRoom->m_cTentid];
			return;
		}
	}
}

void OnEnterRoom(EntityMaze * pMaze, Room * pRoom)
{
	// called when a room begins to transition in
	for (int iTentid = 0; iTentid < pRoom->m_cTentid; ++iTentid)
	{
		int tentid = pRoom->m_aTentid[iTentid];
		Entity * pEnt = &pMaze->m_mpTentidEnt[tentid];

		AddEntityUpdate(pMaze, pEnt, EUPO_Items);
	}
}

void OnExitRoom(EntityMaze * pMaze, Room * pRoom)
{
	// called when a room completes a transition out
	for (int iTentid = 0; iTentid < pRoom->m_cTentid; ++iTentid)
	{
		Tentid tentid = pRoom->m_aTentid[iTentid];
		Entity * pEnt = &pMaze->m_mpTentidEnt[tentid];

		RemoveEntityUpdate(pMaze, pEnt);
	}
}

Entity * PEntAllocate(EntityMaze * pMaze, ENTK entk, int x, int y, char iTile)
{
	Tentid tentid = Frog_TentidAllocate(&pMaze->m_tworld);
	Entity * pEnt = &pMaze->m_mpTentidEnt[tentid];

	pEnt->m_tentid = tentid;
	pEnt->m_entk = entk;
	pEnt->m_eupo = EUPO_Nil;
	pEnt->m_ipEntUpdate = -1;
	pEnt->m_iRoom = -1;
	pEnt->m_iTile = iTile;

	pEnt->m_x = x;
	pEnt->m_y = y;

	return pEnt;
}

void FreeEntity(EntityMaze * pMaze, Entity * pEnt)
{
	if (pEnt->m_tentid != kTentidNil)
	{
		Frog_FreeEntity(&pMaze->m_tworld, pEnt->m_tentid);
		pEnt->m_tentid = kTentidNil;
	}
}

void SortEntityUpdateList(EntityMaze * pMaze)
{
	if (pMaze->m_fUpdateIsDirty)
		return;

	pMaze->m_fUpdateIsDirty = false;

	// radix sort our entity updates
	int mpEupoCEnt[EUPO_Max] = {0};
	Entity * apEntWork[kCTentWorldMax];

	int cEntRemoved = 0;
	Entity ** ppEntMax = &pMaze->m_apEntUpdate[pMaze->m_cpEntUpdate];
	Entity ** ppEntWork = apEntWork;
	for (Entity** ppEnt = pMaze->m_apEntUpdate; ppEnt != ppEntMax; ++ppEnt)
	{
		if (*ppEnt == NULL)
		{
			++cEntRemoved;
			continue;
		}

		*ppEntWork++ = *ppEnt;
		Entity * pEnt = *ppEnt;
		++mpEupoCEnt[pEnt->m_eupo];
	}

	int mpEupoITentidMin[EUPO_Max];
	int iTentid = 0;
	for (int eupo = 0; eupo < EUPO_Max; ++eupo)
	{
		mpEupoITentidMin[eupo] = iTentid;
		iTentid += mpEupoCEnt[eupo];
	}
	
	pMaze->m_cpEntUpdate -= cEntRemoved;
	FR_ASSERT(pMaze->m_cpEntUpdate == iTentid, "bad entity array calculations");

	Entity ** ppEntWorkMax = &apEntWork[pMaze->m_cpEntUpdate];
	for (Entity ** ppEntWork = apEntWork; ppEntWork != ppEntWorkMax; ++ppEntWork)
	{
		Entity * pEnt = *ppEntWork;
		int iTentid = mpEupoITentidMin[pEnt->m_eupo]++;
		
		pEnt->m_ipEntUpdate = iTentid;
		pMaze->m_apEntUpdate[iTentid] = pEnt;
	}
}

void AddEntityUpdate(EntityMaze * pMaze, Entity * pEnt, EUPO eupo)
{
	if (!FR_FVERIFY(pMaze->m_cpEntUpdate < FR_DIM(pMaze->m_apEntUpdate), "too many active entities"))	
		return;

	if (!FR_FVERIFY(pEnt->m_eupo == EUPO_Nil && pEnt->m_ipEntUpdate < 0, "Entity is already registered for update"))
		return;

	pEnt->m_eupo = eupo;
	pEnt->m_ipEntUpdate = pMaze->m_cpEntUpdate++;
	pMaze->m_apEntUpdate[pEnt->m_ipEntUpdate] = pEnt;
	pMaze->m_fUpdateIsDirty = true;
}

void RemoveEntityUpdate(EntityMaze * pMaze, Entity * pEnt)
{
	if (!FR_FVERIFY(pEnt->m_eupo != EUPO_Nil && pEnt->m_ipEntUpdate >= 0, "Entity is not registered for update"))
		return;

	pMaze->m_apEntUpdate[pEnt->m_ipEntUpdate] = NULL;
	pEnt->m_ipEntUpdate = -1;
	pEnt->m_eupo = EUPO_Nil;
	pMaze->m_fUpdateIsDirty = true;
}

void InitEntityMaze(EntityMaze * pMaze)
{
	int cBRoomMap = sizeof(Room) * kDXMaze * kDYMaze;
	pMaze->m_aRoom = (Room *)malloc(cBRoomMap);
	ZeroAB(pMaze->m_aRoom, cBRoomMap); 

	Frog_InitTileWorld(&pMaze->m_tworld);

	FrColor colWallFg = Frog_ColCreate(0xFFa5c9c3);
	FrColor colWallBg = Frog_ColCreate(0xFF85aaa4);
	FrColor colGrassFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colGrassBg = Frog_ColCreate(0xFF156f3a); //#3a6f16
	FrColor colItemFg = Frog_ColCreate(0xFFa4d8d1); //#d1d8a4
	FrColor colItemBg = Frog_ColCreate(0xFF383e26); //#383e26
	FrColor colPathFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colPathBg = Frog_ColCreate(0xFF52afbf); //#bfaf52

	FrTileMap * pTmap = &pMaze->m_tworld.m_tmap;
	//Frog_SetTile(&pMaze->m_tmap, 'W', L'▓', colWallFg, colWallBg);
	Frog_SetTile(pTmap, 'W', 'W', colWallFg, colWallBg, FTILE_Collide);
	Frog_SetTile(pTmap, 'S', 'S', colWallFg, colWallBg, FTILE_Collide);
	Frog_SetTile(pTmap, 'E', 'E', colWallFg, colWallBg, FTILE_Collide);
	Frog_SetTile(pTmap, '.', ' ', colGrassFg, colGrassBg, FTILE_None);
	Frog_SetTile(pTmap, '_', ' ', colPathFg, colPathBg, FTILE_None);
	Frog_SetTile(pTmap, 'C', 'o', colItemFg, colItemBg, FTILE_None);
	Frog_SetTile(pTmap, '@', '@', colItemFg, colItemBg, FTILE_None);

	pMaze->m_xScr = s_xStart;
	pMaze->m_yScr = s_yStart;
	pMaze->m_cpEntUpdate = 0;
	pMaze->m_fUpdateIsDirty = false;

	for (int yScr = 0; yScr < kDYMaze; ++yScr)
	{
		for (int xScr = 0; xScr < kDYMaze; ++xScr)
		{
			int iRoom = xScr + yScr * kDXMaze;
			Room * pRoom = &pMaze->m_aRoom[iRoom];
			pRoom->m_pScr = PScreenCreate(pMaze, xScr, yScr);
			pRoom->m_pScr->m_nId = iRoom;
			pRoom->m_cTentid = 0;
			pRoom->m_iRoom = iRoom;

			Entity * pEntCoin = PEntAllocate(pMaze, ENTK_Avatar, 7, 5, 'C');
			AddEntityToRoom(pRoom, pEntCoin);
		}
	}

	pMaze->m_pRoomCur = NULL;
	SetCurRoom(pMaze, pMaze->m_xScr, pMaze->m_yScr);

	pMaze->m_pEntAvatar = PEntAllocate(pMaze, ENTK_Avatar, 3, 3, '@');
	AddEntityUpdate(pMaze, pMaze->m_pEntAvatar, EUPO_Avatar);
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
		case KEYCODE_ArrowUp:		MoveAvatar(pMaze, 0, 1);	break;
		case KEYCODE_ArrowDown:		MoveAvatar(pMaze, 0, -1);	break;
		case KEYCODE_ArrowLeft:		MoveAvatar(pMaze, -1, 0);	break;
		case KEYCODE_ArrowRight:	MoveAvatar(pMaze, 1, 0);	break;
		}
	}
	Frog_ClearInputEvents(pInput->m_pInevfifo);
}

void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT)
{
	FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	FrVec2 posText = Frog_Vec2Create(20.0f, 20.0f);

	char aCh[32];
	sprintf_s(aCh, FR_DIM(aCh), "(%d, %d)", pMaze->m_xScr, pMaze->m_yScr);
	Frog_DrawTextRaw(pDrac, posText, aCh);

	FrScreenTransition * pScrtr = &pMaze->m_scrtr;
	Room * pRoomPrev = NULL;
	if (pScrtr->m_scrtrk != SCRTRK_None && pScrtr->m_pScrPrev)
	{
		pRoomPrev = &pMaze->m_aRoom[pScrtr->m_pScrPrev->m_nId];
	}

	Frog_UpdateTransition(&pMaze->m_scrtr, dT);
	if (pScrtr->m_r >= 1.0f)
	{
		if (pRoomPrev)
		{
			OnExitRoom(pMaze, pRoomPrev);
		}
		Frog_SetTransition(pScrtr, SCRTRK_None, NULL, pScrtr->m_pScr);
	}

	Frog_RenderTransition(pDrac, &pMaze->m_tworld, pScrtr, s_posScreen);

	UpdateInput(pMaze, pInput);

	SortEntityUpdateList(pMaze);

	Room * pRoomCur = pMaze->m_pRoomCur;
	Entity ** ppEntMax = &pMaze->m_apEntUpdate[pMaze->m_cpEntUpdate];
	for (Entity** ppEnt = pMaze->m_apEntUpdate; ppEnt != ppEntMax; ++ppEnt)
	{
		Entity * pEnt = *ppEnt;
		if (!pEnt)
			continue;

		Frog_UpdateEntity(&pMaze->m_tworld, pRoomCur->m_pScr, pEnt->m_tentid, pEnt->m_x, pEnt->m_y, pEnt->m_iTile);
	}
}
