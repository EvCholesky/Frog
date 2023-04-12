#pragma once

#include "WorldMaze.h"

typedef enum EDTOOLK_t // EDitor TOOL Kind
{
	EDTOOLK_TileBrush,
	EDTOOLK_EntityPlacement,

	EDTOOLK_Max,
	EDTOOLK_Min = 0,
	EDTOOLK_Nil = -1,
} EDTOOLK;

typedef enum CURSORS_t // CURSOR State 
{
	CURSORS_Tool,
	CURSORS_GrabPan,

	CURSORS_Max,
	CURSORS_Min = 0,
	CURSORS_Nil = -1,
} CURSORS;

typedef struct TileBrush_t // tag = tbr
{
	int					m_iTile;
} TileBrush;

FROG_CALL void InitTileBrush(TileBrush * pTbr);

typedef struct Editor_t // tag = edit
{
	char				m_aChRoom[kCChRoomNameMax];

	TileBrush			m_tbr;
	EDTOOLK				m_edtoolkCur;
	EDGES				m_edgesSpace;		// Is the user pressing space (used to pan the camera)

	FrVec2				m_posCursor;
	FrVec2				m_posCursorGrab;	// start position of a grab-pan interaction

} Editor;

FROG_CALL Editor * PEditStaticInit();

FROG_CALL void UpdateEditorWindow(World * pWorld);
FROG_CALL void UpdateInputEditor(World * pWorld, FrInput * pInput);
