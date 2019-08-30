#include <windows.h>
#include <mmsystem.h>

#include "FrogPlatform.h"

#include "glfw/include/GLFW/glfw3.h"
#include "glfw/deps/GL/glext.h"

FrKeyPressState g_keyps;
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

FR_FORCE_INLINE f32 DTElapsed(s64 cTickStart, s64 cTickEnd, float rTickToSecond)
{
    return f32(cTickEnd - cTickStart) * rTickToSecond;
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

KEYCODE KeycodeFromGlfwKey(int nKey)
{
	struct SKeyPair // tag = keyp 
	{
		int			m_nKeyGlfw;
		KEYCODE		m_keycode;
	};

	SKeyPair s_aKeyp [] = 
	{
		{ GLFW_KEY_LEFT,	KEYCODE_ArrowLeft},
		{ GLFW_KEY_RIGHT,	KEYCODE_ArrowRight},
		{ GLFW_KEY_UP,		KEYCODE_ArrowUp},
		{ GLFW_KEY_DOWN,	KEYCODE_ArrowDown},
		{ GLFW_KEY_ESCAPE,	KEYCODE_Escape},

		{ GLFW_KEY_ENTER,	KEYCODE_Enter},

		{ GLFW_KEY_F1,		KEYCODE_F1},
		{ GLFW_KEY_F2,		KEYCODE_F2},
		{ GLFW_KEY_F3,		KEYCODE_F3},
		{ GLFW_KEY_F4,		KEYCODE_F4},
		{ GLFW_KEY_F5,		KEYCODE_F5},
		{ GLFW_KEY_F6,		KEYCODE_F6},
		{ GLFW_KEY_F7,		KEYCODE_F7},
		{ GLFW_KEY_F8,		KEYCODE_F8},
		{ GLFW_KEY_F9,		KEYCODE_F9},
		{ GLFW_KEY_F10,		KEYCODE_F10},
		{ GLFW_KEY_F11,		KEYCODE_F11},
		{ GLFW_KEY_F12,		KEYCODE_F12},

		{ GLFW_KEY_SPACE,	KEYCODE_Space },
		{ GLFW_KEY_APOSTROPHE,	KEYCODE_Apostrophe },
		{ GLFW_KEY_COMMA,	KEYCODE_Comma },
		{ GLFW_KEY_MINUS,	KEYCODE_Minus },
		{ GLFW_KEY_PERIOD,	KEYCODE_Period },
		{ GLFW_KEY_SLASH,	KEYCODE_Slash },
		{ GLFW_KEY_0,	KEYCODE_0 },
		{ GLFW_KEY_1,	KEYCODE_1 },
		{ GLFW_KEY_2,	KEYCODE_2 },
		{ GLFW_KEY_3,	KEYCODE_3 },
		{ GLFW_KEY_4,	KEYCODE_4 },
		{ GLFW_KEY_5,	KEYCODE_5 },
		{ GLFW_KEY_6,	KEYCODE_6 },
		{ GLFW_KEY_7,	KEYCODE_7 },
		{ GLFW_KEY_8,	KEYCODE_8 },
		{ GLFW_KEY_9,	KEYCODE_9 },
		{ GLFW_KEY_SEMICOLON,	KEYCODE_Semicolon },
		{ GLFW_KEY_EQUAL,	KEYCODE_Equal },
		{ GLFW_KEY_A,	KEYCODE_A },
		{ GLFW_KEY_B,	KEYCODE_B },
		{ GLFW_KEY_C,	KEYCODE_C },
		{ GLFW_KEY_D,	KEYCODE_D },
		{ GLFW_KEY_E,	KEYCODE_E },
		{ GLFW_KEY_F,	KEYCODE_F },
		{ GLFW_KEY_G,	KEYCODE_G },
		{ GLFW_KEY_H,	KEYCODE_H },
		{ GLFW_KEY_I,	KEYCODE_I },
		{ GLFW_KEY_J,	KEYCODE_J },
		{ GLFW_KEY_K,	KEYCODE_K },
		{ GLFW_KEY_L,	KEYCODE_L },
		{ GLFW_KEY_M,	KEYCODE_M },
		{ GLFW_KEY_N,	KEYCODE_N },
		{ GLFW_KEY_O,	KEYCODE_O },
		{ GLFW_KEY_P,	KEYCODE_P },
		{ GLFW_KEY_Q,	KEYCODE_Q },
		{ GLFW_KEY_R,	KEYCODE_R },
		{ GLFW_KEY_S,	KEYCODE_S },
		{ GLFW_KEY_T,	KEYCODE_T },
		{ GLFW_KEY_U,	KEYCODE_U },
		{ GLFW_KEY_V,	KEYCODE_V },
		{ GLFW_KEY_W,	KEYCODE_W },
		{ GLFW_KEY_X,	KEYCODE_X },
		{ GLFW_KEY_Y,	KEYCODE_Y },
		{ GLFW_KEY_Z,	KEYCODE_Z },
		{ GLFW_KEY_LEFT_BRACKET,	KEYCODE_LeftBracket },
		{ GLFW_KEY_BACKSLASH,	KEYCODE_Backslash },			// '\'
		{ GLFW_KEY_RIGHT_BRACKET,	KEYCODE_RightBracket },
		{ GLFW_KEY_GRAVE_ACCENT,	KEYCODE_GraveAccent },		// `
	};

	SKeyPair * pKeypMax = FR_PMAX(s_aKeyp);
	for (SKeyPair * pKeyp = s_aKeyp; pKeyp != pKeypMax; ++pKeyp)
	{
		if (pKeyp->m_nKeyGlfw == nKey)
			return pKeyp->m_keycode;
	}

	return KEYCODE_Unknown;
}

static void GlfwKeyCallback(GLFWwindow * pGlfwin, int key, int scancode, int action, int mods)
{
	auto keycode = KeycodeFromGlfwKey(key);
    if (action == GLFW_PRESS)
	{
		g_keyps.m_mpKeycodeEdges[keycode] = EDGES_Press;
	}
    if (action == GLFW_RELEASE)
	{
		g_keyps.m_mpKeycodeEdges[keycode] = EDGES_Release;
	}
}

static void GlfwMouseButtonCallback(GLFWwindow * pGlfwin, int nButton,int nAction, int mods)
{
}

static void GlfwMouseCursorCallback(GLFWwindow * pGlfwin, double xCursor, double yCursor)
{
}

static void GlfwCharCallback(GLFWwindow * pGlfwin, unsigned int c)
{
}

static void GlfwScrollCallback(GLFWwindow * pGlfwin, double dX, double dY)
{
}

FrKeyPressState::FrKeyPressState()
{
	ZeroAB(m_mpKeycodeEdges, sizeof(m_mpKeycodeEdges));
}

FrPlatform::FrPlatform()
:m_pGlfwin(nullptr)
,m_fRequestedSaveState(false)
,m_fRequestedLoadState(false)
{
	Frog_ClearTimers(this); 
}

bool Frog_FTryInitPlatform(FrPlatform * pPlat, int nHzTarget)
{
	if (!glfwInit())
		return false;

	Frog_InitPlatformTime(&pPlat->m_pltime, nHzTarget);

	//InitJoystickInput(pPlat);
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

	glfwSetKeyCallback(pGlfwin, GlfwKeyCallback);
	glfwSetMouseButtonCallback(pGlfwin, GlfwMouseButtonCallback);
	glfwSetCursorPosCallback(pGlfwin, GlfwMouseCursorCallback);
    glfwSetScrollCallback(pGlfwin, GlfwScrollCallback);
    glfwSetCharCallback(pGlfwin, GlfwCharCallback);

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

	glfwGetFramebufferSize(pPlat->m_pGlfwin, &dX, &dY);
    glViewport(0, 0, dX, dY);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Frog_SwapBuffers(FrPlatform * pPlat)
{
	glfwSwapBuffers(pPlat->m_pGlfwin);
}

void Frog_PollInput(FrPlatform * pPlat)
{
	for (int keycode = 0; keycode < KEYCODE_Max; ++keycode)
	{
		g_keyps.m_mpKeycodeEdges[keycode] = (g_keyps.m_mpKeycodeEdges[keycode] < EDGES_Hold) ? EDGES_Off : EDGES_Hold;
	}

	glfwPollEvents();
	//PollJoystickInput(pPlat);
}

void Frog_ShutdownPlatform(FrPlatform * pPlat)
{
	glfwTerminate();
}





