#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef struct SimpleMaze_t // tag = maze
{
	FrTileWorld			m_tworld;
	FrRoom *			m_pRoomCur;
	FrRoomTransition	m_roomt;
	int					m_xScr;
	int					m_yScr;
} SimpleMaze;

FROG_CALL void InitSimpleMaze(SimpleMaze * pMaze);
FROG_CALL void UpdateSimpleMaze(SimpleMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);