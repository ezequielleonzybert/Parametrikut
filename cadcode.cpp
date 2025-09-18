/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.

Append shapes to fuse in the args array and shapes to cut in the tools array, then run fusecut()
*/

#include "Assembly.h"
#include "OcctUtils.h"

void Assembly::cadCode()
{
	bool doFillet = true;
	std::vector<gp_Pnt> fillet1Locations;
	std::vector<gp_Pnt> fillet2Locations;

#pragma region Back
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
	std::vector<vec> tabsLocs;
	std::vector<vec> tabsJoints;
	for (int i = 0; i < tabs; i++) {
		float x = backBase.w / 2;
		float ySpacing = (sideHeight - slotLength * 2) / (tabs-1);
		float y = -backBase.h/2 + slotLength/2 + i*ySpacing;
		tabsLocs.push_back(vec(x, y));
		tabsJoints.push_back(vec(x+backTabCut.w/2, y+backTabCut.h, thickness/2));

		TopoDS_Shape add = translate(backTabBase, vec(x, y));
		TopoDS_Shape sub = translate(backTabCut, vec(x, y));

		args.Append(add);
		tools.Append(sub);
		args.Append(mirror(add, vec(1, 0, 0)));
		tools.Append(mirror(sub, vec(1, 0, 0)));
	}

	// backSlots
	Rectangle backSlot(backSlotLength, slotThicknessLoose);
	for (int i = 0; i < levels; i++) {
		float y = -backBase.h / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			float spacing = (backBase.w - backPinsQ * backSlotLength) / (backPinsQ + 1);
			float x = backBase.w / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
			tools.Append(translate(backSlot, vec(x,y)));
		}
	}

	TopoDS_Shape backFace = fusecut(&args, &tools);
	args.Clear();
	tools.Clear();

	if(doFillet)
	{
		std::vector<TopoDS_Vertex> vv1;
		std::vector<TopoDS_Vertex> vv2;
		std::vector<TopoDS_Vertex> vv30;

		auto gx = groupBy(vertices(backFace), Axis::x);
		auto gy = groupBy(vertices(backFace), Axis::y);

		sortBy(gx[1], Axis::y);
		for (int i = 0; i < gx[1].size(); i++) {
			if (i % 2 == 0) {
				vv1.push_back(gx[1][i]);
			}
		}
		sortBy(gx[gx.size() - 2], Axis::y);
		for (int i = 0; i < gx[gx.size() - 2].size(); i++) {
			if (i % 2 == 0) {
				vv1.push_back(gx[gx.size() - 2][i]);
			}
		}

		vv2.insert(vv2.end(), gx[0].begin(), gx[0].end());
		vv2.insert(vv2.end(), gx[gx.size() - 1].begin(), gx[gx.size() - 1].end());
		vv2.insert(vv2.end(), gy[0].begin(), gy[0].end());
		vv30.insert(vv30.end(), gy[gy.size() - 1].begin(), gy[gy.size() - 1].end());

		backFace = fillet(backFace, vv1, 1);
		backFace = fillet(backFace, vv2, thickness);
		backFace = fillet(backFace, vv30, 30);
	}

	Part Back = extrude(backFace, thickness);

#pragma endregion

#pragma region Lateral

	// lateralBase
	Rectangle lateralBase(depth, sideHeight, Align::lh);
	args.Append(lateralBase);

	// lateralSideSlots
	for (vec loc : tabsLocs) {
		float w = slotThicknessLoose;
		float h = slotLength;
		float x = -tabWidth - w / 2;
		float y = tabWidth + loc.y + backBase.h/2;
		tools.Append(Rectangle(w, h, x, y));
	}

	TopoDS_Shape lateralFace = fusecut(&args, &tools);
	args.Clear();
	tools.Clear();

	TopoDS_Shape Lateral = extrude(lateralFace, thickness);

#pragma endregion

#pragma region Assembly
	
	gp_Trsf tr;
	tr.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(1,0,0)), M_PI/2);
	//Back = Back.Located(TopLoc_Location(tr));
	Part BackP(Back);
	BackP.rotate(0,0,0);
	//BackP.addJoint(0, -200, 0);
	BackP.addJoint(tabsJoints[0],-90);

#pragma endregion

	//parts.push_back(Lateral);
	//parts.push_back(Back);
	parts.push_back(BackP);
}