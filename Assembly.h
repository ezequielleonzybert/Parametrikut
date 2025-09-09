#pragma once
#include <AIS_Shape.hxx>
#include <BRep_Builder.hxx>
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

struct Assembly
{
	std::vector<TopoDS_Shape> parts;

	float thickness;
	float width;
	float depth;
	float height;
	float tabWidth;
	float slotThicknessLoose;
	float slotThicknessMid;
	float slotThicknessTight;
	float slotLength;
	float backSlotLength;
	float shelvesSpacing;
	float signHeight;

	int levels;
	int backPinsQ;
	int frontPinsQ;
	int tabs;
		
	Assembly();
	~Assembly();

	void build();
};

