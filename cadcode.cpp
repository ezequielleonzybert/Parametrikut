/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.
*/

#include "Assembly.h"

void Assembly::cadCode()
{
	gp_Trsf tr;

	BRepAlgoAPI_Cut cut;
	BRepAlgoAPI_Fuse fuse;

	TopTools_ListOfShape argList;
	TopTools_ListOfShape toolList;

	float spacing;

#pragma region Back

	// backBase
	float backBaseW = inWidth;
	float backBaseH = height;
	TopoDS_Shape backBase = BRepBuilderAPI_MakeFace(gp_Pln(), -backBaseW / 2, backBaseW / 2, -backBaseH / 2, backBaseH / 2);
	argList.Append(backBase);

	// backTabs
	float backTabW = slotThicknessLoose + tabWidth;
	float backTabH = slotLength;
	TopoDS_Shape backTab = BRepBuilderAPI_MakeFace(gp_Pln(), -backTabW / 2, backTabW / 2, -backTabH / 2, backTabH / 2);
	backTab = BRepAlgoAPI_Cut(backTab,
		BRepBuilderAPI_MakeFace(gp_Pln(), -backTabW / 2, -backTabW / 2 + slotThicknessLoose, -backTabH / 2, -backTabH / 2 + slotLength / 3));
	TopoDS_Compound backTabs;
	spacing = (sideHeight - slotLength * tabs) / (tabs - 1); //tab == 1 CRASH
	for (int i = 0; i < tabs; i++) {
		float x = inWidth / 2 + backTabW / 2;
		float y = -backBaseH / 2 + slotLength + i * spacing + slotLength / 2 - slotLength / 3;
		tr.SetTranslation(gp_Vec(x, y, 0));
		argList.Append(backTab.Located(TopLoc_Location(tr)));
	}

	// backSlots
	TopoDS_Shape backSlot = BRepBuilderAPI_MakeFace(gp_Pln(), -backSlotLength / 2, backSlotLength / 2, -slotThicknessMid / 2, slotThicknessMid / 2);
	TopoDS_Compound backSlots;
	spacing = (backBaseW - backPinsQ * backSlotLength) / (backPinsQ + 1);
	for (int i = 0; i < levels; i++) {
		float y = -backBaseH / 2 + i * shelvesSpacing + tabWidth + slotThicknessMid / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			float x = backBaseW / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
			tr.SetTranslation(gp_Vec(x, y, 0));
			toolList.Append(backSlot.Located(TopLoc_Location(tr)));
		}
	}

	fuse.SetArguments(argList);
	fuse.SetTools(argList);
	fuse.Build();
	fuse.SimplifyResult();
	TopoDS_Shape backSketch = fuse.Shape();
	argList.Clear();
	argList.Append(backSketch);

	cut.SetArguments(argList);
	cut.SetTools(toolList);
	cut.Build();
	backSketch = cut.Shape();

	TopoDS_Shape back = BRepPrimAPI_MakePrism(backSketch, gp_Vec(0, 0, thickness));

	parts.push_back(back);

#pragma endregion


}