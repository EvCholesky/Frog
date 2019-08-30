#include "Common.h"
#include "FrogRender.h"
#include "FrogPlatform.h"

FrDrawContext g_drac;
static const int s_dXWindow = 1200;
static const int s_dYWindow = 800;

/*
const char * s_pChzRoom =

"▓▓▓▓▓▓▓▓▓▓▓▓▓" //0
"▓           ▓"
"▓   ▒       ▓"
"▓   ▒       ▓"
"▓           ▓" //4
"▓           ▓"
"▓           ▓"
"▓           ▓"
"▓           ▓" //8
"▓           ▓"
"▓           ▓"
"▓▓▓▓▓▓▓▓▓▓▓▓▓";
*/

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



int main(int cpChzArg, const char * apChzArg[])
{
	static const int s_nHzTarget = 60;

	if (!Frog_FTryInitPlatform(&g_plat, s_nHzTarget))
		return 0;

	if (!Frog_FTryCreateWindow(&g_plat, s_dXWindow, s_dYWindow, "Frog"))
		return 0;

	if (!Frog_FTryStaticInitDrawContext(&g_drac))
		return 0;

	Frog_SetupOrthoViewport(0, 0, s_dXWindow, s_dYWindow);

	auto cTickLast = Frog_CTickWallClock();

	static FrVec2 s_posScreen = Frog_Vec2Create(10, 400);
	static const char * s_pChzQuickFox = "TheQuickBrownFoxJumpsOverTheLazyDog";
	auto posText = Frog_Vec2Create(20.0f, 20.0f);


	FrColor colWallFg = Frog_ColCreate(0xFFa5c9c3);
	FrColor colWallBg = Frog_ColCreate(0xFF85aaa4);
	FrColor colGrassFg = Frog_ColCreate(0xFF267043); //#437026
	FrColor colGrassBg = Frog_ColCreate(0xFF156f3a); //#3a6f16
	FrColor colItemFg = Frog_ColCreate(0xFFa4d8d1); //#d1d8a4
	FrColor colItemBg = Frog_ColCreate(0xFF383e26); //#383e26
	FrTileMap tmap;
	Frog_SetTile(&tmap, 'W', 'W', colWallFg,  colWallBg);
	Frog_SetTile(&tmap, '.', ' ', colGrassFg,  colGrassBg);
	Frog_SetTile(&tmap, 'C', 'C', colItemFg,  colItemBg);
	Frog_SetTile(&tmap, '@', '@', colItemFg,  colItemBg);

	auto pScr = Frog_AllocateScreen(12, 12);
	Frog_MapScreen(pScr, &tmap, s_pChzRoom);

	while (!Frog_FShouldWindowClose(&g_plat))
	{
		Frog_ClearScreen(&g_plat);

		Frog_DrawTextRaw(&g_drac, posText, s_pChzQuickFox);
		Frog_RenderScreen(&g_drac, pScr, s_posScreen);

		Frog_FlushFontVerts(&g_drac);
		Frog_SwapBuffers(&g_plat);
		Frog_PollInput(&g_plat);

		Frog_WaitUntilFrameEnd(&g_plat.m_pltime, cTickLast);
		Frog_FrameEndTimers(&g_plat);
		cTickLast = Frog_CTickWallClock();
	}

	Frog_ShutdownPlatform(&g_plat);
	return 1;
}
