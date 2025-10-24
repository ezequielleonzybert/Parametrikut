#pragma once

#include "OcctUtils.h"

struct Param {
	const char* name = nullptr;
	int vali = -1;
	Standard_Real valf = -1;

	Param(){}
	Param(const char* name, int vali) : name(name), vali(vali){}
	Param(const char* name, Standard_Real valf) : name(name), valf(valf){}
};

struct Assembly{

	std::vector<TopoDS_Wire> outlines;

	std::vector<Part> parts;
	std::vector<Param> params;

	Standard_Real thickness;
	Standard_Real tabWidth;
	Standard_Real slotThicknessLoose;
	Standard_Real slotThicknessMid;
	Standard_Real slotThicknessTight;
	Standard_Real looseDiff;
	Standard_Real slotLength;
	Standard_Real pinLength;
	Standard_Real shelvesSpacing;
	Standard_Real topShelfDepth;
	Standard_Real signHeight;
	Standard_Real sideHeight;

	int levels;
	int backPinsQ;
	int frontPinsQ;
	int tabs;

	Standard_Real inWidth;
	Standard_Real inDepth;
	Standard_Real width;
	Standard_Real depth;
	Standard_Real height;
		
	Assembly();
	~Assembly(){};

	void build();

	void addParam(const char* name, int vali);
	void addParam(const char* name, Standard_Real valf);

	int getParamVali(const char* name);
	Standard_Real getParamValf(const char* name);

	void cadCode();
	Part shelf(Standard_Real h);
};

