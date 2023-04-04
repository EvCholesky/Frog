#pragma once

#include "FrogRender.h"

typedef struct FrInput_t FrInput;



typedef enum TRANSK_t
{
	TRANSK_DirL,
	TRANSK_DirU,
	TRANSK_DirR,
	TRANSK_DirD,

	TRANSK_Door1,
	TRANSK_Door2,
	TRANSK_Hole,
	TRANSK_ReverseWorld,

	TRANSK_Max,
	TRANSK_Min = 0,
	TRANSK_Nil = - 1
}TRANSK;

const char * PChzFromTransk(TRANSK transk);

typedef struct RoomTransition_t // tag = rmtrans
{
	TRANSK				m_transk;
	const char *		m_pChzDest;
	int					m_dX;			// offset added to room when transitioning
	int					m_dY;
} RoomTransition;

typedef struct RoomDefinition_t // tag = rmdef
{
	const char *		m_pChzName;
	int					m_dX;
	int					m_dY;
	const char *		m_aiTile;
	RoomTransition		m_aRmtrans[6];
} RoomDefinition;

typedef struct SpriteDefinition_t //tag = spdef
{
	float				m_uMin;
	float				m_vMin;
	float				m_uMax;
	float				m_vMax;
	const char *		m_pChzName;
} SpriteDefinition;

typedef struct TileDefinition_t // tag = tiledef
{
	char				m_ch;
	FrColor				m_colFg;
	FrColor				m_colBg;
	s16					m_iSpdef;
	u8					m_grftile;
} TileDefinition;

typedef struct RoomLibrary_t // tag = rmlib
{
	RoomDefinition **	m_apRmdef;
	TileDefinition *	m_aTiledef;
	SpriteDefinition *	m_aSpdef;
} RoomLibrary;



typedef enum INVID_t // inventory id
{
	INVID_Min = 0,
	INVID_Nil = -1,
} INVID;

typedef enum INVTRAIT // Inventory Item Kind
{
	INVTRAIT_Money,
	INVTRAIT_Key,

	INVTRAIT_Max,
	INVTRAIT_Min = 0,
	INVTRAIT_Nil = -1
} INVTRAIT;

typedef struct InventoryType_t // tag = invtype
{
	const char *		m_pChzName;
	INVTRAIT			m_invtrait;

} InventoryType;

typedef struct InventoryItem_t
{
	int					m_iInvtype;
	int					m_value;
	int					m_c;			// how much of this do we have
} InventoryItem;



typedef enum RMID_t
{
	RMID_Min,
	RMID_Nil = -1
} RMID;



typedef struct GameRoom_t // tag = groom
{
	RMID				m_rmid;
	FrRoom *			m_pRoomFrog;
	RoomDefinition *	m_pRmdef;
	
	//ENTID				m_aEntid[64];			// entities belonging to this room
	//int					m_cEntid;				// number of entities belonging to this room
} GameRoom;


typedef enum EUPO_t // entity update order - lower priority updates earlier
{
	EUPO_Avatar,
	EUPO_Item,
	
} EUPO;

typedef enum ENTK_t // tag = ent
{
	ENTK_Avatar,

	ENTK_Max,
	ENTK_Min = 0,
	ENTK_Nil = -1
} ENTK;

typedef struct GameEntity_t // tag = gent
{
	ENTK				m_entk;
	ENTID				m_entid;				// tile entity id
	int					m_x;
	int					m_y;
	char				m_iTile;				// lookup into tile registry

} GameEntity;



enum { kCEntityCellMax = 20 };

typedef struct World_t // tag = maze
{
	FrTileWorld			m_tworld;
	FrNoteQueue			m_noteq;

	GameEntity			m_mpEntidEnt[kCEntWorldMax];
	GameEntity *		m_pGentAvatar;

	GameRoom *			m_mpRmidGroom;
	GameRoom *			m_pGroomCur;
	RMID				m_rmidMax;

	FrRoomTransition	m_roomt;
	int					m_xScr;
	int					m_yScr;

	//int					m_aCIik[IIK_Max];			// how many of each item do we have in our inventory
	bool				m_fUpdateIsDirty;

} World;


FROG_CALL bool FTryDumpRoomFile(const char * pChzFilename);

FROG_CALL void InitWorldMaze(World * pWorld);
FROG_CALL void UpdateWorldMaze(World * pWorld, FrDrawContext * pDrac, FrInput * pInput, f32 dT);

