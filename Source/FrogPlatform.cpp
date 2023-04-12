#include <windows.h>
#include <mmsystem.h>

#include "FrogPlatform.h"

#include "glfw/include/GLFW/glfw3.h"
#include "glfw/deps/GL/glext.h"

FrPlatform g_plat;

s64 Frog_CTickPerSecond()
{
    LARGE_INTEGER lrgintFrequency;
    QueryPerformanceFrequency(&lrgintFrequency);
    return lrgintFrequency.QuadPart;
}

s64 Frog_CTickWallClock()
{
    LARGE_INTEGER lrgint;
    QueryPerformanceCounter(&lrgint);
    return lrgint.QuadPart;
}

bool Frog_FTrySetTimerResolution(u32 msResolution)
{
    return timeBeginPeriod(msResolution) == TIMERR_NOERROR;
}

void Frog_InitPlatformTime(FrPlatformTime * pPltime, int nHzTarget)
{
    pPltime->m_dTFrameTarget = 1.0f / nHzTarget;

	static const int s_msResolutionDesired = 1;
    pPltime->m_fHasMsSleep = Frog_FTrySetTimerResolution(s_msResolutionDesired);

    pPltime->m_cTickPerSecond = Frog_CTickPerSecond();
	pPltime->m_rTickToSecond = 1.0f / f32(pPltime->m_cTickPerSecond);
}

void Frog_ClearTimers(FrPlatform * pPlat)
{
	ZeroAB(pPlat->m_mpTvalCTickStart, sizeof(pPlat->m_mpTvalCTickStart));
	ZeroAB(pPlat->m_mpTvalCTick, sizeof(pPlat->m_mpTvalCTick));
	ZeroAB(pPlat->m_mpTvalDTFrame, sizeof(pPlat->m_mpTvalDTFrame));

	ZeroAB(pPlat->m_aTHistory, sizeof(pPlat->m_aTHistory));
}

void Frog_StartTimer(TVAL tval)
{
	auto pPlat = &g_plat;
	FR_ASSERT(pPlat->m_mpTvalCTickStart[tval] == 0, "EndTimer was not called");	
	pPlat->m_mpTvalCTickStart[tval] =  Frog_CTickWallClock();
}

void Frog_EndTimer(TVAL tval)
{
	auto pPlat = &g_plat;
	auto cTickNow = Frog_CTickWallClock();
	auto pCTickStart = & pPlat->m_mpTvalCTickStart[tval];
	FR_ASSERT(*pCTickStart != 0, "StartTimer was not called");	

	pPlat->m_mpTvalCTick[tval] += cTickNow - *pCTickStart;
	*pCTickStart = 0;
}

f32 Frog_DTFrame(FrPlatform * pPlat, TVAL tval)
{
	return pPlat->m_mpTvalDTFrame[tval];
}

f32 Frog_DMsecFrame(FrPlatform * pPlat, TVAL tval)
{
	return pPlat->m_mpTvalDTFrame[tval] * 1000.0f;
}

void Frog_FrameEndTimers(FrPlatform * pPlat)
{
	f32 rTickToSecond = pPlat->m_pltime.m_rTickToSecond;
	for (int tval = 0; tval < TVAL_Max; ++tval)
	{
		if (!FR_FVERIFY(pPlat->m_mpTvalCTickStart[tval] == 0, "EndTimer was not called before frame end"))
		{
			Frog_EndTimer(TVAL(tval));
		}

		pPlat->m_mpTvalDTFrame[tval] = f32(pPlat->m_mpTvalCTick[tval]) * rTickToSecond;
		pPlat->m_mpTvalCTick[tval] = 0;
	}

	static const TVAL s_tvalHistogram = TVAL_TotalFrame;// TVAL_FamicomUpdate;
	for (int iT = FR_DIM(pPlat->m_aTHistory); --iT >= 1; )
	{
		pPlat->m_aTHistory[iT] = pPlat->m_aTHistory[iT - 1];
	}
	pPlat->m_aTHistory[0] = pPlat->m_mpTvalDTFrame[s_tvalHistogram];
}

void Frog_WaitUntilFrameEnd(FrPlatformTime * pPltime, s64 cTickStart)
{
	auto cTickEnd = Frog_CTickWallClock();
	f32 dTElapsed = DTElapsed(cTickStart, cTickEnd, pPltime->m_rTickToSecond);

	if (dTElapsed < pPltime->m_dTFrameTarget)
	{                        
		if (pPltime->m_fHasMsSleep)
		{
			u32 SleepMS = u32(1000.0 * (pPltime->m_dTFrameTarget - dTElapsed));
			if (SleepMS > 0)
			{
				Sleep(SleepMS);
			}
		}

		while (dTElapsed < pPltime->m_dTFrameTarget)
		{                            
			dTElapsed = DTElapsed(cTickStart, Frog_CTickWallClock(), pPltime->m_rTickToSecond);
		}
	}
}

void Frog_InitPlatformTime(FrPlatformTime * pPltime)
{
	pPltime->m_nHzDesired = 60;
	pPltime->m_dTFrameTarget = 1.0f / 60.0f;
	pPltime->m_rTickToSecond = 0.001f;
	pPltime->m_cTickPerSecond = 1000;
	pPltime->m_fHasMsSleep = false;
}

bool Frog_FTryInitPlatform(FrPlatform * pPlat, int nHzTarget)
{
	pPlat->m_pGlfwin = nullptr;
	pPlat->m_fRequestedSaveState = false;
	pPlat->m_fRequestedLoadState = false;

	Frog_InitPlatformTime(&pPlat->m_pltime);
	Frog_ClearTimers(pPlat); 

	if (!glfwInit())
		return false;

	Frog_InitPlatformTime(&pPlat->m_pltime, nHzTarget);
	return true;
}

bool Frog_FTryCreateWindow(FrPlatform * pPlat, int dX, int dY, const char* pChzTitle)
{
	auto pGlfwin = glfwCreateWindow(dX, dY, pChzTitle, nullptr, nullptr);
	
	if (!pGlfwin)
	{
		glfwTerminate();
		return false;
	}

	pPlat->m_pGlfwin = pGlfwin;
	glfwMakeContextCurrent(pGlfwin);
	glfwSwapInterval(0);

	return true;
}

bool Frog_FShouldWindowClose(FrPlatform * pPlat)
{
	return glfwWindowShouldClose(pPlat->m_pGlfwin) != 0;
}

void Frog_ClearScreen(FrPlatform * pPlat)
{
    int dX, dY;
    glfwGetFramebufferSize(pPlat->m_pGlfwin, &dX, &dY);
    glViewport(0, 0, dX, dY);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Frog_SwapBuffers(FrPlatform * pPlat)
{
	glfwSwapBuffers(pPlat->m_pGlfwin);
}

void Frog_ShutdownPlatform(FrPlatform * pPlat)
{
	glfwTerminate();
}





