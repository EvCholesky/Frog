#pragma once

#include "Common.h"

struct GLFWwindow;

enum KEYCODE : s16
{
	KEYCODE_Unknown = 0,
	KEYCODE_ArrowLeft = 1,
	KEYCODE_ArrowRight = 2,
	KEYCODE_ArrowUp = 3,
	KEYCODE_ArrowDown = 4,
	KEYCODE_Shift = 5,
	KEYCODE_Escape = 6,
	KEYCODE_MouseButtonLeft = 7,
	KEYCODE_MouseButtonRight = 8,

	KEYCODE_Enter = 10,

	KEYCODE_F1 =  11,
	KEYCODE_F2 =  12,
	KEYCODE_F3 =  13,
	KEYCODE_F4 =  14,
	KEYCODE_F5 =  15,
	KEYCODE_F6 =  16,
	KEYCODE_F7 =  17,
	KEYCODE_F8 =  18,
	KEYCODE_F9 =  19,
	KEYCODE_F10 = 20,
	KEYCODE_F11 = 21,
	KEYCODE_F12 = 22,

	KEYCODE_Space = 30,
	KEYCODE_Apostrophe = 31,
	KEYCODE_Comma = 32,
	KEYCODE_Minus = 33,
	KEYCODE_Period = 34,
	KEYCODE_Slash = 35,
	KEYCODE_0 = 36,
	KEYCODE_1 = 37,
	KEYCODE_2 = 38,
	KEYCODE_3 = 39,
	KEYCODE_4 = 40,
	KEYCODE_5 = 41,
	KEYCODE_6 = 42,
	KEYCODE_7 = 43,
	KEYCODE_8 = 44,
	KEYCODE_9 = 45,
	KEYCODE_Semicolon = 46,
	KEYCODE_Equal = 47,
	KEYCODE_A = 48,
	KEYCODE_B = 49,
	KEYCODE_C = 50,
	KEYCODE_D = 51,
	KEYCODE_E = 52,
	KEYCODE_F = 53,
	KEYCODE_G = 54,
	KEYCODE_H = 55,
	KEYCODE_I = 56,
	KEYCODE_J = 57,
	KEYCODE_K = 58,
	KEYCODE_L = 59,
	KEYCODE_M = 60,
	KEYCODE_N = 61,
	KEYCODE_O = 62,
	KEYCODE_P = 63,
	KEYCODE_Q = 64,
	KEYCODE_R = 65,
	KEYCODE_S = 66,
	KEYCODE_T = 67,
	KEYCODE_U = 68,
	KEYCODE_V = 69,
	KEYCODE_W = 70,
	KEYCODE_X = 71,
	KEYCODE_Y = 72,
	KEYCODE_Z = 73,
	KEYCODE_LeftBracket = 74,
	KEYCODE_Backslash = 75,
	KEYCODE_RightBracket = 76,
	KEYCODE_GraveAccent = 77,

	KEYCODE_JoypadButtonMin = 80, 
	KEYCODE_JoypadButton0 = KEYCODE_JoypadButtonMin,
	KEYCODE_JoypadButton1 = 81,
	KEYCODE_JoypadButton2 = 82,
	KEYCODE_JoypadButton3 = 83,
	KEYCODE_JoypadButton4 = 84,
	KEYCODE_JoypadButton5 = 85,
	KEYCODE_JoypadButton6 = 86,
	KEYCODE_JoypadButton7 = 87,
	KEYCODE_JoypadButton8 = 88,
	KEYCODE_JoypadButton9 = 89,
	KEYCODE_JoypadButton10 = 90,
	KEYCODE_JoypadButton11 = 91,
	KEYCODE_JoypadButton12 = 92,
	KEYCODE_JoypadButton13 = 93,
	KEYCODE_JoypadButton14 = 94,
	KEYCODE_JoypadButton15 = 95,
	KEYCODE_JoypadButton16 = 96,
	KEYCODE_JoypadButton17 = 97,
	KEYCODE_JoypadButton18 = 98,
	KEYCODE_JoypadButton19 = 99,

	KEYCODE_AxisNegMin = 100,
	KEYCODE_JoypadAxisNeg0 = KEYCODE_AxisNegMin,
	KEYCODE_JoypadAxisNeg1 = 101,
	KEYCODE_JoypadAxisNeg2 = 102,
	KEYCODE_JoypadAxisNeg3 = 103,
	KEYCODE_JoypadAxisNeg4 = 104,
	KEYCODE_JoypadAxisNeg5 = 105,
	KEYCODE_AxisNegMax = 106,

	KEYCODE_AxisPosMin = KEYCODE_AxisNegMax,
	KEYCODE_JoypadAxisPos0 = KEYCODE_AxisPosMin,
	KEYCODE_JoypadAxisPos1 = 107,
	KEYCODE_JoypadAxisPos2 = 108,
	KEYCODE_JoypadAxisPos3 = 109,
	KEYCODE_JoypadAxisPos4 = 110,
	KEYCODE_JoypadAxisPos5 = 111,
	KEYCODE_AxisPosMax = 112,

	KEYCODE_Max,
	KEYCODE_Min = 0,
	KEYCODE_Nil = -1,
};

enum TVAL // Timer record 
{
	TVAL_TotalFrame,

	TVAL_Max,
	TVAL_Min = 0,
	TVAL_Nil = -1,
};

enum JCONS // Joystick CONnection State
{
	JCONS_Disconnected,
	JCONS_Connected,

	JCONS_Nil = -1
};

struct FrPlatformTime // tag = pltime
{
						FrPlatformTime()
						:m_nHzDesired(60)
						,m_dTFrameTarget(1.0f/60.0f)
						,m_rTickToSecond(0.001f)
						,m_cTickPerSecond(1000)
						,m_fHasMsSleep(false)
							{ ; }

	s32					m_nHzDesired;
	f32					m_dTFrameTarget;	
	f32					m_rTickToSecond;		// 1.0f / cTickPerSecond;
	s64					m_cTickPerSecond;
	bool				m_fHasMsSleep;
};

struct FrKeyPressState // tag = keys
{
						FrKeyPressState();

	EDGES				m_mpKeycodeEdges[KEYCODE_Max];
};

struct FrPlatform // tag = plat
{
						FrPlatform();
	
	GLFWwindow *		m_pGlfwin;
	FrPlatformTime		m_pltime;
	s64					m_mpTvalCTickStart[TVAL_Max];	// when was StartTimer called for a given TVAL
	s64					m_mpTvalCTick[TVAL_Max];	// total ticks this frame
	float				m_mpTvalDTFrame[TVAL_Max];		// last frame's timings - used for reporting
	bool				m_fRequestedSaveState;
	bool				m_fRequestedLoadState;

	static const int s_cHistorySample = 60;
	float				m_aTHistory[s_cHistorySample];

	//FrPlatformAudio		m_plaud;

	//static const int s_cJoystickMax = 16;
	//FrPlatformJoystick	m_aPljoy[s_cJoystickMax];
};
extern FrPlatform g_plat;

bool Frog_FTryInitPlatform(FrPlatform * pPlat, int nHzTarget);
//void Frog_SetupDefaultInputMapping(Famicom * pFam);
void Frog_ShutdownPlatform(FrPlatform * pPlat);

void Frog_ClearScreen(FrPlatform * pPlat);
void Frog_SwapBuffers(FrPlatform * pPlat);
void Frog_PollInput(FrPlatform * pPlat);


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

