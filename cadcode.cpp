/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.
*/

#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadCode()
{

	// backBase
	Rectangle backBase(inWidth + (thickness-slotThicknessLoose), height);
	args.Append(backBase);

	// backTabs
	Rectangle backTabBase(slotThicknessLoose + tabWidth, slotLength, Align::hh);
	Rectangle backTabCut(slotThicknessLoose, slotLength/3 ,Align::hh);
	for (int i = 0; i < tabs; i++) {
		float x = backBase.w / 2;
		float ySpacing = (sideHeight - slotLength * 2) / (tabs - 1);
		float y = -backBase.h/2 + slotLength/2 + i*ySpacing;

		TopoDS_Shape add = translate(backTabBase, vec(x, y));
		TopoDS_Shape sub = translate(backTabCut, vec(x, y));

		args.Append(add);
		tools.Append(sub);
		args.Append(mirror(add, vec(1, 0, 0)));
		tools.Append(mirror(sub, vec(1, 0, 0)));
	}

	// backSlots
	Rectangle backSlot(backSlotLength, slotThicknessMid);
	for (int i = 0; i < levels; i++) {
		float y = -backBase.h / 2 + i * shelvesSpacing + tabWidth + slotThicknessMid / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			float spacing = (backBase.w - backPinsQ * backSlotLength) / (backPinsQ + 1);
			float x = backBase.w / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
			tools.Append(translate(backSlot, vec(x,y)));
		}
	}

	TopoDS_Shape backFace = fusecut(args, tools);

	parts.push_back(extrude(backFace,thickness));
}