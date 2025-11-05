#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadcode2() {

	bool doFillet = true;
	Standard_Real f1 = 1, f2 = thickness;

#pragma region Back

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

	// backSlots
	Standard_Real backWidth = inWidth + tabWidth * 2 + slotThicknessLoose * 2;
	for (int i = 0; i < levels; i++) {
		Standard_Real y = -height / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			Standard_Real spacing = (backWidth - backPinsQ * pinLength) / (backPinsQ + 1);
			Standard_Real x = backWidth / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
			bt.rectangle(pinLength, slotThicknessLoose, x, y);
		}
	}

	bt.build(thickness);

#pragma endregion

	parts.push_back(bt.prism);

	edges = bt.edges;

}