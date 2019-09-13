#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef struct EntityMaze_t // tag = maze
{
	FrTileMap			m_tmap;
	FrScreen *			m_pScrCur;
	FrScreenTransition	m_scrtr;
	int					m_xScr;
	int					m_yScr;
} EntityMaze;

FROG_CALL void InitEntityMaze(EntityMaze * pMaze);
FROG_CALL void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);