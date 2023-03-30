#include "Common.h"
#include "FrogInput.h"
#include "FrogRender.h"
#include "FrogPlatform.h"
#include "stdlib.h"

#define USE_WORLD_MAZE 1
#define USE_ENTITIES 1

#if USE_WORLD_MAZE
#include "WorldMaze.h"
#elif USE_ENTITIES
#include "EntityMaze.h"
#include "EntityMaze.c"
#else
#include "SimpleMaze.h"
#endif

FrInput g_input;
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

	Frog_InitInput(&g_input, &g_plat);

	if (!Frog_FTryStaticInitDrawContext(&g_drac))
		return 0;

	if (!Frog_FTryInitDrawContext(&g_drac))
		return 0;

	Frog_SetupOrthoViewport(0, 0, s_dXWindow, s_dYWindow);

	s64 cTickNew;
	s64 cTickPrev = Frog_CTickWallClock();

#if USE_WORLD_MAZE
	World maze;
	InitWorldMaze(&maze);
#elif USE_ENTITIES
	EntityMaze maze;
	InitEntityMaze(&maze);
#else
	SimpleMaze maze;
	InitSimpleMaze(&maze);
#endif

	f32 dTPrev = g_plat.m_pltime.m_dTFrameTarget;
	while (!Frog_FShouldWindowClose(&g_plat))
	{
		Frog_ClearScreen(&g_plat);

#if USE_WORLD_MAZE 
		UpdateWorldMaze(&maze, &g_drac, &g_input, dTPrev);
#elif USE_ENTITIES
		UpdateEntityMaze(&maze, &g_drac, &g_input, dTPrev);
#else
		UpdateSimpleMaze(&maze, &g_drac, &g_input, dTPrev);
#endif

		Frog_FlushFontVerts(&g_drac);
		Frog_SwapBuffers(&g_plat);

		Frog_WaitUntilFrameEnd(&g_plat.m_pltime, cTickPrev);
		Frog_FrameEndTimers(&g_plat);

		cTickNew = Frog_CTickWallClock();
		dTPrev = DTElapsed(cTickPrev, cTickNew, g_plat.m_pltime.m_rTickToSecond);
		cTickPrev = cTickNew;
	}

	Frog_ShutdownPlatform(&g_plat);
	return 1;
}
