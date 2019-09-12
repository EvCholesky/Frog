#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef struct Maze_t // tag = maze
{
	FrTileMap			m_tmap;
	FrScreen *			m_pScrCur;
	FrScreenTransition	m_scrtr;
	int					m_xScr;
	int					m_yScr;
} Maze;

FROG_CALL void InitMaze(Maze * pMaze);
FROG_CALL void UpdateMaze(Maze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);