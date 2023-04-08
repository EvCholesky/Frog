#include "FrogInput.h"
#include "FrogPlatform.h"
#include "GlWrapper.h"



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


static FrInputEventFifo s_inevfifo;

static void GlfwMouseButtonCallback(GLFWwindow * pWindow, int nButton, int nAction, int nMods)
{

}

static void GlfwKeyboardCallback(GLFWwindow * pWindow, int nKey, int nScancode, int nAction, int nMods)
{
	EDGES edges;
	switch (nAction)
	{
		case GLFW_PRESS:	edges = EDGES_Press;	break;
		case GLFW_RELEASE:	edges = EDGES_Release;	break;
		default: return; // GLFW_REPEAT
	}

	u8 finev = FINEV_None;
	if (nMods & GLFW_MOD_CONTROL)
	{
		finev |= FINEV_Ctrl;
	}

	if (nMods & GLFW_MOD_SHIFT)
	{
		finev |= FINEV_Shift;
	}

	if (nMods & GLFW_MOD_ALT)
	{
		finev |= FINEV_Alt;
	}
	FrInputEvent * pInev = Frog_PInevPushNew(&s_inevfifo);
	pInev->m_eventk = EVENTK_Keyboard;
	pInev->m_edges = edges;
	pInev->m_nDeviceId = 0;
	pInev->m_finev = finev;
	pInev->m_keycode = KeycodeFromGlfwKey(nKey);
}

void Frog_InitInput(FrInput * pInput, FrPlatform * pPlat)
{
	ZeroAB(pInput->m_mpKeycodeEdges, sizeof(pInput->m_mpKeycodeEdges));

	pInput->m_pInevfifo = &s_inevfifo;
	Frog_InitEventFifo(pInput->m_pInevfifo);
	glfwSetKeyCallback(pPlat->m_pGlfwin, GlfwKeyboardCallback);

	glfwSetMouseButtonCallback(pPlat->m_pGlfwin, GlfwMouseButtonCallback);
}

FROG_CALL void Frog_PollInput(FrInput * pInput)
{
	for (int keycode = 0; keycode < KEYCODE_Max; ++keycode)
	{
		pInput->m_mpKeycodeEdges[keycode] = (pInput->m_mpKeycodeEdges[keycode] < EDGES_Hold) ? EDGES_Off : EDGES_Hold;
	}

	glfwPollEvents();
}

FROG_CALL void Frog_InitEventFifo(FrInputEventFifo * pInevfifo)
{
	pInevfifo->m_cInev = 0;
	pInevfifo->m_iInevFront = 0;
}

FROG_CALL void Frog_InitInputEvent(FrInputEvent * pInev)
{
	pInev->m_eventk = EVENTK_Nil;
	pInev->m_edges = EDGES_Off;
	pInev->m_keycode = KEYCODE_Unknown;
	pInev->m_nTextInput = 0;	
	pInev->m_nDeviceId = 0;
}

FROG_CALL FrInputEvent * Frog_PInevFront(FrInputEventFifo * pInevfifo)
{
	return &pInevfifo->m_aInev[pInevfifo->m_iInevFront];
}

FROG_CALL FrInputEvent * Frog_PInevPushNew(FrInputEventFifo * pInevfifo)
{
	if (!FR_FVERIFY(pInevfifo->m_cInev < FR_DIM(pInevfifo->m_aInev), "EventFifo overflow"))
		return Frog_PInevFront(pInevfifo);

	int iEvent = (pInevfifo->m_iInevFront + pInevfifo->m_cInev) % FR_DIM(pInevfifo->m_aInev);
	++pInevfifo->m_cInev;

	FrInputEvent * pEvent = &pInevfifo->m_aInev[iEvent];
	Frog_InitInputEvent(pEvent);
	return pEvent;
}

FROG_CALL void PopFront(FrInputEventFifo * pInevfifo)
{
	if (!FR_FVERIFY(pInevfifo->m_cInev > 0, "SEventFifo underflow"))
		return;

	pInevfifo->m_iInevFront = (pInevfifo->m_iInevFront + 1) % FR_DIM(pInevfifo->m_aInev);
	--pInevfifo->m_cInev;
}

FROG_CALL void Frog_ClearInputEvents(FrInputEventFifo * pInevfifo)
{
	pInevfifo->m_cInev = 0;
}

FROG_CALL FrInputEventIterator Frog_Inevit(FrInputEventFifo * pInevfifo)
{
	FrInputEventIterator inevit;
	if (pInevfifo->m_cInev == 0)	
	{
		inevit.m_pInevfifo = NULL;	
		inevit.m_iiInev = 0;
		return inevit;
	}

	inevit.m_pInevfifo = pInevfifo;	
	inevit.m_iiInev = 0;
	return inevit;
}

FROG_CALL FrInputEvent * Frog_PInevNext(FrInputEventIterator * pInevit)
{
	FrInputEventFifo * pInevfifo = pInevit->m_pInevfifo;
	if (!pInevfifo)
		return NULL;

	int iiInev = pInevit->m_iiInev;
	if (pInevit->m_iiInev >= pInevfifo->m_cInev)
		return NULL;

	++pInevit->m_iiInev;
	return &pInevfifo->m_aInev[(pInevfifo->m_iInevFront + iiInev) % pInevfifo->m_cInev];
}


