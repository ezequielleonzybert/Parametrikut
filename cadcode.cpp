/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.
*/

#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadCode()
{
	BRepAlgoAPI_Cut cut;
	BRepAlgoAPI_Fuse fuse;
	cut.SetRunParallel(true);
	fuse.SetRunParallel(true);

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
		vec pos(
			inWidth / 2 + backTabW / 2,
			-backBaseH / 2 + slotLength + i * spacing + slotLength / 2 - slotLength / 3
		);

		TopoDS_Shape translated = translate(backTab, pos);
		argList.Append(translated);
		argList.Append(mirror( translated, vec(0,0,0), vec(1, 0, 0) ));
	}

	// backSlots
	TopoDS_Shape backSlot = BRepBuilderAPI_MakeFace(gp_Pln(), -backSlotLength / 2, backSlotLength / 2, -slotThicknessMid / 2, slotThicknessMid / 2);
	TopoDS_Compound backSlots;
	spacing = (backBaseW - backPinsQ * backSlotLength) / (backPinsQ + 1);
	for (int i = 0; i < levels; i++) {
		vec pos;
		pos.y = -backBaseH / 2 + i * shelvesSpacing + tabWidth + slotThicknessMid / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			pos.x = backBaseW / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
			toolList.Append(translate(backSlot, pos));
		}
	}

	fuse.SetArguments(argList);
	fuse.SetTools(argList);
	fuse.Build();
	fuse.SimplifyResult();
	TopoDS_Shape fused = fuse.Shape();
	argList.Clear();
	argList.Append(fused);
	cut.SetArguments(argList);
	cut.SetTools(toolList);
	cut.Build();
	TopoDS_Shape backSketch = cut.Shape();

	TopoDS_Shape back = BRepPrimAPI_MakePrism(backSketch, gp_Vec(0, 0, thickness));

	parts.push_back(back);

#pragma endregion


}