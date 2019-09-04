#pragma once

#include "FrogRender.h"

typedef struct Maze_tag // tag = maze
{
	FrTileMap		m_tmap;
	FrScreen *		m_pScrCur;

} Maze;

FROG_CALL void InitMaze(Maze * pMaze);
FROG_CALL void UpdateMaze(Maze * pMaze, FrDrawContext * pDrac);

