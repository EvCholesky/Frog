#include "Common.h"
#include "FrogRender.h"
#include "FrogPlatform.h"

#include "SimpleMaze.h"

FrDrawContext g_drac;
static const int s_dXWindow = 1200;
static const int s_dYWindow = 800;



int main(int cpChzArg, const char * apChzArg[])
{
	static const int s_nHzTarget = 60;

	if (!Frog_FTryInitPlatform(&g_plat, s_nHzTarget))
		return 0;

	if (!Frog_FTryCreateWindow(&g_plat, s_dXWindow, s_dYWindow, "Frog"))
		return 0;

	if (!Frog_FTryStaticInitDrawContext(&g_drac))
		return 0;

	if (!Frog_FTryInitDrawContext(&g_drac))
		return 0;

	Frog_SetupOrthoViewport(0, 0, s_dXWindow, s_dYWindow);

	auto cTickLast = Frog_CTickWallClock();

	Maze maze;
	InitMaze(&maze);

	while (!Frog_FShouldWindowClose(&g_plat))
	{
		Frog_ClearScreen(&g_plat);

		UpdateMaze(&maze, & g_drac);

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
