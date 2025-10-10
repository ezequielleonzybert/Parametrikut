/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.

Push the finished parts to the "parts" vector so the viewer can show them.
*/

#include "Assembly.h"
#include "OcctUtils.h"

bool doFillet = true;

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

	if (doFillet) {
		std::vector<TopoDS_Vertex> vv1;
		std::vector<TopoDS_Vertex> vv2;

		auto gx = groupBy(vertices(fused), Axis::x);
		auto gy = groupBy(vertices(fused), Axis::y);

		vv2.push_back(gx[0][0]);
		vv2.push_back(gx[0][1]);
		vv2.push_back(gx[2][1]);
		vv2.push_back(gx[gx.size() - 1][0]);
		vv2.push_back(gx[gx.size() - 1][1]);
		vv2.push_back(gx[gx.size() - 3][1]);

		vv1.push_back(gx[1][1]);
		vv1.push_back(gx[gx.size() - 2][0]);
		
		fused = fillet(fused, vv1, 1);
		fused = fillet(fused, vv2, thickness);
	}

	Shelf.shape = extrude(fused, thickness);

	Shelf.addJoint("slide0", inWidth / 2 + thickness / 2, 0, thickness / 2);
	Shelf.addJoint("slide1", -inWidth / 2 - thickness / 2, 0, thickness / 2);

	return Shelf;
}

void Assembly::cadCode()
{
#pragma region Back

	Part Back;

	// backBase
	Rect backBase(inWidth + (thickness - slotThicknessLoose), height);

	// backTabs
	TopoDS_Shape backTab = tab(tabWidth, slotLength, slotThicknessLoose, slotLength/3);
	std::vector<vec> backTabsLocs;
	for (int i = 0; i < tabs; i++) {
		float x = backBase.w / 2;
		float ySpacing = (sideHeight - slotLength * 2.5) / (tabs - 1);
		float y = -backBase.h / 2 + slotLength / 2 + i * ySpacing;
		backTabsLocs.push_back(vec(x, y));

		TopoDS_Shape backTabMoved = translate(backTab, vec(x, y));
		args.Append(backTabMoved);
		args.Append(mirror(backTabMoved, vec(1, 0, 0)));

		Back.addJoint("tab" + std::to_string(i), x + slotThicknessLoose / 2, y + slotLength/3, thickness / 2, -90);
		Back.addJoint("tab" + std::to_string(i*2), -x - slotThicknessLoose / 2, y + slotLength/3, thickness / 2, -90);
	}
	args.Append(backBase);
	args.Append(fuse(&args));

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

	TopoDS_Shape backFace = cut(&args, &tools);

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

	// frontTabs
	TopoDS_Shape frontTab = tab(tabWidth, slotLength, slotThicknessLoose, slotLength/2);
	TopoDS_Shape rightTab = translate(frontTab, vec(frontBase.w/2, -frontBase.h/2, 0));
	TopoDS_Shape leftTab = mirror(rightTab, vec(1,0,0));

	args.Append(frontBase);
	args.Append(rightTab);
	args.Append(leftTab);

	float x = frontBase.w / 2 + thickness/2;
	float y = -slotLength / 2 + slotLength / 2;
	Front.addJoint("tab0", vec(x, y, thickness / 2),-90);
	Front.addJoint("tab1", vec(-x, y, thickness / 2),-90);

	// frontCut
	float cutX = frontBase.w / 2 - tabWidth;
	Triangle triangleCut(-slotLength/3, slotLength/2, cutX - slotLength/4, frontBase.h/2);
	Rect rectangleCut(cutX - slotLength/4, frontBase.h/3, 0 ,frontBase.h/2 - frontBase.h/3,Align::hh);

	tools.Append(triangleCut);
	tools.Append(rectangleCut);
	tools.Append(mirror(triangleCut, vec(1, 0, 0)));
	tools.Append(mirror(rectangleCut,vec(1,0,0)));

	//frontSlot
	tools.Append(Rect(pinLength, slotThicknessLoose, 0, -tabWidth/2 - slotThicknessLoose/2));

	TopoDS_Shape frontShape(fusecut(&args, &tools));

	if (doFillet) {
		std::vector<TopoDS_Vertex> vv1;
		std::vector<TopoDS_Vertex> vv2;

		auto gx = groupBy(vertices(frontShape), Axis::x);
		auto gy = groupBy(vertices(frontShape), Axis::y);

		vv1.push_back(gx[1][0]);
		vv1.push_back(gx[2][0]);
		vv1.push_back(gx[gx.size() - 2][1]);
		vv1.push_back(gx[gx.size() - 3][0]);

		vv2.push_back(gx[0][0]);
		vv2.push_back(gx[0][1]);
		vv2.push_back(gx[3][0]);
		vv2.push_back(gx[4][0]);
		vv2.push_back(gx[gx.size() - 5][0]);
		vv2.push_back(gx[gx.size() - 4][0]);
		vv2.push_back(gx[gx.size() - 1][0]);
		vv2.push_back(gx[gx.size() - 1][1]);

		frontShape = fillet(frontShape, vv1, 1);
		frontShape = fillet(frontShape, vv2, thickness);

	}

	Front.shape = extrude(frontShape,thickness);

#pragma endregion

#pragma region Assembly

	Part Lateral1(Lateral);
	Part Lateral2(Lateral);

	Back.rotate(90, 0, 0);

	Lateral1.connect(Lateral1.joints["backSlot1"], Back.joints["tab1"]);
	Lateral2.connect(Lateral2.joints["backSlot1"], Back.joints["tab2"]);

	std::vector<Part> fronts;
	for (int i = 0; i < levels; i++)
	{
		fronts.push_back(Part(Front));
		shelves[i].connect(shelves[i].joints["slide0"], Lateral1.joints["slide" + std::to_string(i)]);
		fronts[i].connect(fronts[i].joints["tab0"], Lateral1.joints["frontSlot" + std::to_string(i)]);
	}

	//Back.rotate(45, 0, 90);

#pragma endregion

	//parts.push_back(Lateral1);
	parts.push_back(Lateral2);
	parts.push_back(Back);
	for (int i = 0; i < levels; i++) {
		parts.push_back(shelves[i]);
		parts.push_back(fronts[i]);
	}
}