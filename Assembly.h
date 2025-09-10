#pragma once
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <GC_MakeCircle.hxx>	
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopTools_ListOfShape.hxx>

struct Param {
	const char* name = nullptr;
	int vali = -1;
	float valf = -1;

	Param(){}
	Param(const char* name, int vali) : name(name), vali(vali){}
	Param(const char* name, float valf) : name(name), valf(valf){}
};

struct Assembly{

	std::vector<TopoDS_Shape> parts;
	std::vector<Param> params;

	float thickness;
	float tabWidth;
	float slotThicknessLoose;
	float slotThicknessMid;
	float slotThicknessTight;
	float slotLength;
	float backSlotLength;
	float shelvesSpacing;
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

