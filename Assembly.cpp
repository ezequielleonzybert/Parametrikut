#include "Assembly.h"

Assembly::Assembly()
{
	thickness = 3.12f;
	tabWidth = 10.f;
	slotThicknessLoose = thickness + .5f;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5f;
	looseDiff = slotThicknessLoose - thickness;
	slotLength = 30.f;
	pinLength = slotLength * 1.5f;
	shelvesSpacing = 100.f;
	topShelfDepth = 70.f;
	signHeight = 50.f;

	levels = 3;
	backPinsQ = 2;
	frontPinsQ = std::min(std::max(1,backPinsQ-1),2);
	tabs = 2;

	inWidth = 200.f;
	inDepth = 100.f;

	width = inWidth + tabWidth*2 + thickness*2;
	depth = inDepth + tabWidth*2;
	height = tabWidth + shelvesSpacing * levels + thickness * levels  + signHeight;
	sideHeight = height - signHeight;

	addParam("Thickness", thickness);
	addParam("Inner width", inWidth);
	addParam("Inner depth", inDepth);
	addParam("Top shelf depth", topShelfDepth);
	addParam("Shelves spacing", shelvesSpacing);
	addParam("Tab width", tabWidth);
	addParam("Slot length", slotLength);
	addParam("Pin length", pinLength);
	addParam("Sign height", signHeight);
	addParam("Levels", levels);
	addParam("Back pins quantity", backPinsQ);
	addParam("Tabs", tabs);
}

void Assembly::build()
{
	parts.clear();

	thickness = getParamValf("Thickness");
	shelvesSpacing = getParamValf("Shelves spacing");
	tabWidth = getParamValf("Tab width");
	slotLength = getParamValf("Slot length");
	pinLength = getParamValf("Pin length");
	signHeight = getParamValf("Sign height");
	levels = getParamVali("Levels");
	backPinsQ = getParamVali("Back pins quantity");
	tabs = getParamVali("Tabs");
	inWidth = getParamValf("Inner width");
	inDepth = getParamValf("Inner depth");
	topShelfDepth = getParamValf("Top shelf depth");

	slotThicknessLoose = thickness + .5f;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5f; 

	width = inWidth + tabWidth * 2 + thickness * 2;
	depth = inDepth + tabWidth * 2;
	height = tabWidth*2 + shelvesSpacing * (levels-1) +thickness * levels + signHeight;
	sideHeight = height - signHeight;

	cadCode();
}

void Assembly::addParam(const char* name, int vali) {
	params.push_back(Param(name, vali));
}

void Assembly::addParam(const char* name, Standard_Real valf) {
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

Standard_Real Assembly::getParamValf(const char* name) {
	for (int i = 0; i < params.size(); i++) {
		if (strcmp(params[i].name, name) == 0) {
			return params[i].valf;
		}
	}
	return -1;
}