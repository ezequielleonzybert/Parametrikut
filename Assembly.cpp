#include "Assembly.h"

Assembly::Assembly()
{
	thickness = 3.12;
	tabWidth = 10;
	slotThicknessLoose = thickness + .5;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5;
	slotLength = 30;
	backSlotLength = slotLength * 1.5;
	shelvesSpacing = 100;
	signHeight = 70;

	levels = 3;
	backPinsQ = 2;
	frontPinsQ = 1;
	tabs = 2;

	width = 200;
	depth = 200;
	height = tabWidth + shelvesSpacing * levels + thickness * levels  + signHeight;

	/* addParams */ {
		addParam("Thickness", thickness);
		addParam("Shelves spacing", shelvesSpacing);
		addParam("Tab width", tabWidth);
		addParam("Slot length", slotLength);
		addParam("Back slot length", backSlotLength);
		addParam("Sign height", signHeight);

		addParam("Levels", levels);
		addParam("Front pins quantity", frontPinsQ);
		addParam("Back pins quantity", backPinsQ);
		addParam("Tabs", tabs);

		addParam("Width", width);
		addParam("Depth", depth);
	}
}

void Assembly::build()
{
	parts.clear();

	thickness = getParamValf("Thickness");
	shelvesSpacing = getParamValf("Shelves spacing");
	tabWidth = getParamValf("Tab width");
	slotLength = getParamValf("Slot length");
	backSlotLength = getParamValf("Back slot length");
	signHeight = getParamValf("Sign height");
	levels = getParamVali("Levels");
	frontPinsQ = getParamVali("Front pins quantity");
	backPinsQ = getParamVali("Back pins quantity");
	tabs = getParamVali("Tabs");
	width = getParamValf("Width");
	depth = getParamValf("Depth");

	slotThicknessLoose = thickness + .5;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5;
	height = tabWidth + shelvesSpacing * levels + thickness * levels + signHeight;
	
	/* Back */ {

		float w = width - tabWidth * 2 - slotThicknessMid * 2;
		float h = height;
		TopoDS_Shape backBase = BRepBuilderAPI_MakeFace(gp_Pln(), -w / 2, w / 2, -h / 2, h / 2);

		TopoDS_Shape backSlot = BRepBuilderAPI_MakeFace(gp_Pln(), -slotLength / 2, slotLength / 2, -slotThicknessMid / 2, slotThicknessMid / 2);
		TopoDS_Compound backSlots;
		BRep_Builder builder;
		builder.MakeCompound(backSlots);
		float spacing = (w - backPinsQ * backSlotLength) / (backPinsQ + 1);
		gp_Trsf transform;
		for (int i = 0; i < levels; i++) {
			float y = -h / 2 + i * shelvesSpacing + tabWidth + slotThicknessMid / 2 + thickness * i;
			for (int j = 0; j < backPinsQ; j++) {
				float x = w / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
				transform.SetTranslation(gp_Vec(x, y, 0));
				builder.Add(backSlots, backSlot.Located(TopLoc_Location(transform)));
			}
		}
		TopoDS_Shape backSketch = BRepAlgoAPI_Cut(backBase, backSlots);
		TopoDS_Shape back = BRepPrimAPI_MakePrism(backSketch, gp_Vec(0, 0, thickness));

		parts.push_back(back);
	}
}

void Assembly::addParam(const char* name, int vali) {
	params.push_back(Param(name, vali));
}

void Assembly::addParam(const char* name, float valf) {
	params.push_back(Param(name, valf));
}

int Assembly::getParamVali(const char* name) {
	for (int i = 0; i < params.size(); i++) {
		if (strcmp(params[i].name, name) == 0) {
			return params[i].vali;
		}
	}
	return -1;
}

float Assembly::getParamValf(const char* name) {
	for (int i = 0; i < params.size(); i++) {
		if (strcmp(params[i].name, name) == 0) {
			return params[i].valf;
		}
	}
	return -1;
}