#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef enum ENTK_t // tag = ent
{
	ENTK_Avatar,
	ENTK_Key,
	ENTK_Lock,

	ENTK_Max,
	ENTK_Min = 0,
	ENTK_Nil = -1
} ENTK;

typedef enum EUPO_t // Entity UPdate Order
{
	EUPO_Avatar,
	EUPO_Enemies,
	EUPO_Items,

	EUPO_Max,
	EUPO_Min = 0,
	EUPO_Nil = -1
} EUPO;

typedef struct Room_t // tag = room
{
	FrScreen *			m_pScr;
	
	Tentid				m_aTentid[64];			// entities belonging to this room
	int					m_cTentid;				// number of entities belonging to this room
	int					m_iRoom;				// which room is this?
} Room;

typedef struct Entity_t // tag = ent
{
	ENTK				m_entk;
	EUPO				m_eupo;					// Which group is this entity updating with? 
	Tentid				m_tentid;				// tile entity id
	int					m_ipEntUpdate;			// index of this entity in the maze update array
	int					m_iRoom;				// which roomm is this entity parented to?
	int					m_x;
	int					m_y;
	char				m_iTile;				// lookup into tile registry

	FrScreen *			m_pScrParent;
} Entity;

typedef struct EntityMaze_t // tag = maze
{
	FrTileWorld			m_tworld;

	Entity				m_mpTentidEnt[kCTentWorldMax];
	Entity *			m_apEntUpdate[kCTentWorldMax];
	Entity *			m_pEntAvatar;

	Room *				m_aRoom;		// world screen grid
	Room *				m_pRoomCur;

	FrScreenTransition	m_scrtr;
	int					m_xScr;
	int					m_yScr;

	int					m_cpEntUpdate;
	bool				m_fUpdateIsDirty;
} EntityMaze;

FROG_CALL void InitEntityMaze(EntityMaze * pMaze);
FROG_CALL void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);

void AddEntityToRoom(Room * pRoom, Entity * pEnt);
void RemoveEntityFromRoom(Room * pRoom, Entity * pEnt);
