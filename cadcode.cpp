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

	TopTools_ListOfShape args;
	TopTools_ListOfShape tools;
	args.Clear();
	tools.Clear();

	float spacing;

//#pragma region Back
//
//	// backBase
//	float backBaseW = inWidth;
//	float backBaseH = height;
//	TopoDS_Face backBase = BRepBuilderAPI_MakeFace(gp_Pln(), -backBaseW / 2, backBaseW / 2, -backBaseH / 2, backBaseH / 2);
//	args.Append(backBase);
//
//	// backTabs
//	float backTabW = slotThicknessLoose + tabWidth;
//	float backTabH = slotLength;
//	TopoDS_Shape backTab = BRepBuilderAPI_MakeFace(gp_Pln(), -backTabW / 2, backTabW / 2, -backTabH / 2, backTabH / 2);
//	backTab = BRepAlgoAPI_Cut(backTab,
//		BRepBuilderAPI_MakeFace(gp_Pln(), -backTabW / 2, -backTabW / 2 + slotThicknessLoose, -backTabH / 2, -backTabH / 2 + slotLength / 3));
//	TopoDS_Compound backTabs;
//	spacing = (sideHeight - slotLength * tabs) / (tabs - 1); //tab == 1 CRASH
//	for (int i = 0; i < tabs; i++) {
//		vec pos(
//			inWidth / 2 + backTabW / 2,
//			-backBaseH / 2 + slotLength + i * spacing + slotLength / 2 - slotLength / 3
//		);
//
//		TopoDS_Shape translated = translate(backTab, pos);
//		args.Append(translated);
//		args.Append(mirror( translated, vec(1, 0, 0)));
//	}
//
//	// backSlots
//	TopoDS_Shape backSlot = BRepBuilderAPI_MakeFace(gp_Pln(), -backSlotLength / 2, backSlotLength / 2, -slotThicknessMid / 2, slotThicknessMid / 2);
//	TopoDS_Compound backSlots;
//	spacing = (backBaseW - backPinsQ * backSlotLength) / (backPinsQ + 1);
//	for (int i = 0; i < levels; i++) {
//		vec pos;
//		pos.y = -backBaseH / 2 + i * shelvesSpacing + tabWidth + slotThicknessMid / 2 + thickness * i;
//		for (int j = 0; j < backPinsQ; j++) {
//			pos.x = backBaseW / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
//			tools.Append(translate(backSlot, pos));
//		}
//	}
//
//	fuse.SetArguments(args);
//	fuse.SetTools(args);
//	fuse.Build();
//	fuse.SimplifyResult();
//	TopoDS_Shape fused = fuse.Shape();
//	args.Clear();
//	args.Append(fused);
//	cut.SetArguments(args);
//	cut.SetTools(tools);
//	cut.Build();
//	TopoDS_Shape backSketch = cut.Shape();
//
//	TopExp_Explorer edges(backSketch, TopAbs_EDGE);
//	//BRepFilletAPI_MakeFillet filleted(backSketch);
//
//	TopoDS_Shape back = BRepPrimAPI_MakePrism(backSketch, gp_Vec(0, 0, thickness));
//
//	parts.push_back(back);
//
//#pragma endregion


	// backBase
	Rectangle backBase(inWidth + (thickness-slotThicknessLoose), height);
	args.Append(backBase);

	// backTabs
	Rectangle backTabBase(slotThicknessLoose + tabWidth, slotLength, Align::hh);
	Rectangle backTabCut(slotThicknessLoose, slotLength/3 ,Align::hh);
	for (int i = 0; i < tabs; i++) {
		float x = backBase.w / 2;
		float ySpacing = (sideHeight - slotLength * 2) / (tabs - 1);
		float y = -backBase.h/2 + slotLength/2 + i*ySpacing;

		TopoDS_Shape add = translate(backTabBase, vec(x, y));
		TopoDS_Shape sub = translate(backTabCut, vec(x, y));

		args.Append(add);
		tools.Append(sub);
		args.Append(mirror(add, vec(1, 0, 0)));
		tools.Append(mirror(sub, vec(1, 0, 0)));
	}

	parts.push_back(fusecut(args, tools));
}