/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.

Append shapes to fuse in the args array and shapes to cut in the tools array, then run fusecut()
*/

#include "Assembly.h"
#include "OcctUtils.h"

Part Assembly::shelf(float d) {

	Part Shelf;

	// base
	float slotW = slotThicknessLoose;
	float thicknessDiff = (slotW - thickness);
	Rectangle base(inWidth - thicknessDiff, d);

	// tabs
	float tabX = base.w / 2;
	float tabY = base.h / 2;
	Rectangle tabBase1(tabWidth + slotW, d / 2 + tabWidth, tabX, -tabY, Align::hh);
	Rectangle tabBase2(tabWidth + slotW, d / 2 + tabWidth, -tabX, -tabY, Align::lh);
	args.Append(tabBase1);
	args.Append(tabBase2);
	Rectangle tabSlot1(slotW, d / 2, tabX, tabY, Align::hl);
	Rectangle tabSlot2(slotW, d / 2, -tabX, tabY, Align::ll);
	tools.Append(tabSlot1);
	tools.Append(tabSlot2);

	TopoDS_Shape tab = cut(&args, &tools);
	args.Append(base);
	args.Append(tab);

	TopoDS_Shape fused= fuse(&args);
	Shelf.shape = extrude(fused, thickness);

	Shelf.addJoint("slide0", inWidth / 2 + thickness / 2, 0, thickness / 2);
	Shelf.addJoint("slide1", -inWidth / 2 - thickness / 2, 0, thickness / 2);

	return Shelf;
}

void Assembly::cadCode()
{
	bool doFillet = true;
	std::vector<gp_Pnt> fillet1Locations;
	std::vector<gp_Pnt> fillet2Locations;

#pragma region Back

	Part Back;

	// backBase
	Rectangle backBase(inWidth + (thickness - slotThicknessLoose), height);
	args.Append(backBase);
	{
		float x = backBase.w / 2;
		float y = backBase.h / 2;
		fillet2Locations.push_back(gp_Pnt(-x, -y, 0));
		fillet2Locations.push_back(gp_Pnt(x, -y, 0));
	}

	// backTabs
	Rectangle backTabBase(slotThicknessLoose + tabWidth, slotLength, Align::hh);
	Rectangle backTabCut(slotThicknessLoose, slotLength / 3, Align::hh);
	std::vector<vec> backTabsLocs;
	for (int i = 0; i < tabs; i++) {
		float x = backBase.w / 2;
		float ySpacing = (sideHeight - slotLength * 2.5) / (tabs - 1);
		float y = -backBase.h / 2 + slotLength / 2 + i * ySpacing;
		backTabsLocs.push_back(vec(x, y));

		TopoDS_Shape add = translate(backTabBase, vec(x, y));
		TopoDS_Shape sub = translate(backTabCut, vec(x, y));

		args.Append(add);
		tools.Append(sub);
		args.Append(mirror(add, vec(1, 0, 0)));
		tools.Append(mirror(sub, vec(1, 0, 0)));

		Back.addJoint("tab" + std::to_string(i), x + backTabCut.w / 2, y + backTabCut.h, thickness / 2, -90);
		Back.addJoint("tab" + std::to_string(i*2), -x - backTabCut.w / 2, y + backTabCut.h, thickness / 2, -90);
	}

	// backSlots
	Rectangle backSlot(backSlotLength, slotThicknessLoose);
	std::vector<vec> backSlotsLocs;
	for (int i = 0; i < levels; i++) {
		float y = -backBase.h / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			float spacing = (backBase.w - backPinsQ * backSlotLength) / (backPinsQ + 1);
			float x = backBase.w / 2 - backSlotLength / 2 - spacing * (j + 1) - j * backSlotLength;
			tools.Append(translate(backSlot, vec(x, y)));
			backSlotsLocs.push_back(vec(x, y));
		}
	}

	TopoDS_Shape backFace = fusecut(&args, &tools);
	args.Clear();
	tools.Clear();

	if (doFillet)
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

	Back.shape = extrude(backFace, thickness);

#pragma endregion

#pragma region Lateral

	Part Lateral;

	// lateralBase
	Rectangle lateralBase(depth, sideHeight, Align::lh);
	args.Append(lateralBase);

	Rectangle lateralStraightRect(topShelfDepth + tabWidth + thickness, lateralBase.h, Align::lh);

	// lateralElipseEdge
	Ellipse ellipse(depth - lateralStraightRect.w, sideHeight, -lateralStraightRect.w);
	tools.Append(ellipse);
	args.Append(intersect(&args, &tools));

	args.Append(lateralStraightRect);
	args.Append(fuse(&args));


	// lateralBackSlots
	for (int i = 0; i < backTabsLocs.size(); i++) {
		float w = slotThicknessLoose;
		float h = slotLength;
		float x = -tabWidth - thickness / 2;
		float y = slotLength * 5 / 6 + backTabsLocs[i].y + backBase.h / 2;
		tools.Append(Rectangle(w, h, x, y));
		Lateral.addJoint("backSlot" + std::to_string(i), x, y - h / 2, thickness / 2, -90, -90, 0);
	}

	// lateralSlides / lateralFrontSlots
	for (int i = 0; i < levels; i++) {
		float slideY = backSlotsLocs[i * backPinsQ].y + backBase.h / 2;
		float slideX = -lateralStraightRect.w - ellipse.getRadAtY(slideY);
		float slideH = slotThicknessLoose;
		float slotW = slotThicknessLoose;
		float slotH = slotLength;
		float slotY = slideY + slideH / 2 + tabWidth / 2;
		float spacing = tabWidth;

		float slotX;
		float slideW;

		if (i != levels - 1) {
			slideW = abs(slideX) - tabWidth - thickness;
			slotX = -lateralStraightRect.w - ellipse.getRadAtY(slotY + slotLength) + slotW / 2 + spacing;
		}
		else {
			slideW = (
				lateralStraightRect.w + ellipse.getRadAtY(slideY) - tabWidth - thickness
				- (lateralStraightRect.w - slotW + (slotW - thickness) / 2 - (tabWidth + thickness)) / 2
				) * 2;
			slotX = -lateralStraightRect.w + slotW / 2;
		}

		tools.Append(Rectangle(slideW, slideH, slideX, slideY));
		tools.Append(Rectangle(slotW, slotH, slotX, slotY, Align::ch));

		Lateral.addJoint(
			"slide" + std::to_string(i),
			slideX + slideW / 2, slideY, thickness / 2,
			-90, -90
		);
	}

	TopoDS_Shape lateralFace = fusecut(&args, &tools);
	args.Clear();
	tools.Clear();

	Lateral.shape = extrude(lateralFace, thickness);

#pragma endregion

#pragma region Shelves

	std::vector<Part> shelves;
	for (int i = 0; i < levels; i++) {
		float shelfDepth = -(Lateral.joints["slide"+std::to_string(i)].global.TranslationPart().X());
		shelves.push_back(shelf(shelfDepth));
	}

#pragma endregion

#pragma region Assembly

	Part Lateral1(Lateral);
	Part Lateral2(Lateral);

	Back.rotate(90, 0, 0);

	Lateral1.connect(Lateral1.joints["backSlot1"], Back.joints["tab1"]);
	Lateral2.connect(Lateral2.joints["backSlot1"], Back.joints["tab2"]);

	for (int i = 0; i < levels; i++)
	{
		shelves[i].connect(shelves[i].joints["slide0"], Lateral1.joints["slide" + std::to_string(i)]);
	}

	//Back.rotate(45, 0, 90);

#pragma endregion

	parts.push_back(Lateral1);
	parts.push_back(Lateral2);
	parts.push_back(Back);
	for(Part s : shelves)
		parts.push_back(s);
}