#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadcode2() {

	Standard_Real f1 = 1, f2 = tabWidth / 3;

#pragma region Back

	Part Back;

	// outer Wire
	BuildingTool back(-inWidth / 2, 0);
	back.lineTo(inWidth, 0, f2);
	back.lineTo(0, slotLength / 2);

	// Back tabs
	std::vector<gp_Pnt> backTabsLocs;
	Standard_Real ySpacing = (sideHeight - slotLength - (slotLength * 2 / 3) * tabs) / (tabs - 1);
	;	for (int i = 0; i < tabs; i++) {
		back.addJoint("tab" + std::to_string(i * 2), back.X() + slotThicknessLoose / 2, back.Y(), thickness / 2, -90);
		back.addJoint("tab" + std::to_string(i * 2 + 1), -back.X() - slotThicknessLoose / 2, back.Y(), thickness / 2, -90);
		back.lineTo(slotThicknessLoose, 0);
		back.lineTo(0, -slotLength / 3, f1);
		back.lineTo(tabWidth, 0, f2);
		back.lineTo(0, slotLength, f2);
		back.lineTo(-tabWidth - slotThicknessLoose, 0);
		backTabsLocs.push_back(gp_Pnt(back.X(), back.Y() - slotLength / 2, 0));
		if (i != tabs - 1) {
			back.lineTo(0, ySpacing);
		}
	}

	back.lineTo(0, slotLength / 2 + signHeight, f2);
	back.lineTo(-inWidth, 0, f2);
	back.lineTo(0, -slotLength / 2 - signHeight);

	for (int i = 0; i - tabs; i++) {
		back.lineTo(-tabWidth - slotThicknessLoose, 0, f2);
		back.lineTo(0, -slotLength, f2);
		back.lineTo(tabWidth, 0, f1);
		back.lineTo(0, slotLength / 3);
		back.lineTo(slotThicknessLoose, 0);
		if (i != tabs - 1) {
			back.lineTo(0, -ySpacing);
		}
	}
	back.close(f2);

	// backSlots
	std::vector<gp_Pnt> backSlotsLocs;
	Standard_Real backWidth = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	for (int i = 0; i < levels; i++) {
		Standard_Real y = tabWidth + i * shelvesSpacing + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			Standard_Real spacing = (backWidth - backPinsQ * pinLength) / (backPinsQ + 1);
			Standard_Real x = backWidth / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
			back.rectangle(pinLength, slotThicknessLoose, x, y);
			backSlotsLocs.push_back(gp_Pnt(x, y, 0));
		}
	}

	back.build(thickness);

#pragma endregion

#pragma region Lateral

	// Outer Wire
	Standard_Real slotEdgeSpacing = thickness * 2;
	Standard_Real shelvesStartX = tabWidth + looseDiff / 2 + thickness;

	BuildingTool lateral;

	// Calculate Bottom Front Slot, needed to calculate the lateral total depth and ellipse rx:
	Standard_Real latFrontSlotX = tabWidth + looseDiff / 2 + slotEdgeSpacing + botShelfDepth;
	Standard_Real latFrontSlotY = backSlotsLocs[0].Y() + slotLength / 2 + thickness * 1.5;
	Standard_Real latFrontSlotW = slotThicknessLoose;
	Standard_Real latFrontSLotH = slotLength;

	// ellipse must pass through this point.
	gp_Pnt recTopRight(
		latFrontSlotX + latFrontSlotW / 2 + slotEdgeSpacing,
		latFrontSlotY + latFrontSLotH / 2,
		0
	);

	Standard_Real ellipseRadY = sideHeight;
	Standard_Real ery2 = ellipseRadY * ellipseRadY;
	Standard_Real px = recTopRight.X() - (topShelfDepth + tabWidth + looseDiff + thickness * 2);
	Standard_Real px2 = px * px;
	Standard_Real py2 = recTopRight.Y() * recTopRight.Y();
	Standard_Real ellipseRadX = abs(px) * sqrt(ery2 / (ery2 - py2));

	depth = topShelfDepth + tabWidth + looseDiff + thickness * 2 + ellipseRadX;

	// Ellipse arcs and shelves slides
	lateral.moveTo(0, 0);
	lateral.lineTo(depth, 0, f2);

	gp_Pnt center(lateral.getCurrentPos().Translated({ -ellipseRadX,0,0 }));
	Ellipse2 ellipse(center, ellipseRadX, ellipseRadY);

	std::vector<gp_Pnt> lateralFrontSlotsLocs;
	Standard_Real lastSlideTopY;
	std::vector<Standard_Real> shelvesDepth;
	for (int i = 0; i < levels; i++) {

		Standard_Real y = backSlotsLocs[i * backPinsQ].Y() - slotThicknessLoose / 2;
		Standard_Real x = center.X() + ellipse.getRadiusX(y);
		lateral.arcTo(x, y, ellipseRadX, ellipseRadY, center, f1);

		// prepare front slots locations for next step
		Standard_Real slotCornerY = lateral.Y() + slotThicknessLoose + slotEdgeSpacing + slotLength;
		Standard_Real slotMidX = center.X() + ellipse.getRadiusX(slotCornerY) - slotEdgeSpacing;
		Standard_Real slotMidY = lateral.Y() + slotThicknessLoose + slotEdgeSpacing + slotLength / 2;
		if (i < levels - 1)
			lateralFrontSlotsLocs.push_back(gp_Pnt(slotMidX, slotMidY, 0));

		// slides
		if (i < levels - 1) {
			Standard_Real shelfDepth = lateralFrontSlotsLocs[i].X() - thickness / 2 - shelvesStartX; // thickness or slotThickness? wrong
			shelvesDepth.push_back(shelfDepth);
			lateral.LineTo(shelvesStartX + shelfDepth / 2, lateral.Y());
		}
		else {
			lateral.LineTo(shelvesStartX + topShelfDepth / 2, lateral.Y());
			shelvesDepth.push_back(topShelfDepth);
		}
		lateral.addJoint("slide" + std::to_string(i), lateral.X(), lateral.Y() + slotThicknessLoose / 2, thickness / 2, -90, 90, 0);
		lateral.lineTo(0, slotThicknessLoose);
		y = lateral.Y();
		x = center.X() + ellipse.getRadiusX(y);
		lateral.LineTo(x, y, f1);
		if (i == levels - 1) lastSlideTopY = y;
	}

	lateral.arcTo(
		center.X(),
		center.Y() + ellipseRadY,
		ellipseRadX, ellipseRadY,
		center,
		f1
	);

	lateral.LineTo(lateral.X(), lastSlideTopY + slotEdgeSpacing);
	lateral.lineTo(-slotThicknessLoose, 0);
	lateral.LineTo(lateral.X(), sideHeight, f1);
	lateral.LineTo(0, sideHeight, f2);
	lateral.close(f2);


	// Lateral front slots:
	for (int i = 0; i < levels - 1; i++) {
		Standard_Real x = lateralFrontSlotsLocs[i].X();
		Standard_Real y = lateralFrontSlotsLocs[i].Y();
		lateral.rectangle(latFrontSlotW, latFrontSLotH, x, y);
	}

	// Lateral back slots
	for (int i = 0; i < tabs; i++) {
		Standard_Real x = tabWidth + slotThicknessLoose / 2;
		Standard_Real y = backTabsLocs[i].Y() + slotLength / 3;
		lateral.rectangle(slotThicknessLoose, slotLength, x, y);
		lateral.addJoint("frontSlot" + std::to_string(i), x, y - slotLength / 2, thickness / 2, -90, 90, 0);
	}

	lateral.build(thickness);

#pragma endregion

#pragma region Shelves

	std::vector<BuildingTool> shelves;

	for (int j = 0; j < levels; j++) {
		Standard_Real shelfDepth;

		int pinsQ = 1;
		Standard_Real frontPinsSpacing;
		Standard_Real firstPinX = -pinLength / 2;
		if (pinsQ * pinLength < inWidth / 4 / 3) {
			pinsQ++;
			firstPinX = -inWidth / 8;
			frontPinsSpacing = inWidth / 4 - pinsQ * pinLength;
		}

		// front pins
		BuildingTool shelf(-width / 2, 0);
		shelf.LineTo(firstPinX, 0);
		for (int i = 0; i < pinsQ; i++) {
			shelf.lineTo(0, -thickness, f1);
			shelf.lineTo(pinLength, 0, f1);
			shelf.lineTo(0, thickness);
			if (pinsQ > 1 && i < pinsQ - 1)
				shelf.lineTo(frontPinsSpacing, 0);
		}

		// tab
		shelf.LineTo(width / 2, 0, f2);
		shelf.lineTo(0, shelvesDepth[j] / 2 + tabWidth, f2);
		shelf.lineTo(-tabWidth, 0, f1);
		shelf.lineTo(0, -tabWidth);
		shelf.addJoint("tab", shelf.X() - slotThicknessLoose / 2, shelf.Y(), thickness / 2);
		shelf.lineTo(-slotThicknessLoose, 0);
		shelf.lineTo(0, shelvesDepth[j] / 2, f1);

		// back pins
		for (int i = 0; i < backPinsQ; i++) {
			Standard_Real x = backSlotsLocs[i].X() + pinLength / 2;
			shelf.LineTo(x, shelf.Y());
			shelf.lineTo(0, thickness * 1.5, f1);
			shelf.lineTo(-pinLength, 0, f1);
			shelf.lineTo(0, -thickness * 1.5);
		}

		// tab
		shelf.LineTo(-inWidth / 2, shelvesDepth[j], f1);
		shelf.lineTo(0, -shelvesDepth[j] / 2);
		shelf.lineTo(-slotThicknessLoose, 0);
		shelf.lineTo(0, tabWidth, f1);
		shelf.lineTo(-tabWidth, 0, f2);
		shelf.close(f2);

		shelf.build(thickness);

		shelves.push_back(shelf);
	}

#pragma endregion

#pragma region Assembly

	back.rotate(90);

	BuildingTool lateral2 = lateral;
	lateral.rigidJoint(lateral.joints["frontSlot0"], back.joints["tab0"]);
	lateral2.rigidJoint(lateral2.joints["frontSlot0"], back.joints["tab1"]);

	for (int i = 0; i < levels; i++) {
		shelves[i].rigidJoint(shelves[i].joints["tab"], lateral.joints["slide" + std::to_string(i)]);
	}

#pragma endregion

	parts.push_back(back);
	parts.push_back(lateral);
	parts.push_back(lateral2);
	for (auto shelf : shelves) {
		parts.push_back(shelf);
	}

	//edges must be poblated to export SVG
	for (auto ee : back.edges) {
		edges.push_back(ee);
	}
	for (auto ee : lateral.edges) {
		edges.push_back(ee);
	}

}