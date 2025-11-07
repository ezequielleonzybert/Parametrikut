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
	Standard_Real slotEdgeSpacing = thickness * 2;

	BuildingTool btLateral;

	// Calculate Bottom Front Slot, needed to calculate the lateral total depth and ellipse rx:
	Standard_Real latFrontSlotX = tabWidth + looseDiff/2 + slotEdgeSpacing + botShelfDepth;
	Standard_Real latFrontSlotY = backSlotsLocs[0].Y() + slotLength / 2 + thickness * 1.5;
	Standard_Real latFrontSlotW = slotThicknessLoose;
	Standard_Real latFrontSLotH = slotLength;

	// ellipse must pass through this point.
	gp_Pnt recTopRight(
		latFrontSlotX + latFrontSlotW/2 + slotEdgeSpacing,
		latFrontSlotY + latFrontSLotH/2,
		0
	);

	Standard_Real ellipseRadY = sideHeight;	
	Standard_Real ery2 = ellipseRadY * ellipseRadY;
	Standard_Real px = recTopRight.X() - (topShelfDepth + tabWidth + looseDiff + thickness * 2);
	Standard_Real px2 = px*px;
	Standard_Real py2 = recTopRight.Y() * recTopRight.Y();
	Standard_Real ellipseRadX = abs(px) * sqrt(ery2 / (ery2 - py2));

	depth = topShelfDepth + tabWidth + looseDiff + thickness * 2 + ellipseRadX;

	// Ellipse arcs and shelves slides
	btLateral.moveTo(0, 0);
	btLateral.lineTo(depth, 0, f2);

	gp_Pnt center(btLateral.getCurrentPos().Translated({-ellipseRadX,0,0}));
	Ellipse2 ellipse(center, ellipseRadX, ellipseRadY);

	std::vector<gp_Pnt> lateralFrontSlotsLocs;
	Standard_Real lastSlideTopY;
	for (int i = 0; i < levels; i++) {

		Standard_Real y = center.Y() + backSlotsLocs[i*2].Y() - slotThicknessLoose / 2;
		Standard_Real x = center.X() + ellipse.getRadiusX(y);
		btLateral.arcTo(x, y, ellipseRadX, ellipseRadY, center, f1);
	
		btLateral.LineTo(tabWidth + looseDiff / 2 + thickness + botShelfDepth / 2, y); //wrong
		btLateral.lineTo(0, slotThicknessLoose);
		y = btLateral.getCurrentPos().Y();
		x = center.X() + ellipse.getRadiusX(y);
		btLateral.LineTo(x, y, f1);
		if (i == levels - 1) lastSlideTopY = y;

		// prepare front slots locations for next step
		Standard_Real slotCornerY = y + slotEdgeSpacing + slotLength;
		x = center.X() + ellipse.getRadiusX(slotCornerY) - slotEdgeSpacing;
		y += slotEdgeSpacing + slotLength / 2;
		lateralFrontSlotsLocs.push_back(gp_Pnt(x, y, 0));
	}

	btLateral.arcTo(
		center.X(),
		center.Y() + ellipseRadY,
		ellipseRadX, ellipseRadY,
		center
	);

	btLateral.LineTo(btLateral.getCurrentPos().X(), lastSlideTopY + slotEdgeSpacing);
	btLateral.lineTo(-slotThicknessLoose, 0);
	btLateral.LineTo(btLateral.getCurrentPos().X(), sideHeight);
	btLateral.LineTo(0, sideHeight);
	btLateral.close();


	// Lateral Front Slots:
	for (int i = 0; i < levels-1; i++) {

		Standard_Real x = lateralFrontSlotsLocs[i].X();
		Standard_Real y = lateralFrontSlotsLocs[i].Y();
		btLateral.rectangle(latFrontSlotW, latFrontSLotH, x, y);

	}

	btLateral.build(thickness);


#pragma endregion

	//parts.push_back(btBack.prism);
	//for (auto w : btLateral.wires) {
	//	parts.push_back(w);
	//}
	//for (auto ee : btLateral.edges) {
	//	for (auto e : ee) {
	//		parts.push_back(e);
	//	}
	//}
	parts.push_back(btLateral.prism);
	//edges = btBack.edges;

}