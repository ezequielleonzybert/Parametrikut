#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadcode2() {

	bool doFillet = true;
	Standard_Real f1 = 1, f2 = thickness;

#pragma region Back

	// backBase

	//BRepBuilderAPI_MakeWire mkBackBaseWire;
	//std::vector<gp_Pnt> pts;
	//std::vector<std::vector<TopoDS_Edge>> wires;
	//Standard_Real x = 0, y = 0;

	//auto addp = [&pts, &x, &y](Standard_Real a, Standard_Real b) {
	//	x += a;
	//	y += b;
	//	pts.push_back(gp_Pnt(x, y, 0));
	//	};

	//addp(-inWidth/2, -height/2);
	//addp(inWidth, 0);
	//addp(0, slotLength / 2);

	//Standard_Real ySpacing = (sideHeight - slotLength * tabs + slotLength/3*tabs) / (tabs - 1);
	//for (int i = 0; i < tabs; i++) {
	//	addp(slotThicknessLoose, 0);
	//	addp(0, -slotLength / 3);
	//	addp(tabWidth, 0);
	//	addp(0, slotLength);
	//	addp(-tabWidth - slotThicknessLoose, 0);
	//	if (i != tabs - 1) {
	//		addp(0, ySpacing);
	//	}
	//}

	//addp(0, slotLength/2 + signHeight);
	//addp(-inWidth,0);
	//addp(0, -slotLength / 2 - signHeight);

	//for (int i = 0; i - tabs; i++) {
	//	addp(-tabWidth - slotThicknessLoose, 0);
	//	addp(0, -slotLength);
	//	addp(tabWidth, 0);
	//	addp(0, slotLength / 3);
	//	addp(slotThicknessLoose, 0);
	//	if (i != tabs - 1) {
	//		addp(0, -ySpacing);
	//	}
	//}

	//std::vector<TopoDS_Edge> edges;
	//for (int i = 0; i < pts.size(); i++) {
	//	BRepBuilderAPI_MakeEdge mkEdge(pts[i], pts[(i+1)%pts.size()]);
	//	edges.push_back(mkEdge.Edge());
	//	mkBackBaseWire.Add(mkEdge.Edge());
	//}
	//wires.push_back(edges);

	//BRepBuilderAPI_MakeFace mkFace(mkBackBaseWire.Wire());

	//// backSlots
	//Standard_Real backWidth = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	//for (int i = 0; i < levels; i++) {
	//	Standard_Real y = -height / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
	//	for (int j = 0; j < backPinsQ; j++) {
	//		Standard_Real spacing = (backWidth - backPinsQ * pinLength) / (backPinsQ + 1);
	//		Standard_Real x = backWidth / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
	//		mkFace.Add(rectangleWire(slotLength, slotThicknessLoose, x, y, true));
	//	}
	//}

	//TopoDS_Face face(mkFace.Face());

	//if (!doFillet)
	//{
	//	std::vector<TopoDS_Vertex> vv1;
	//	std::vector<TopoDS_Vertex> vv2;
	//	std::vector<TopoDS_Vertex> vvSign;

	//	auto gx = groupBy(vertices(face), Axis::x);
	//	auto gy = groupBy(vertices(face), Axis::y);

	//	sortBy(gx[1], Axis::y);
	//	for (int i = 0; i < gx[1].size(); i++) {
	//		if (i % 2 == 0) {
	//			vv1.push_back(gx[1][i]);
	//		}
	//	}
	//	sortBy(gx[gx.size() - 2], Axis::y);
	//	for (int i = 0; i < gx[gx.size() - 2].size(); i++) {
	//		if (i % 2 == 0) {
	//			vv1.push_back(gx[gx.size() - 2][i]);
	//		}
	//	}

	//	vv2.insert(vv2.end(), gx[0].begin(), gx[0].end());
	//	vv2.insert(vv2.end(), gx[gx.size() - 1].begin(), gx[gx.size() - 1].end());
	//	vv2.insert(vv2.end(), gy[0].begin(), gy[0].end());
	//	vvSign.insert(vvSign.end(), gy[gy.size() - 1].begin(), gy[gy.size() - 1].end());

	//	face = fillet2(wires, vv1, thickness / 3);
	//	face = fillet2(wires, vv2, thickness);
	//	face = fillet2(wires, vvSign, thickness);
	//}

	//BRepPrimAPI_MakePrism mkPrism(face, gp_Vec(0, 0, thickness));
	//TopoDS_Shape back = mkPrism.Shape();

	//--------------------

	BuildingTool bt(-inWidth / 2, -height / 2);
	bt.lineTo(inWidth, 0, f2);
	bt.lineTo(0, slotLength / 2);

	Standard_Real ySpacing = (sideHeight - slotLength * tabs + slotLength / 3 * tabs) / (tabs - 1);
	for (int i = 0; i < tabs; i++) {
		bt.lineTo(slotThicknessLoose, 0);
		bt.lineTo(0, -slotLength / 3, f1);
		bt.lineTo(tabWidth, 0, f2);
		bt.lineTo(0, slotLength, f2);
		bt.lineTo(-tabWidth - slotThicknessLoose, 0);
		if (i != tabs - 1) {
			bt.lineTo(0, ySpacing);
		}
	}

	bt.lineTo(0, slotLength / 2 + signHeight, f2);
	bt.lineTo(-inWidth, 0, f2);
	bt.lineTo(0, -slotLength / 2 - signHeight);

	for (int i = 0; i - tabs; i++) {
		bt.lineTo(-tabWidth - slotThicknessLoose, 0, f2);
		bt.lineTo(0, -slotLength, f2);
		bt.lineTo(tabWidth, 0, f1);
		bt.lineTo(0, slotLength / 3);
		bt.lineTo(slotThicknessLoose, 0);
		if (i != tabs - 1) {
			bt.lineTo(0, -ySpacing);
		}
	}

	bt.close(f2);

	Standard_Real backWidth = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	for (int i = 0; i < levels; i++) {
		Standard_Real y = -height / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			Standard_Real spacing = (backWidth - backPinsQ * pinLength) / (backPinsQ + 1);
			Standard_Real x = backWidth / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
			bt.rectangle(slotLength, slotThicknessLoose, x, y);
		}
	}

	bt.build(thickness);

#pragma endregion

	parts.push_back(bt.prism);

}