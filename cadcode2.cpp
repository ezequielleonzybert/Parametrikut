#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadcode2() {

	bool doFillet = true;
	Standard_Real f1 = 1, f2 = thickness;

#pragma region Back

	// outer Wire
	BuildingTool btBack(-inWidth / 2, 0);
	btBack.lineTo(inWidth, 0, f2);
	btBack.lineTo(0, slotLength / 2);

	Standard_Real ySpacing = (sideHeight - slotLength * tabs + slotLength / 3 * tabs) / (tabs - 1);
	for (int i = 0; i < tabs; i++) {
		btBack.lineTo(slotThicknessLoose, 0);
		btBack.lineTo(0, -slotLength / 3, f1);
		btBack.lineTo(tabWidth, 0, f2);
		btBack.lineTo(0, slotLength, f2);
		btBack.lineTo(-tabWidth - slotThicknessLoose, 0);
		if (i != tabs - 1) {
			btBack.lineTo(0, ySpacing);
		}
	}

	btBack.lineTo(0, slotLength / 2 + signHeight, f2);
	btBack.lineTo(-inWidth, 0, f2);
	btBack.lineTo(0, -slotLength / 2 - signHeight);

	for (int i = 0; i - tabs; i++) {
		btBack.lineTo(-tabWidth - slotThicknessLoose, 0, f2);
		btBack.lineTo(0, -slotLength, f2);
		btBack.lineTo(tabWidth, 0, f1);
		btBack.lineTo(0, slotLength / 3);
		btBack.lineTo(slotThicknessLoose, 0);
		if (i != tabs - 1) {
			btBack.lineTo(0, -ySpacing);
		}
	}
	btBack.close(f2);

	// backSlots
	std::vector<gp_Pnt> backSlotsLocs;
	Standard_Real backWidth = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	for (int i = 0; i < levels; i++) {
		Standard_Real y = tabWidth + i * shelvesSpacing + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			Standard_Real spacing = (backWidth - backPinsQ * pinLength) / (backPinsQ + 1);
			Standard_Real x = backWidth / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
			btBack.rectangle(pinLength, slotThicknessLoose, x, y);
			backSlotsLocs.push_back(gp_Pnt(x, y, 0));
		}
	}

	btBack.build(thickness);

#pragma endregion

#pragma region Lateral

	// Outer Wire
	BuildingTool btLateral;

	// Bottom Front Slot, needed to calculate the lateral total depth and ellipse rx:
	Standard_Real latFrontSlotX = tabWidth + thickness * 1.5 + botShelfDepth;
	Standard_Real latFrontSlotY = backSlotsLocs[0].Y() + slotLength / 2 + thickness * 1.5;
	Standard_Real latFrontSlotW = slotThicknessLoose;
	Standard_Real latFrontSLotH = slotLength;
	btLateral.rectangle(latFrontSlotW, latFrontSLotH, latFrontSlotX, latFrontSlotY);


	//gp_Pnt pointInEllipse(
	//	latFrontSlotX + latFrontSlotW/2,
	//	latFrontSlotY + latFrontSLotH/2,
	//	0
	//);

	//Standard_Real ellipseRadY = height;
	//Standard_Real ellipseRadX = abs(pointInEllipse.X()) * sqrt((ellipseRadY * ellipseRadY) / ((ellipseRadY * ellipseRadY) - (pointInEllipse.Y()* pointInEllipse.Y())));

	//depth = tabWidth + thickness * 2 + topShelfDepth + ellipseRadX;

	//// Ellipse arcs and shelves slides
	//btLateral.moveTo(0, 0);
	//btLateral.lineTo(depth, 0);

	////Standard_Real rx = (depth - topShelfDepth); //more or less
	////Standard_Real ry = height;
	//gp_Pnt center(btLateral.getCurrentPos().Translated({-ellipseRadX,0,0}));
	//Ellipse2 ellipse(center, ellipseRadX, ellipseRadY);

	////for (int i = 0; i < levels; i++) {

	//	Standard_Real y = center.Y() + backSlotsLocs[0].Y() - slotThicknessLoose / 2;
	//	Standard_Real x = center.X() + ellipse.getRadiusX(y);
	//	btLateral.arcTo(x, y, ellipseRadX, ellipseRadY, center);

	////}

	btLateral.build();


#pragma endregion

	//parts.push_back(btBack.prism);
	parts.push_back(btLateral.wires[0]);

	//edges = btBack.edges;

}