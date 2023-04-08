#pragma once

#include "WorldMaze.h"

typedef enum EDTOOLK_t
{
	EDTOOLK_TileBrush,
	EDTOOLK_EntityPlacement,
	EDTOOLK_Max,
	EDTOOLK_Min = 0,
	EDTOOLK_Nil = -1,
} EDTOOLK;

typedef struct Editor_t
{
	char				m_aChRoom[kCChRoomNameMax];

	EDTOOLK				m_edtoolkCur;
	EDGES				m_edgesSpace;		// Is the user pressing space (used to pan the camera)

} Editor;

FROG_CALL Editor * PEditorStaticInit();

FROG_CALL void UpdateEditorWindow(World * pWorld);
FROG_CALL void UpdateInputEditor(World * pWorld, FrInput * pInput);
