#pragma once

#include "Common.h"

typedef struct FrPlatform_t FrPlatform;



typedef enum EVENTK_tag
{
    EVENTK_Keyboard,
	EVENTK_Joystick,
    EVENTK_TextInput,
    EVENTK_Window,
    EVENTK_Quit,
	EVENTK_JoystickConnected,
	EVENTK_JoystickDisconnected,

	EVENTK_Max,
	EVENTK_Min = 0,
	EVENTK_Nil = -1,
} EVENTK;

typedef enum KEYCODE_tag
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

	KEYCODE_JoypadButton1 = 80,
	KEYCODE_JoypadButton2 = 81,
	KEYCODE_JoypadButton3 = 82,
	KEYCODE_JoypadButton4 = 83,
	KEYCODE_JoypadButton5 = 84,
	KEYCODE_JoypadButton6 = 85,
	KEYCODE_JoypadButton7 = 86,
	KEYCODE_JoypadButton8 = 87,
	KEYCODE_JoypadButton9 = 88,
	KEYCODE_JoypadButton10 = 89,
	KEYCODE_JoypadButton11 = 90,
	KEYCODE_JoypadButton12 = 91,
	KEYCODE_JoypadButton13 = 92,
	KEYCODE_JoypadButton14 = 93,
	KEYCODE_JoypadButton15 = 94,
	KEYCODE_JoypadButton16 = 95,
	KEYCODE_JoypadButton17 = 96,
	KEYCODE_JoypadButton18 = 97,
	KEYCODE_JoypadButton19 = 98,
	KEYCODE_JoypadButton20 = 99,

	KEYCODE_Max,
	KEYCODE_Min = 0,
	KEYCODE_Nil = -1,
} KEYCODE;



typedef struct FrInputEvent_t // tag=inev
{
    EVENTK				m_eventk;
    EDGES				m_edges;
	s32					m_nDeviceId;
    KEYCODE				m_keycode;
    u32					m_nTextInput;
} FrInputEvent;

typedef struct FrInputEventFifo_t // tag = inevfifo
{
	FrInputEvent		m_aInev[100];
	int					m_iInevFront;
	int					m_cInev;
} FrInputEventFifo;

typedef struct FrInputEventIterator_t // tag = inevit
{
	FrInputEventFifo *		m_pInevfifo;
	int						m_iiInev;	// how many events have we returned (not a wraparound index into m_aInev
} FrInputEventIterator;

typedef struct FrInput_t // tag = input
{
	FrInputEventFifo *		m_pInevfifo;
	EDGES					m_mpKeycodeEdges[KEYCODE_Max];

} FrInput;

FROG_CALL void Frog_InitInput(FrInput * pInput, FrPlatform * pPlat);
FROG_CALL void Frog_PollInput(FrInput * pInput);

FROG_CALL void Frog_InitEventFifo(FrInputEventFifo * pInevfifo);
FROG_CALL void Frog_InitInputEvent(FrInputEvent * pInev);
FROG_CALL void Frog_ClearInputEvents(FrInputEventFifo * pInevfifo);

//FrInputEvent * Frog_PInevFront(FrInputEventFifo * pInevfifo);
FROG_CALL FrInputEventIterator Frog_Inevit(FrInputEventFifo * pInevfifo);
FROG_CALL FrInputEvent * Frog_PInevNext(FrInputEventIterator * pInevit);
FROG_CALL FrInputEvent * Frog_PInevPushNew(FrInputEventFifo * pInevfifo);

FROG_CALL KEYCODE KeycodeFromGlfwKey(int nKey);

