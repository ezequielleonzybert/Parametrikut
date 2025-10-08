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
	Rect base(inWidth - thicknessDiff, d);

	// tabs
	float tabX = base.w / 2;
	float tabY = base.h / 2;
	Rect tabBase1(tabWidth + slotW, d / 2 + tabWidth, tabX, -tabY, Align::hh);
	Rect tabBase2(tabWidth + slotW, d / 2 + tabWidth, -tabX, -tabY, Align::lh);
	args.Append(tabBase1);
	args.Append(tabBase2);
	Rect tabSlot1(slotW, d / 2, tabX, tabY, Align::hl);
	Rect tabSlot2(slotW, d / 2, -tabX, tabY, Align::ll);
	tools.Append(tabSlot1);
	tools.Append(tabSlot2);

	TopoDS_Shape tab = cut(&args, &tools);
	args.Append(base);
	args.Append(tab);

	// shelflBackPins
	for (int i = 0; i < backPinsQ; i++) {
		float w = pinLength;
		float h = thickness * 1.5;
		float spacing = (base.w - backPinsQ * pinLength) / (backPinsQ + 1);
		float x = base.w / 2 - pinLength / 2 - spacing * (i + 1) - i * pinLength;
		float y = base.h / 2.0001 + h / 2;
		args.Append(Rect(w, h, x, y));
	}

	// shelfFrontPins
	int pinsQ = 1;
	float w = pinLength;
	float h = thickness;
	float y = -base.h / 2 - h / 2;
	if (pinsQ * pinLength < base.w / 7) {
		pinsQ++;
		float x = -base.w / 7;
		args.Append(Rect(w, h, x, y));
		args.Append(Rect(w, h, -x, y));
	}
	else args.Append(Rect(w, h, 0, y));


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
	Rect backBase(inWidth + (thickness - slotThicknessLoose), height);
	args.Append(backBase);
	{
		float x = backBase.w / 2;
		float y = backBase.h / 2;
		fillet2Locations.push_back(gp_Pnt(-x, -y, 0));
		fillet2Locations.push_back(gp_Pnt(x, -y, 0));
	}

	// backTabs
	Rect backTabBase(slotThicknessLoose + tabWidth, slotLength, Align::hh);
	Rect backTabCut(slotThicknessLoose, slotLength / 3, Align::hh);
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
	Rect backSlot(pinLength, slotThicknessLoose);
	std::vector<vec> backSlotsLocs;
	for (int i = 0; i < levels; i++) {
		float y = -backBase.h / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			float spacing = (backBase.w - backPinsQ * pinLength) / (backPinsQ + 1);
			float x = backBase.w / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
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
		backFace = fillet(backFace, vv30, thickness);
	}

	Back.shape = extrude(backFace, thickness);

#pragma endregion

#pragma region Lateral

	Part Lateral;

	// lateralBase
	Rect lateralBase(depth, sideHeight, Align::lh);
	args.Append(lateralBase);

	Rect lateralStraightRect(topShelfDepth + tabWidth + thickness, lateralBase.h, Align::lh);

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
		float backSlotX = -tabWidth - thickness / 2;
		float backSlotY = slotLength * 5 / 6 + backTabsLocs[i].y + backBase.h / 2;
		tools.Append(Rect(w, h, backSlotX, backSlotY));
		Lateral.addJoint("backSlot" + std::to_string(i), backSlotX, backSlotY - h / 2, thickness / 2, -90, -90, 0);
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
			slotX = -lateralStraightRect.w - ellipse.getRadAtY(slotY + slotLength) + slotW / 2 + spacing;
			float x1 = abs(slotX) - thickness / 2;
			float x2 = abs(Lateral.joints["backSlot0"].local.TranslationPart().X()) + thickness/2 ;
			slideW = abs(slideX) - tabWidth - thickness - (x1-x2)/2;
		}
		else {
			slotX = -lateralStraightRect.w + slotW / 2;
			float x1 = abs(slotX) - thickness / 2;
			float x2 = abs(Lateral.joints["backSlot0"].local.TranslationPart().X()) + thickness / 2;
			slideW = abs(slideX) - tabWidth - thickness - (x1 - x2) / 2;
		}

		tools.Append(Rect(slideW*2, slideH, slideX, slideY));
		tools.Append(Rect(slotW, slotH, slotX, slotY, Align::ch));

		Lateral.addJoint(
			"slide" + std::to_string(i),
			slideX + slideW, slideY, thickness / 2,
			-90, -90
		);
		Lateral.addJoint(
			"frontSlot" + std::to_string(i),
			slotX, slotY, thickness / 2,
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
		float x1 = abs(Lateral.joints["frontSlot" + std::to_string(i)].global.TranslationPart().X()) - thickness/2;
		float x2 = abs(Lateral.joints["backSlot0"].global.TranslationPart().X()) + thickness/2;
		float shelfDepth = x1 - x2 ;
		shelves.push_back(shelf(shelfDepth));
	}

#pragma endregion

#pragma region Front

	Part Front;

	Rect frontBase(inWidth, slotLength);
	args.Append(frontBase);

	// frontTabs
	TopoDS_Shape frontTab = tab(tabWidth, slotLength, slotThicknessLoose);
	TopoDS_Shape rightTab = translate(frontTab, vec(500, 0, 0));
	TopoDS_Shape leftTab = mirror(frontTab, vec(1,0,0));
	//args.Append(rightTab);
	//args.Append(leftTab);

	Front.shape = extrude(frontBase,thickness);

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
	parts.push_back(Front);
}