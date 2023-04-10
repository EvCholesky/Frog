#include "Editor.h"
#include "FrogInput.h"

#include "imgui.h"

FROG_CALL Editor * PEditorStaticInit()
{
	static Editor s_editor;
	s_editor.m_edgesSpace = EDGES_Off;
	s_editor.m_edtoolkCur = EDTOOLK_TileBrush;

	return &s_editor;
}

FROG_CALL void UpdateInputEditor(World * pWorld, FrInput * pInput)
{
	Frog_PollInput(pInput);

	Editor * pEditor = pWorld->m_pEditor;
	FrInputEventIterator inevit = Frog_Inevit(pInput->m_pInevfifo);
	FrInputEvent * pInev;
	while (pInev = Frog_PInevNext(&inevit))
	{
		switch(pInev->m_eventk)	
		{
		case EVENTK_MousePos:
			{
				
			}
			break;

		case EVENTK_Keyboard:
			{
				if (pInev->m_keycode == KEYCODE_Space)
				{
					pEditor->m_edgesSpace = pInev->m_edges;
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


void SetEditorTool(Editor * pEditor, EDTOOLK edtoolk)
{
	pEditor->m_edtoolkCur = edtoolk;
}

FROG_CALL void UpdateEditorWindow(World * pWorld)
{
	bool fShowImguiDemoWindow = true;

	if (fShowImguiDemoWindow)
	{
		ImGui::ShowDemoWindow(&fShowImguiDemoWindow);
	}

	Editor * pEditor = pWorld->m_pEditor;
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

	int edtoolk = (int)pEditor->m_edtoolkCur;
	ImGui::RadioButton("Tile Brush", &edtoolk, (int)EDTOOLK_TileBrush);
	ImGui::SameLine();
	ImGui::RadioButton("Entity Placement", &edtoolk, (int)EDTOOLK_EntityPlacement);
	SetEditorTool(pEditor, (EDTOOLK)edtoolk);

    if (ImGui::CollapsingHeader("Tool Options", NULL, ImGuiTreeNodeFlags_DefaultOpen))
    {
	}

    if (ImGui::CollapsingHeader("Room", NULL, ImGuiTreeNodeFlags_DefaultOpen))
    {
		GameRoom * pGroomCur = pWorld->m_pGroomCur;
		ImGui::Text(pGroomCur->m_pRmdef->m_pChzName);

		ImGui::Separator();
		RoomDefinition * pRmdef = pGroomCur->m_pRmdef;

		char  aChRoomDest[kCChRoomNameMax];

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
