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
}

void Assembly::build()
{
	parts.clear();

	slotThicknessLoose = thickness + .5;
	slotThicknessMid = thickness;
	slotThicknessTight = thickness - .5;
	height = tabWidth + shelvesSpacing * levels + thickness * levels + signHeight;

	/* Back */ {

		float w = width - tabWidth * 2 - slotThicknessMid * 2;
		float h = height + thickness * levels;
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

Assembly::~Assembly(){}

//TopoDS_Shape aBox = BRepPrimAPI_MakeBox(300.0, 300.0, 300.0).Shape();
//gp_Trsf aTrsf;
//aTrsf.SetTranslation(gp_Vec(-150.0, -150.0, -150.0));
//BRepBuilderAPI_Transform brepTrsf(aBox, aTrsf);
//TopoDS_Shape movedBox = brepTrsf.Shape();
//Handle(AIS_Shape) aShape = new AIS_Shape(movedBox);
//parts.push_back(aBox);
//Handle(AIS_Shape) aShapeWire = new AIS_Shape(movedBox);
//parts.push_back(aShapeWire);
//TopoDS_Shape shape1 = BRepBuilderAPI_MakeFace(gp_Pln(), -50, 50, -100, 100);
//parts.push_back(shape1);
	//GC_MakeCircle c1 = GC_MakeCircle(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), 100);
	//TopoDS_Edge c1e = BRepBuilderAPI_MakeEdge(c1.Value());
	//TopoDS_Wire c1w = BRepBuilderAPI_MakeWire(c1e);
	//TopoDS_Shape c1s = BRepBuilderAPI_MakeFace(c1w);
	//GC_MakeCircle c2 = GC_MakeCircle(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), 50);
	//TopoDS_Edge c2e = BRepBuilderAPI_MakeEdge(c2.Value());
	//TopoDS_Wire c2w = BRepBuilderAPI_MakeWire(c2e);
	//TopoDS_Shape c2s = BRepBuilderAPI_MakeFace(c2w);
	//TopoDS_Shape cut = BRepAlgoAPI_Cut(c1s, c2s);
	//TopoDS_Shape prism = BRepPrimAPI_MakePrism(cut, gp_Vec(0, 0, 200));
	//parts.push_back(prism);