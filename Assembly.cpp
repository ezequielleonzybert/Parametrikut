#include "Assembly.h"

Assembly::Assembly()
{
	thickness = 3;
	tabWidth = 10;
	slotLength = 30;
	pinLength = slotLength * 1.5;
	shelvesSpacing = 100;
	topShelfDepth = 50;
	botShelfDepth = 100;
	signHeight = 50;

	levels = 3;
	backPinsQ = 2;
	tabs = 2;

	inWidth = 200;

	recalculateParams();

	addParam("Thickness", thickness);
	addParam("Inner width", inWidth);
	addParam("Top shelf depth", topShelfDepth);
	addParam("Bottom shelf depth", botShelfDepth);
	addParam("Shelves spacing", shelvesSpacing);
	addParam("Tab width", tabWidth);
	addParam("Slot length", slotLength);
	addParam("Pin length", pinLength);
	addParam("Sign height", signHeight);
	addParam("Levels", levels);
	addParam("Back pins", backPinsQ);
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
	backPinsQ = getParamVali("Back pins");
	tabs = getParamVali("Tabs");
	inWidth = getParamValf("Inner width");
	topShelfDepth = getParamValf("Top shelf depth");
	botShelfDepth = getParamValf("Bottom shelf depth");

	recalculateParams();

	cadcode2();
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

void Assembly::recalculateParams() {
	slotThicknessLoose = thickness + .5;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5;
	railingHeight = thickness * 4;
	looseDiff = slotThicknessLoose - thickness;
	width = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	height = tabWidth + shelvesSpacing * (levels - 1) + thickness * levels + railingHeight + signHeight;
	sideHeight = height - signHeight;
}