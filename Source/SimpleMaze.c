#include "SimpleMaze.h"

const char * s_pChzRoom =
"WWWWWWWWWWWW" // 0
"W..........W"
"W.......C..W"
"W..........W"
"W..........W" // 4
"............"
"..@........."
"W..........W"
"W..........W" // 8
"W..........W"
"W..........W"
"WWWWWWWWWWWW";


void InitMaze(Maze * pMaze)
{
	FrColor colWallFg = Frog_ColCreate(0xFFa5c9c3);
	FrColor colWallBg = Frog_ColCreate(0xFF85aaa4);
	FrColor colGrassFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colGrassBg = Frog_ColCreate(0xFF156f3a); //#3a6f16
	FrColor colItemFg = Frog_ColCreate(0xFFa4d8d1); //#d1d8a4
	FrColor colItemBg = Frog_ColCreate(0xFF383e26); //#383e26

	Frog_SetTile(&pMaze->m_tmap, 'W', 'W', colWallFg, colWallBg);
	Frog_SetTile(&pMaze->m_tmap, '.', ' ', colGrassFg, colGrassBg);
	Frog_SetTile(&pMaze->m_tmap, 'C', 'C', colItemFg, colItemBg);
	Frog_SetTile(&pMaze->m_tmap, '@', '@', colItemFg, colItemBg);

	pMaze->m_pScrCur = Frog_AllocateScreen(12, 12);
	Frog_MapScreen(pMaze->m_pScrCur, &pMaze->m_tmap, s_pChzRoom);
}

void UpdateMaze(Maze * pMaze, FrDrawContext * pDrac)
{
	FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	static const char * s_pChzQuickFox = "TheQuickBrownFoxJumpsOverTheLazyDog";
	FrVec2 posText = Frog_Vec2Create(20.0f, 20.0f);

	Frog_DrawTextRaw(pDrac, posText, s_pChzQuickFox);
	Frog_RenderScreen(pDrac, pMaze->m_pScrCur, s_posScreen);
}
