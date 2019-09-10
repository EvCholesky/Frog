#pragma once

#include "Common.h"

struct GLFWwindow;

typedef enum TVAL_t // Timer record 
{
	TVAL_TotalFrame,

	TVAL_Max,
	TVAL_Min = 0,
	TVAL_Nil = -1,
} TVAL;

typedef enum JCONS_t // Joystick CONnection State
{
	JCONS_Disconnected,
	JCONS_Connected,

	JCONS_Nil = -1
} JCONS;

typedef struct FrPlatformTime_t // tag = pltime
{
	s32					m_nHzDesired;
	f32					m_dTFrameTarget;	
	f32					m_rTickToSecond;		// 1.0f / cTickPerSecond;
	s64					m_cTickPerSecond;
	bool				m_fHasMsSleep;

} FrPlatformTime;

void Frog_InitPlatformTime(FrPlatformTime * pPltime);

typedef struct FrPlatform_t // tag = plat
{
	GLFWwindow *		m_pGlfwin;
	FrPlatformTime		m_pltime;
	s64					m_mpTvalCTickStart[TVAL_Max];	// when was StartTimer called for a given TVAL
	s64					m_mpTvalCTick[TVAL_Max];	// total ticks this frame
	float				m_mpTvalDTFrame[TVAL_Max];		// last frame's timings - used for reporting
	bool				m_fRequestedSaveState;
	bool				m_fRequestedLoadState;

	static const int s_cHistorySample = 60;
	float				m_aTHistory[s_cHistorySample];

} FrPlatform;

extern FrPlatform g_plat;

bool Frog_FTryInitPlatform(FrPlatform * pPlat, int nHzTarget);
void Frog_ShutdownPlatform(FrPlatform * pPlat);

void Frog_ClearScreen(FrPlatform * pPlat);
void Frog_SwapBuffers(FrPlatform * pPlat);

void Frog_InitPlatformTime(FrPlatformTime * pPltime, int nHzTarget);
void Frog_WaitUntilFrameEnd(FrPlatformTime * pPltime, s64 cTickStart);

bool Frog_FTrySetTimerResolution(u32 msResolution);
s64 Frog_CTickPerSecond();
s64 Frog_CTickWallClock();

bool Frog_FTryCreateWindow(FrPlatform * pPlat, int dX, int dY, const char* pChzTitle);
bool Frog_FShouldWindowClose(FrPlatform * pPlat); 
void Frog_ClearScreen(FrPlatform * pPlat);

void Frog_StartTimer(TVAL tval);
void Frog_EndTimer(TVAL tval);
f32 Frog_DTFrame(FrPlatform * pPlat, TVAL tval);
f32 Frog_DMsecFrame(FrPlatform * pPlat, TVAL tval);
void Frog_FrameEndTimers(FrPlatform * pPlat);
void Frog_ClearTimers(FrPlatform * pPlat);

