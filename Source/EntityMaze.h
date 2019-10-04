#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;

typedef enum ENTK_t // tag = ent
{
	ENTK_Avatar,
	ENTK_Key,
	ENTK_Lock,
	ENTK_Coin,

	ENTK_Max,
	ENTK_Min = 0,
	ENTK_Nil = -1
} ENTK;

typedef struct GameRoom_t // tag = groom
{
	FrRoom *			m_pRoom;
	
	ENTID				m_aEntid[64];			// entities belonging to this room
	int					m_cEntid;				// number of entities belonging to this room
} GameRoom;

typedef enum EUPO_t
{
	EUPO_Avatar,
	EUPO_Item,
	
} EUPO;

typedef struct GameEntity_t // tag = gent
{
	ENTK				m_entk;
	ENTID				m_entid;				// tile entity id
	int					m_x;
	int					m_y;
	char				m_iTile;				// lookup into tile registry

	FrRoom *			m_pScrParent;
} GameEntity;

typedef enum IIK_t // Inventory Item Kind
{
	IIK_Coin,
	IIK_Key,

	IIK_Max
} IVNIT;

typedef struct EntityMaze_t // tag = maze
{
	FrTileWorld			m_tworld;
	FrNoteQueue			m_noteq;

	GameEntity			m_mpEntidEnt[kCEntWorldMax];
	GameEntity *		m_pGentAvatar;

	ROOMID *			m_mpXyRoomid;				// map from (xRoom, yRoom) to roomid
	GameRoom 			m_aGroom[kCRoomWorldMax];	
	GameRoom *			m_pGroomCur;

	FrRoomTransition	m_roomt;
	int					m_xScr;
	int					m_yScr;

	int					m_aCIik[IIK_Max];			// how many of each item do we have in our inventory
	bool				m_fUpdateIsDirty;
} EntityMaze;

FROG_CALL void InitEntityMaze(EntityMaze * pMaze);
FROG_CALL void UpdateEntityMaze(EntityMaze * pMaze, FrDrawContext * pDrac, FrInput * pInput, f32 dT);
