/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.
*/

#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadCode()
{
	std::vector<gp_Pnt> fillet1Locations;
	std::vector<gp_Pnt> fillet2Locations;

	// backBase
	Rectangle backBase(inWidth + (thickness-slotThicknessLoose), height);
	args.Append(backBase);
	{
		float x = backBase.w / 2;
		float y = backBase.h / 2;
		fillet2Locations.push_back(gp_Pnt(-x, -y, 0));
		fillet2Locations.push_back(gp_Pnt(x, -y, 0));
	}

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

	TopoDS_Shape backFace = fusecut(&args, &tools);

	// fillet
	std::vector<TopoDS_Vertex> vv1;
	std::vector<TopoDS_Vertex> vv2;
	std::vector<TopoDS_Vertex> vv30;

	auto gx = groupBy(vertices(backFace), Axis::x);
	auto gy = groupBy(vertices(backFace), Axis::y);
	vv1.insert(vv1.end(), gx[1].begin(), gx[1].end());
	vv1.insert(vv1.end(), gx[gx.size() - 2].begin(), gx[gx.size() - 2].end());
	vv2.insert(vv2.end(), gx[0].begin(), gx[0].end());
	vv2.insert(vv2.end(), gx[gx.size()-1].begin(), gx[gx.size()-1].end());
	vv2.insert(vv2.end(), gy[0].begin(), gy[0].end());
	vv30.insert(vv30.end(), gy[gy.size() - 1].begin(), gy[gy.size() - 1].end());

	backFace = fillet(backFace, vv1, 1);
	backFace = fillet(backFace, vv2, thickness);
	backFace = fillet(backFace, vv30, 30);

	TopoDS_Shape back = extrude(backFace, thickness);

	parts.push_back(back);
}