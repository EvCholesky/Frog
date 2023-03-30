#include "WorldMaze.h"

#include "Common.h"
#include "FrogInput.h"
#include "FrogRender.h"
#include "FrogString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct DumpContext_t // tag = dctx
{
	FILE *				m_pFile;
	FrEditBuffer		m_freb;
	int					m_cTabIndent;

} DumpContext;

void DumpInt(DumpContext * pDctx, int n, const char * pChzComment);
void DumpRoomDefinition(DumpContext * pDctx, const RoomDefinition * pRmdef, char * aChScratch, int cChScratch);
void DumpTileDefinition(DumpContext * pDctx, const TileDefinition * pTiledef);
void DumpRoomLibrary(DumpContext * pDctx, const RoomLibrary * pRmlib);

#include "worldDef.inl"

static int s_cChComment = 60;
static int s_cColTab = 4;

const char * PChzFromTransk(TRANSK transk)
{
	static const char * s_mpTranskPChz[] =
	{
		"DirL",				// TRANSK_DirL,
		"DirU",				// TRANSK_DirU,
		"DirR",				// TRANSK_DirR,
		"DirD",				// TRANSK_DirD,
		"Door1",			// TRANSK_Door1,
		"Door2",			// TRANSK_Door2,
		"Hole",				// TRANSK_Hole,
		"ReverseWorld",		//TRANSK_ReverseWorld,
	};
	FR_CASSERT(FR_DIM(s_mpTranskPChz) == TRANSK_Max, "missing TRANSK string");
	if (transk == TRANSK_Nil)
		return "Nil";

	if ((transk < TRANSK_Nil) | (transk >= TRANSK_Max))
		return "Unknown";

	return s_mpTranskPChz[transk];
}
	  
void PadToColumn(FrEditBuffer * pFreb, char ch, int cColumn)
{
	int iCol = 0;
	for (const char * pCh = pFreb->m_pChzMin; pCh != pFreb->m_pChzAppend; ++pCh)
	{
		++iCol;
		int iColMod = iCol % s_cColTab;
		if (*pCh == '\t')
		{
			iCol += (s_cColTab - iColMod) % s_cColTab;
		}
	}

	if (cColumn > iCol)
	{
		// round up to next tab

		int iColTabNext = ((iCol + 4) / s_cColTab) * s_cColTab;
		if (cColumn >= iColTabNext)
		{
			Freb_AppendCh(pFreb, "\t", 2);
			iCol = iColTabNext;
		}

		while (cColumn >= iCol + s_cColTab)
		{
			Freb_AppendCh(pFreb, "\t", 2);
			iCol += s_cColTab;
		}

		while (cColumn > iCol)
		{
			Freb_AppendCh(pFreb, " ", 2);
			++iCol;
		}
	}
}

void DumpLine(DumpContext * pDctx, const char * pChzLine)
{
	PadToColumn(&pDctx->m_freb, ' ', pDctx->m_cTabIndent * s_cColTab);
	Freb_Printf(&pDctx->m_freb, "%s", pChzLine);

	fprintf(pDctx->m_pFile, "%s\n", pDctx->m_freb.m_pChzMin);
	Freb_Clear(&pDctx->m_freb);
}
void DumpInt(DumpContext * pDctx, int n, const char * pChzComment)
{
	PadToColumn(&pDctx->m_freb, ' ', pDctx->m_cTabIndent * s_cColTab);

	Freb_Printf(&pDctx->m_freb, "%d,", n);
	PadToColumn(&pDctx->m_freb, ' ', s_cChComment);
	Freb_Printf(&pDctx->m_freb, "// %s", pChzComment);
	fprintf(pDctx->m_pFile, "%s\n", pDctx->m_freb.m_pChzMin);
	Freb_Clear(&pDctx->m_freb);
}

void DumpNamedValue(DumpContext * pDctx, const char * pChzValue, const char * pChzComment)
{
	PadToColumn(&pDctx->m_freb, ' ', pDctx->m_cTabIndent * s_cColTab);

	Freb_Printf(&pDctx->m_freb, "%s,", pChzValue);
	if (pChzComment && pChzComment[0] != '\0')
	{
		PadToColumn(&pDctx->m_freb, ' ', s_cChComment);
		Freb_Printf(&pDctx->m_freb, "// %s", pChzComment);
	}

	fprintf(pDctx->m_pFile, "%s\n", pDctx->m_freb.m_pChzMin);
	Freb_Clear(&pDctx->m_freb);
}

void DumpOpen(DumpContext * pDctx, const char * pChzOpen)
{
	DumpLine(pDctx, pChzOpen);
	++pDctx->m_cTabIndent;
}

void DumpClose(DumpContext * pDctx, const char * pChzClose)
{
	--pDctx->m_cTabIndent;
	DumpLine(pDctx, pChzClose);
}

void DumpRoomDefinition(DumpContext * pDctx, const RoomDefinition * pRmdef, char * aChScratch, int cChScratch)
{
	sprintf_s(aChScratch, cChScratch, "static char s_aiTileRm%s[] = ", pRmdef->m_pChzName);
	DumpLine(pDctx, aChScratch);
	DumpOpen(pDctx, "{");

	for (int y=0; y<pRmdef->m_dY; ++y)
	{
		PadToColumn(&pDctx->m_freb, ' ', pDctx->m_cTabIndent * s_cColTab);

		for (int x=0; x<pRmdef->m_dX; ++x)
		{
			Freb_Printf(&pDctx->m_freb, "%d, ", pRmdef->m_aiTile[x + y*pRmdef->m_dX]);
		}

		fprintf(pDctx->m_pFile, "%s\n", pDctx->m_freb.m_pChzMin);
		Freb_Clear(&pDctx->m_freb);
	}
	DumpClose(pDctx, "};\n");

	sprintf_s(aChScratch, cChScratch, "static RoomDefinition s_rmdef%s = ", pRmdef->m_pChzName);
	DumpLine(pDctx, aChScratch);
	DumpOpen(pDctx, "{");
	sprintf_s(aChScratch, cChScratch, "\"%s\"", pRmdef->m_pChzName);
	DumpNamedValue(pDctx, aChScratch, "m_pChzName");
	DumpInt(pDctx, pRmdef->m_dX, "m_dX");
	DumpInt(pDctx, pRmdef->m_dY, "m_dY");
	sprintf_s(aChScratch, cChScratch, "s_aiTileRm%s", pRmdef->m_pChzName);
	DumpNamedValue(pDctx, aChScratch, "m_aiTile");
	fprintf(pDctx->m_pFile, "\n");

	DumpOpen(pDctx, "{");
	for (int iRmtrans = 0; iRmtrans < FR_DIM(pRmdef->m_aRmtrans); ++iRmtrans)
	{
		const RoomTransition * pRmtrans = &pRmdef->m_aRmtrans[iRmtrans];
		if (pRmtrans->m_transk == TRANSK_Nil)
			break;

		sprintf_s(aChScratch, cChScratch, "{ TRANSK_%s, \"%s\", %d, %d }", 
				PChzFromTransk(pRmtrans->m_transk), 
				pRmtrans->m_pChzDest, 
				pRmtrans->m_dX,
				pRmtrans->m_dY);
		DumpNamedValue(pDctx, aChScratch, "");
	}
	DumpNamedValue(pDctx, "{ TRANSK_Nil }", "");
	DumpClose(pDctx, "}");

	DumpClose(pDctx, "};\n");
}

void DumpTileDefinition(DumpContext * pDctx, const TileDefinition * pTiledef)
{
	PadToColumn(&pDctx->m_freb, ' ', pDctx->m_cTabIndent * s_cColTab);

	Freb_Printf(&pDctx->m_freb, "{ '%c', (s32)0x%8X, (s32)0x%8X, ", pTiledef->m_ch, pTiledef->m_colFg, pTiledef->m_colBg );

	if(pTiledef->m_grftile == FTILE_None)
	{
		Freb_AppendChz(&pDctx->m_freb, "FTILE_None");
	}
	else
	{
		const char * pChzFormat = "FTILE_%s";
		for (int ftile = 0x1; ftile && ftile < FTILE_All; ftile = ftile << 1)
		{
			if (ftile & pTiledef->m_grftile)
			{
				Freb_Printf(&pDctx->m_freb, pChzFormat, PChzFromFTile(ftile));
				pChzFormat = "|FTILE_%s";
			}
		}
	}
	Freb_Printf(&pDctx->m_freb, "},");

	fprintf(pDctx->m_pFile, "%s\n", pDctx->m_freb.m_pChzMin);
	Freb_Clear(&pDctx->m_freb);
}

void DumpRoomLibrary(DumpContext * pDctx, const RoomLibrary * pRmlib)
{
	char aChScratch[128];

	for (int ipRmdef = 0; pRmlib->m_apRmdef[ipRmdef]; ++ipRmdef)
	{
		DumpRoomDefinition(pDctx, pRmlib->m_apRmdef[ipRmdef], aChScratch, FR_DIM(aChScratch));
	}

	DumpLine(pDctx, "static RoomDefinition * s_apRmdef[] = ");
	DumpOpen(pDctx, "{");
	for (int ipRmdef = 0; pRmlib->m_apRmdef[ipRmdef]; ++ipRmdef)
	{
		RoomDefinition * pRmdef = pRmlib->m_apRmdef[ipRmdef];
		sprintf_s(aChScratch, FR_DIM(aChScratch), "&s_rmdef%s", pRmdef->m_pChzName);
		DumpNamedValue(pDctx, aChScratch, "");
	}
	DumpNamedValue(pDctx, "NULL", "");
	DumpClose(pDctx, "};\n");
	
	DumpLine(pDctx, "static TileDefinition s_aTiledef[] = ");
	DumpOpen(pDctx, "{");
	for (int iTiledef = 0; ; ++iTiledef)
	{
		TileDefinition * pTiledef = &pRmlib->m_aTiledef[iTiledef];
		if (pTiledef->m_ch == '\0')
			break;
		DumpTileDefinition(pDctx, pTiledef);
	}
	DumpNamedValue(pDctx, "{'\\0'}", "");
	DumpClose(pDctx, "};\n");

	DumpLine(pDctx, "RoomLibrary s_rmlib = ");
	DumpOpen(pDctx, "{");

	DumpNamedValue(pDctx, "s_apRmdef", "m_apRmdef");
	DumpNamedValue(pDctx, "s_aTiledef", "m_aTiledef");

	DumpClose(pDctx, "};\n");
}

bool FTryDumpRoomFile(const char * pChzFilename)
{
	FILE * pFile = NULL;
	int nErr = fopen_s(&pFile, pChzFilename, "w");
    if (nErr || !pFile)
		return false;

	DumpContext dctx;
	char aCh[1024];
	dctx.m_pFile = pFile;
	dctx.m_cTabIndent = 0;
	Freb_init(&dctx.m_freb, aCh, FR_PMAX(aCh));

	DumpRoomLibrary(&dctx, &s_rmlib);
  
    fclose(pFile);
	return true;
}

static void OnEnterRoom(World * pWorld, GameRoom * pGroom)
{
}

static void OnExitRoom(World * pWorld, GameRoom * pGroom)
{
}

static void SetCurRoom(World * pWorld, GameRoom * pGroom)
{
	pWorld->m_pGroomCur = pGroom;

	Frog_SetTransition(&pWorld->m_roomt, ROOMTK_None, NULL, pGroom->m_pRoomFrog);

	OnEnterRoom(pWorld, pGroom);
}

GameRoom * PGroomLookup(World * pWorld, const char * pChzName)
{
	for (int rmid = 0; rmid < pWorld->m_rmidMax; ++rmid)
	{
		GameRoom * pGroom = &pWorld->m_mpRmidGroom[rmid];
		if (!strcmp(pChzName, pGroom->m_pRmdef->m_pChzName))
		{
			return pGroom;	
		}
	}

	return NULL;
}

const RoomTransition * PRmtransLookup(RoomDefinition * pRmdef, TRANSK transk)
{
	for (int iRmtrans = 0; iRmtrans < FR_DIM(pRmdef->m_aRmtrans); ++iRmtrans)
	{
		const RoomTransition * pRmtrans = &pRmdef->m_aRmtrans[iRmtrans];
		if (pRmtrans->m_transk == TRANSK_Nil)
			break;
		
		if (pRmtrans->m_transk == transk)
			return pRmtrans;
	}

	return NULL;
}

static void TranslateCurScreen(World * pWorld, GameRoom * pGroomNew, FrVec2 dPosTransit)
{
	GameRoom * pGroomPrev = pWorld->m_pGroomCur;

	pWorld->m_pGroomCur = pGroomNew;
	OnEnterRoom(pWorld, pGroomNew);

	if (!pGroomPrev)
	{
		Frog_SetTransition(&pWorld->m_roomt, ROOMTK_None, NULL, pGroomNew->m_pRoomFrog);
	}
	else
	{
		Frog_SetTransition(&pWorld->m_roomt, ROOMTK_Translate, pGroomPrev->m_pRoomFrog, pGroomNew->m_pRoomFrog);
		pWorld->m_roomt.m_dPos = dPosTransit;
	}
}

void HandleBumpAvatar(World * pWorld, GameEntity * pGentSrc, ENTID entidDest, bool * pFAllowMove)
{
	GameEntity * pGentDest = &pWorld->m_mpEntidEnt[entidDest];

	switch (pGentDest->m_entk)
	{
	/*
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
		*/
	}
}

bool FCanMoveToCell(World * pWorld, int xCell, int yCell, GameRoom * pGroom)
{
	FrRoom * pRoomFrog = pGroom->m_pRoomFrog;
	if (xCell < 0 || xCell >= pRoomFrog->m_dX ||
		yCell < 0 || yCell >= pRoomFrog->m_dY)
	{
		return false;
	}

	ENTID aEntid[kCEntityCellMax];
	FrCellContents cellc;
	Frog_InitCellContents(aEntid, FR_DIM(aEntid), &cellc);
	Frog_FindCellContents(&pWorld->m_tworld, pGroom->m_pRoomFrog, xCell, yCell, &cellc);

	bool fAllowMove = (cellc.m_tile.m_ftile & FTILE_Collide) == 0;
	return fAllowMove;
}

bool FTryMoveEntity(World * pWorld, GameEntity * pGent, int xCellNew, int yCellNew, GameRoom * pGroomPrev, GameRoom * pGroomNext)
{
	ENTID aEntid[kCEntityCellMax];
	FrCellContents cellc;
	Frog_InitCellContents(aEntid, FR_DIM(aEntid), &cellc);
	Frog_FindCellContents(&pWorld->m_tworld, pGroomNext->m_pRoomFrog, xCellNew, yCellNew, &cellc);

	bool fAllowMove = (cellc.m_tile.m_ftile & FTILE_Collide) == 0;
	
	ENTID * pEntidMax = &cellc.m_aEntid[cellc.m_cEntid];
	for (ENTID * pEntidDest = cellc.m_aEntid; pEntidDest != pEntidMax; ++pEntidDest)
	{
		switch (pGent->m_entk)
		{
		case ENTK_Avatar:
			HandleBumpAvatar(pWorld, pGent, *pEntidDest, &fAllowMove);
			break;
		}
	}
	return fAllowMove;
}

void DyDyFromTransk(TRANSK transk, int * pdX, int * pdY)
{
	switch (transk)
	{
	case TRANSK_DirL:
		*pdX = -1;
		*pdY = 0;
		break;
	case TRANSK_DirR:
		*pdX = 1;
		*pdY = 0;
		break;
	case TRANSK_DirD:
		*pdX = 0;
		*pdY = 1;
		break;
	case TRANSK_DirU:
		*pdX = 0;
		*pdY = -1;
		break;
	default:
		*pdX = 0;
		*pdY = 0;
	}
}

void DestXyFromTransk(TRANSK transk, GameRoom * pGroom, int * pXDest, int *pYDest)
{
	switch (transk)
	{
	case TRANSK_DirL:
		*pXDest = pGroom->m_pRoomFrog->m_dX - 1;
		break;
	case TRANSK_DirR:
		*pXDest = 0;
		break;
	case TRANSK_DirD:
		*pYDest = pGroom->m_pRoomFrog->m_dY - 1;
		break;
	case TRANSK_DirU:
		*pYDest = 0;
		break;
	}
}

static void MoveAvatar(World * pWorld, GameEntity * pGent, int dX, int dY)
{
	GameRoom * pGroom = pWorld->m_pGroomCur;
	GameRoom * pGroomNext = pGroom;
	FrRoom * pRoom = pGroom->m_pRoomFrog;

	int xNew = pGent->m_x + dX;
	int yNew = pGent->m_y + dY;
	int dXScreen = 0;
	int dYScreen = 0;

	int xTrans = pGent->m_x + dX;
	int yTrans = pGent->m_y + dY;
	f32 dXPixelScr = (f32)pRoom->m_dX * pRoom->m_dXCharPixel;
	f32 dYPixelScr = (f32)pRoom->m_dY * pRoom->m_dYCharPixel;
	f32 dXPixelTrans = 0.0f;
	f32 dYPixelTrans = 0.0f;

	TRANSK transk = TRANSK_Nil;
	if (xNew < 0)
	{ 
		transk = TRANSK_DirL;
	}
	else if (xNew >= pRoom->m_dX)
	{ 
		transk = TRANSK_DirR;
	}
	else if (yNew < 0)
	{ 
		transk = TRANSK_DirD;
	}
	else if (yNew >= pRoom->m_dY)
	{ 
		transk = TRANSK_DirU;
	}

	const RoomTransition * pRmtrans = NULL;
	if (transk != TRANSK_Nil)
	{
		const RoomDefinition * pRmdef = pGroom->m_pRmdef;
		for (int iRmtrans = 0; iRmtrans < FR_DIM(pRmdef->m_aRmtrans); ++iRmtrans)
		{
			const RoomTransition * pRmtransIt = &pRmdef->m_aRmtrans[iRmtrans];
			if (transk == TRANSK_Nil)
				break;;
			if (transk != pRmtransIt->m_transk)
				continue;

			GameRoom * pGroomTrans = PGroomLookup(pWorld, pRmtransIt->m_pChzDest);
			if (!pGroomTrans)
				continue;

			int xCellAdj = pGent->m_x + dX;
			int yCellAdj = pGent->m_y + dY;
			DestXyFromTransk(transk, pGroomTrans, &xCellAdj, &yCellAdj);
			xCellAdj += pRmtransIt->m_dX;
			yCellAdj += pRmtransIt->m_dY;

			if (FCanMoveToCell(pWorld, xCellAdj, yCellAdj, pGroomTrans))
			{
				xTrans = xCellAdj;
				yTrans = yCellAdj;
				pRmtrans = pRmtransIt;
				pGroomNext = pGroomTrans;
				break;
			}
		}
	}

	if (transk != TRANSK_Nil)
	{
		if (!pRmtrans)	
		{
			xNew = pGent->m_x;
			yNew = pGent->m_y;
		}
		else
		{
			//DyDyFromTransk(transk, &dXScreen, &dYScreen);

			switch (transk)
			{
			case TRANSK_DirL:
				dXScreen = -1;
				dYScreen = 0;
				break;
			case TRANSK_DirR:
				dXScreen = 1;
				dYScreen = 0;
				break;
			case TRANSK_DirD:
				dXScreen = 0;
				dYScreen = 1;
				break;
			case TRANSK_DirU:
				dXScreen = 0;
				dYScreen = -1;
				break;
			}
			xNew = xTrans;
			yNew = yTrans;
			dYPixelTrans = pRmtrans->m_dX * pRoom->m_dXCharPixel;
			dYPixelTrans = pRmtrans->m_dY * pRoom->m_dYCharPixel;
		}
	}

#if 0
	if (xNew < 0)
	{
		xNew = pGent->m_x;
		const RoomTransition * pRmtrans = PRmtransLookup(pGroom->m_pRmdef, TRANSK_DirL);
		if (pRmtrans)
		{
			pGroomNext = PGroomLookup(pWorld, pRmtrans->m_pChzDest);
		}

		if (pGroomNext != pGroom)
		{
			dXScreen = -1;
			xNew = pGroomNext->m_pRoomFrog->m_dX - 1;
			yNew = pGent->m_y + pRmtrans->m_dY;
			dYPixelTrans = pRmtrans->m_dX * pRoom->m_dXCharPixel;
			dYPixelTrans = pRmtrans->m_dY * pRoom->m_dYCharPixel;
		}
	}
	else if (xNew >= pRoom->m_dX)
	{
		xNew = pGent->m_x;
		const RoomTransition * pRmtrans = PRmtransLookup(pGroom->m_pRmdef, TRANSK_DirR);
		if (pRmtrans)
		{
			pGroomNext = PGroomLookup(pWorld, pRmtrans->m_pChzDest);
		}

		if (pGroomNext != pGroom)
		{
			dXScreen = 1;
			xNew = 0;
			yNew = pGent->m_y + pRmtrans->m_dY;
			dYPixelTrans = pRmtrans->m_dX * pRoom->m_dXCharPixel;
			dYPixelTrans = pRmtrans->m_dY * pRoom->m_dYCharPixel;
		}
	}

	if (yNew < 0)
	{
		yNew = pGent->m_y;
		const RoomTransition * pRmtrans = PRmtransLookup(pGroom->m_pRmdef, TRANSK_DirD);
		if (pRmtrans)
		{
			pGroomNext = PGroomLookup(pWorld, pRmtrans->m_pChzDest);
		}

		if (pGroomNext != pGroom)
		{
			dYScreen = 1;
			yNew = pGroomNext->m_pRoomFrog->m_dY - 1;
			dYPixelTrans = pRmtrans->m_dX * pRoom->m_dXCharPixel;
			dYPixelTrans = pRmtrans->m_dY * pRoom->m_dYCharPixel;
		}
	}
	else if (yNew >= pRoom->m_dY)
	{
		yNew = pGent->m_y;
		const RoomTransition * pRmtrans = PRmtransLookup(pGroom->m_pRmdef, TRANSK_DirU);
		if (pRmtrans)
		{
			pGroomNext = PGroomLookup(pWorld, pRmtrans->m_pChzDest);
		}

		if (pGroomNext != pGroom)
		{
			dYScreen = -1;
			yNew = 0;
			dYPixelTrans = pRmtrans->m_dX * pRoom->m_dXCharPixel;
			dYPixelTrans = pRmtrans->m_dY * pRoom->m_dYCharPixel;
		}
	}
#endif

	if (FTryMoveEntity(pWorld, pGent, xNew, yNew, pGroom, pGroomNext))
	{
		pGent->m_x = xNew;
		pGent->m_y = yNew;

		if (dXScreen != 0 || dYScreen != 0)
		{
			// remove the player from the screen we're leaving
			FrEntity * pEntAvatar = Frog_PEnt(&pWorld->m_tworld, pGent->m_entid);

			Frog_RemoveFromRoom(pGroom->m_pRoomFrog, pEntAvatar);
			Frog_AddToRoom(pGroomNext->m_pRoomFrog, pEntAvatar, EUPO_Avatar);

			Frog_PostNote(&pWorld->m_noteq, NOTEK_LowPriority, "Entered room %s", pGroomNext->m_pRmdef->m_pChzName);	
			TranslateCurScreen(
				pWorld, 
				pGroomNext,
				Frog_Vec2Create(-dXScreen * dYPixelScr + dXPixelTrans, dYScreen * dYPixelScr + dYPixelTrans));
		}
	}
}

static GameEntity * PGentAllocate(World * pWorld, ENTK entk, int x, int y, char iTile)
{
	ENTID entid = Frog_EntidAllocate(&pWorld->m_tworld);
	GameEntity * pGent = &pWorld->m_mpEntidEnt[entid];

	pGent->m_entid = entid;
	pGent->m_entk = entk;
	pGent->m_iTile = iTile;

	pGent->m_x = x;
	pGent->m_y = y;

	return pGent;
}

static void FreeEntity(World * pWorld, GameEntity * pGent)
{
	if (pGent->m_entid != ENTID_Nil)
	{
		Frog_FreeEntity(&pWorld->m_tworld, pGent->m_entid);
		pGent->m_entid = ENTID_Nil;
	}
}


void UpdateRoomEntities(World * pWorld, FrRoom * pRoom, float dT)
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

		ENTID entid = Frog_EntidFromEnt(&pWorld->m_tworld, pEnt);
		GameEntity * pGent = &pWorld->m_mpEntidEnt[entid];

		switch (pGent->m_entk)
		{
		/*
		case ENTK_Monster:
			{
				if (fShouldMove)
				{
					GameEntity * pGentAvatar = pWorld->m_pGentAvatar;
					int dX = pGentAvatar->m_x - pGent->m_x;
					int dY = pGentAvatar->m_y - pGent->m_y;

					if (abs(dX) > abs(dY))
					{
						// move x towards avatar
						int moveX = 1;
						if (dX < 0)
							moveX = -1;
						MoveAvatar(pWorld, pGent, moveX, 0);
					}
					else if (dY != 0)
					{
						int moveY = 1;
						if (dY < 0)
							moveY = -1;
						MoveAvatar(pWorld, pGent, 0, moveY);
					}
				}
			} break;
			*/
		}

		Frog_UpdateEntity(&pWorld->m_tworld, pRoom, entid, pGent->m_x, pGent->m_y, pGent->m_iTile);
	}
}

void InitWorldMaze(World * pWorld)
{
	RoomLibrary * pRmlib = &s_rmlib;
/*
	int cBXyMap = sizeof(ROOMID) * kDXMaze * kDYMaze;
	pWorld->m_mpXyRoomid = (ROOMID *)malloc(cBXyMap);

	for (int iRoom = 0; iRoom < kDXMaze * kDYMaze; ++iRoom)
	{
		pWorld->m_mpXyRoomid[iRoom] = ROOMID_Nil;
	}
	*/

	//ZeroAB(pWorld->m_aCIik, sizeof(pWorld->m_aCIik));

	Frog_InitNoteQueue(&pWorld->m_noteq, 16);
	Frog_InitTileWorld(&pWorld->m_tworld);

	FrTileMap * pTmap = &pWorld->m_tworld.m_tmap;
	for (int iTiledef = 0; ; ++iTiledef)
	{
		TileDefinition * pTiledef = &pRmlib->m_aTiledef[iTiledef];
		if (pTiledef->m_ch == '\0')
			break;

		Frog_SetTile(pTmap, iTiledef, pTiledef->m_ch, pTiledef->m_colFg, pTiledef->m_colBg, pTiledef->m_grftile);
	}

	int cRmdef = 0;
	for ( ; s_rmlib.m_apRmdef[cRmdef]; ++cRmdef)
	{ ; }

	pWorld->m_mpRmidGroom = (GameRoom *)malloc(sizeof(GameRoom) * cRmdef);
	ZeroAB(pWorld->m_mpRmidGroom, sizeof(pWorld->m_mpRmidGroom)); 
	pWorld->m_rmidMax = (RMID)cRmdef;

	for (int iRmdef = 0; iRmdef < cRmdef; ++iRmdef)
	{
		RMID rmid = (RMID)iRmdef;
		RoomDefinition * pRmdef = s_rmlib.m_apRmdef[iRmdef];

		FrRoom * pRoomFrog = Frog_PRoomAllocate(&pWorld->m_tworld, pRmdef->m_dX, pRmdef->m_dY);

		GameRoom * pGroom = &pWorld->m_mpRmidGroom[rmid];
		pGroom->m_pRoomFrog = pRoomFrog;
		pGroom->m_pRmdef = pRmdef;
		pGroom->m_rmid = rmid;

		Frog_SetRoomTiles(pRoomFrog, &pWorld->m_tworld.m_tmap, pRmdef->m_aiTile);
	}

	GameRoom * pGroomStarting = &pWorld->m_mpRmidGroom[RMID_Min];
	pWorld->m_pGroomCur = NULL;
	SetCurRoom(pWorld, pGroomStarting);

	FrTileWorld * pTworld = &pWorld->m_tworld;
	pWorld->m_pGentAvatar = PGentAllocate(pWorld, ENTK_Avatar, 3, 3, 3);//'@');
	Frog_AddToRoom(pWorld->m_pGroomCur->m_pRoomFrog, Frog_PEnt(pTworld, pWorld->m_pGentAvatar->m_entid), EUPO_Avatar);
}

static void UpdateInput(World * pWorld, FrInput * pInput)
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
		case KEYCODE_ArrowUp:		MoveAvatar(pWorld, pWorld->m_pGentAvatar, 0, 1);	break;
		case KEYCODE_ArrowDown:		MoveAvatar(pWorld, pWorld->m_pGentAvatar, 0, -1);	break;
		case KEYCODE_ArrowLeft:		MoveAvatar(pWorld, pWorld->m_pGentAvatar, -1, 0);	break;
		case KEYCODE_ArrowRight:	MoveAvatar(pWorld, pWorld->m_pGentAvatar, 1, 0);	break;

		case KEYCODE_S:
			{
				if (pInev->m_finev & FINEV_Ctrl)
				{
					const char * pChzFilename = "source\\worldTest.inl";
					FTryDumpRoomFile(pChzFilename);
					Frog_PostNote(&pWorld->m_noteq, NOTEK_LowPriority, "Wrote inline file '%s'", pChzFilename);
				}
			}
		}
	}
	Frog_ClearInputEvents(pInput->m_pInevfifo);
}

void UpdateWorldMaze(World * pWorld, FrDrawContext * pDrac, FrInput * pInput, f32 dT)
{
	FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	FrVec2 posText = Frog_Vec2Create(10.0f, 300.0f);
	FrVec2 posNoteQueue = Frog_Vec2Create(10.0f, 250.0f);

	//char aCh[32];
	//sprintf_s(aCh, FR_DIM(aCh), "$%d, %d keys", pWorld->m_aCIik[IIK_Coin], pWorld->m_aCIik[IIK_Key]);
	Frog_DrawTextRaw(pDrac, posText, "test hud");

	Frog_RenderNoteQueue(pDrac, &pWorld->m_noteq, posNoteQueue, dT);

	FrRoomTransition * pRoomt = &pWorld->m_roomt;
	GameRoom * pGroomPrev = NULL;
	if (pRoomt->m_roomtk != ROOMTK_None && pRoomt->m_pRoomPrev)
	{
		pGroomPrev = &pWorld->m_mpRmidGroom[pRoomt->m_pRoomPrev->m_roomid];
	}

	Frog_UpdateTransition(&pWorld->m_roomt, dT);
	if (pRoomt->m_r >= 1.0f)
	{
		if (pGroomPrev)
		{
			OnExitRoom(pWorld, pGroomPrev);
		}
		Frog_SetTransition(pRoomt, ROOMTK_None, NULL, pRoomt->m_pRoom);
	}

	Frog_RenderTransition(pDrac, &pWorld->m_tworld, pRoomt, s_posScreen);

	UpdateInput(pWorld, pInput);

	if (pWorld->m_roomt.m_pRoomPrev)
	{
		FrRoom * pRoomPrev = pWorld->m_roomt.m_pRoomPrev;
		Frog_SortEntityUpdateList(pRoomPrev);

		UpdateRoomEntities(pWorld, pRoomPrev, dT);
	}

	GameRoom * pGroomCur = pWorld->m_pGroomCur;
	Frog_SortEntityUpdateList(pGroomCur->m_pRoomFrog);

	UpdateRoomEntities(pWorld, pGroomCur->m_pRoomFrog, dT);
}
