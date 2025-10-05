#pragma once

#include "OcctUtils.h"

struct Param {
	const char* name = nullptr;
	int vali = -1;
	float valf = -1;

	Param(){}
	Param(const char* name, int vali) : name(name), vali(vali){}
	Param(const char* name, float valf) : name(name), valf(valf){}
};

struct Assembly{

	std::vector<Part> parts;
	std::vector<Param> params;

	float thickness;
	float tabWidth;
	float slotThicknessLoose;
	float slotThicknessMid;
	float slotThicknessTight;
	float slotLength;
	float backSlotLength;
	float shelvesSpacing;
	float topShelfDepth;
	float signHeight;
	float sideHeight;

	int levels;
	int backPinsQ;
	int frontPinsQ;
	int tabs;

	float inWidth;
	float inDepth;
	float width;
	float depth;
	float height;
		
	Assembly();
	~Assembly(){};

	void build();

	void addParam(const char* name, int vali);
	void addParam(const char* name, float valf);

	int getParamVali(const char* name);
	float getParamValf(const char* name);

	void cadCode();
};

