#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef enum ENTID_t // tag = ent
{
	ENTID_Avatar,

	ENTID_Max,
	ENTID_Min = 0,
	ENTID_Nil = -1
} ENTID;

typedef struct Entity_t
{
	Tentid				m_tentid;
	int					m_x;
	int					m_y;
} Entity;

typedef struct EntityMaze_t // tag = maze
{
	FrTileWorld			m_tworld;

	Entity				m_aEnt[ENTID_Max];
	FrScreen **			m_mpIScrPScr;		// world screen grid

	FrScreen *			m_pScrCur;
	FrScreenTransition	m_scrtr;
	int					m_xScr;
	int					m_yScr;
} EntityMaze;

FROG_CALL void InitEntityMaze(EntityMaze * pMaze);
FROG_CALL void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);