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
	signHeight = 50;

	levels = 3;
	backPinsQ = 2;
	frontPinsQ = 1;
	tabs = 2;

	inWidth = 200;
	inDepth = 100;

	width = inWidth + tabWidth*2 + thickness*2;
	depth = inDepth + tabWidth*2;
	height = tabWidth + shelvesSpacing * levels + thickness * levels  + signHeight;
	sideHeight = height - signHeight;

	rotation = 0;

	/* addParams */ {
		addParam("Thickness", thickness);
		addParam("Inner width", inWidth);
		addParam("Inner depth", inDepth);
		addParam("Shelves spacing", shelvesSpacing);
		addParam("Tab width", tabWidth);
		addParam("Slot length", slotLength);
		addParam("Back slot length", backSlotLength);
		addParam("Sign height", signHeight);

		addParam("Levels", levels);
		addParam("Front pins quantity", frontPinsQ);
		addParam("Back pins quantity", backPinsQ);
		addParam("Tabs", tabs);

		addParam("Rotation", rotation);
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
	inWidth = getParamValf("Inner width");
	inDepth = getParamValf("Inner depth");

	rotation = getParamValf("Rotation");

	slotThicknessLoose = thickness + .5;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5; 

	width = inWidth + tabWidth * 2 + thickness * 2;
	depth = inDepth + tabWidth * 2;
	height = tabWidth*2 + shelvesSpacing * (levels-1) +thickness * levels + signHeight;
	sideHeight = height - signHeight;

	cadCode();
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