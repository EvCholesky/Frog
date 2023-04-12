#include "Editor.h"
#include "FrogInput.h"

#include "imgui.h"

extern "C" RoomLibrary s_rmlib;

FROG_CALL void InitTileBrush(TileBrush * pTbr)
{
	pTbr->m_iTile = 1;
}

FROG_CALL Editor * PEditStaticInit()
{
	static Editor s_edit;
	s_edit.m_edgesSpace = EDGES_Off;
	s_edit.m_edtoolkCur = EDTOOLK_TileBrush;
	InitTileBrush(&s_edit.m_tbr);

	return &s_edit;
}

FROG_CALL void UpdateInputEditor(World * pWorld, FrInput * pInput)
{
	Frog_PollInput(pInput);

	Editor * pEdit = pWorld->m_pEdit;
	FrInputEventIterator inevit = Frog_Inevit(pInput->m_pInevfifo);
	FrInputEvent * pInev;
	while (pInev = Frog_PInevNext(&inevit))
	{
		switch(pInev->m_eventk)	
		{
		case EVENTK_MousePos:
			{
				pEdit->m_posCursor = Frog_Vec2Create(pInev->m_x, pInev->m_y);
			}
			break;

		case EVENTK_MousePress:
			{
				 ImGuiIO& io = ImGui::GetIO();
				 if (io.WantCaptureMouse)
					 break;

				if (pEdit->m_edgesSpace >= EDGES_Press)
				{
					
				}
				else
				{
					if (pInev->m_edges == EDGES_Press)
					{
						//FrVec2 posOrigin = Frog_VecCreate(0.0f, 0.0f);
						//pos = PosWinToRm(&pWorld->m_tworld.m_cam, &posOrigin);
						int xCell, yCell;
						GameRoom * pGroom = pWorld->m_pGroomCur;
						FrRoom * pRoomFrog = pWorld->m_pGroomCur->m_pRoomFrog;
						Frog_FindCellFromPosSc(&pWorld->m_tworld.m_cam, pRoomFrog, &pEdit->m_posCursor, &xCell, &yCell);
	
						if (xCell >= 0 && xCell < pRoomFrog->m_dX && 
							yCell >= 0 && yCell < pRoomFrog->m_dY)
						{
							char ch = s_rmlib.m_aTiledef[pEdit->m_tbr.m_iTile].m_ch;

							int iCell = xCell + (pRoomFrog->m_dY - 1 - yCell) * pRoomFrog->m_dX;
							pGroom->m_pRmdef->m_aiTile[iCell] = pEdit->m_tbr.m_iTile;

							Frog_SetRoomTile(pRoomFrog, &pWorld->m_tworld.m_tmap, xCell, yCell, pEdit->m_tbr.m_iTile);
						}
					}
				}
			}
			break;

		case EVENTK_Keyboard:
			{
				if (pInev->m_keycode == KEYCODE_Space)
				{
					pEdit->m_edgesSpace = pInev->m_edges;
					if (pEdit->m_edgesSpace == EDGES_Press)
					{
						pEdit->m_posCursorGrab = pEdit->m_posCursor;
					}
					else if (pEdit->m_edgesSpace == EDGES_Release)
					{
						FrCamera * pCam = &pWorld->m_tworld.m_cam;
						FrVec2 dPos = Frog_Vec2Sub(&pEdit->m_posCursorGrab, &pEdit->m_posCursor);
						pCam->m_posRm = Frog_Vec2Add(&pCam->m_posRm, &dPos);
						pCam->m_posRmDesired = pCam->m_posRm;

					}
				}

				if (pInev->m_edges != EDGES_Press)
					continue;

				switch (pInev->m_keycode)
				{
				case KEYCODE_S:
					if (pInev->m_finev & FINEV_Ctrl)
					{
						SaveWorldInline(pWorld);
					}
					break;

				case KEYCODE_F1:
					SetGameMode(pWorld, GAMEMODE_Play);
					break;
				}
			}
			break;
		}
	}
	Frog_ClearInputEvents(pInput->m_pInevfifo);
}


void SetEditorTool(Editor * pEdit, EDTOOLK edtoolk)
{
	pEdit->m_edtoolkCur = edtoolk;
}

FROG_CALL void UpdateEditorWindow(World * pWorld)
{
	bool fShowImguiDemoWindow = true;
	if (fShowImguiDemoWindow)
	{
		ImGui::ShowDemoWindow(&fShowImguiDemoWindow);
	}

	Editor * pEdit = pWorld->m_pEdit;
	bool fEditorActive = true;
	ImGui::Begin("Editor", &fEditorActive, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Room", NULL))   
			{ 

			}
			if (ImGui::BeginMenu("Load Room"))
			{
				for (int rmid = 0; rmid < pWorld->m_rmidMax; ++rmid)
				{
					GameRoom * pGroomIt = &pWorld->m_mpRmidGroom[rmid];
					if (ImGui::MenuItem(pGroomIt->m_pRmdef->m_pChzName))
					{
						CutToRoom(pWorld, pGroomIt);	
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))   
			{ 
				SaveWorldInline(pWorld);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl+Z"))   
			{ 

			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y"))   
			{ 

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Resize room", NULL))   
			{ 

			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (!fEditorActive)
	{
		SetGameMode(pWorld, GAMEMODE_Play);
	}

	int edtoolk = (int)pEdit->m_edtoolkCur;
	ImGui::RadioButton("Tile Brush", &edtoolk, (int)EDTOOLK_TileBrush);
	ImGui::SameLine();
	ImGui::RadioButton("Entity Placement", &edtoolk, (int)EDTOOLK_EntityPlacement);
	SetEditorTool(pEdit, (EDTOOLK)edtoolk);

    if (ImGui::CollapsingHeader("Tool Options", NULL, ImGuiTreeNodeFlags_DefaultOpen))
    {
		int xCell, yCell;
		GameRoom * pGroom = pWorld->m_pGroomCur;
		FrRoom * pRoomFrog = pWorld->m_pGroomCur->m_pRoomFrog;
		Frog_FindCellFromPosSc(&pWorld->m_tworld.m_cam, pRoomFrog, &pEdit->m_posCursor, &xCell, &yCell);
		ImGui::Text("Cursor (%d, %d)", xCell, yCell);

        ImTextureID texid = (ImTextureID)(intptr_t)pWorld->m_tworld.m_pTexSprite->m_driId;

		int iButton = 0;
		for (int iTiledef = 0; ; ++iTiledef)
		{
			TileDefinition * pTiledef = &s_rmlib.m_aTiledef[iTiledef];
			if (pTiledef->m_ch == '\0')
				break;

			if (pTiledef->m_iSpdef < 0)
				continue;

			if ((iButton % 6) != 0)
			{
				ImGui::SameLine();
			}
			++iButton;

			SpriteDefinition * pSpdef = &s_rmlib.m_aSpdef[pTiledef->m_iSpdef];

            ImVec2 size = ImVec2(64.0f, 64.0f);                         // Size of the image we want to make visible
            ImVec2 uv0 = ImVec2(pSpdef->m_uMin, pSpdef->m_vMax);        // UV coordinates for lower-left
            ImVec2 uv1 = ImVec2(pSpdef->m_uMax, pSpdef->m_vMin);		// UV coordinates for (32,32) in our texture
            ImVec4 colBg = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
            ImVec4 colTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
            ImGui::PushID(pTiledef);
            if (ImGui::ImageButton("", texid, size, uv0, uv1, colBg, colTint))
			{
				pEdit->m_tbr.m_iTile = iTiledef;
			}
			ImGui::PopID();
		}
	}

    if (ImGui::CollapsingHeader("Room", NULL, ImGuiTreeNodeFlags_DefaultOpen))
    {
		GameRoom * pGroomCur = pWorld->m_pGroomCur;
		ImGui::Text(pGroomCur->m_pRmdef->m_pChzName);

		ImGui::Separator();
		RoomDefinition * pRmdef = pGroomCur->m_pRmdef;

		char aChRoomDest[kCChRoomNameMax];

		if (ImGui::TreeNodeEx("Transitions", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int iRmtrans = 0; ; ++iRmtrans)
			{
				RoomTransition * pRmtrans = &pRmdef->m_aRmtrans[iRmtrans];
				if (pRmtrans->m_transk == TRANSK_Nil)
					break;

				static float s_dXDestRoom = 120.0f;
				static float s_dXKind = 100.0f;
				static float s_dXOffset = 150.0f;

				ImGui::PushID(pRmtrans);

				strcpy_s(aChRoomDest, FR_DIM(aChRoomDest), pRmtrans->m_pChzDest);
				ImGui::SetNextItemWidth(s_dXDestRoom);
				ImGui::InputText("Dest", aChRoomDest, FR_DIM(aChRoomDest));
				ImGui::SameLine(0, 20.0f);

				int iTransk = (int)pRmtrans->m_transk;
				const char ** pMpTranskPChz = PMpTranskPChz();
				ImGui::SetNextItemWidth(s_dXKind);
				ImGui::Combo("kind", &iTransk, pMpTranskPChz, TRANSK_Max);
				ImGui::SameLine(0, 20.0f);

				int dXy[2] = {pRmtrans->m_dX, pRmtrans->m_dY};
				ImGui::SetNextItemWidth(s_dXOffset);
				ImGui::InputInt2("offset", dXy);
				pRmtrans->m_dX = dXy[0];
				pRmtrans->m_dY = dXy[1];
			
				ImGui::PopID();
			}

			if (ImGui::Button("Add Transition"))
			{

			}

			ImGui::TreePop();
		}
	}


	ImGui::End();
}
