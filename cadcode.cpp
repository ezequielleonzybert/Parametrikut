/*
This is where the CAD code goes. Within this function, you can use OpenCASCADE 
but the parameters should be added in the Assembly struct with "addParam()"
and not as local variables.

Push the finished parts to the "parts" vector so the viewer can show them.
*/

#include "Assembly.h"
#include "OcctUtils.h"

bool doFillet = true;
TopTools_ListOfShape args;
TopTools_ListOfShape tools;

Part Assembly::shelf(Standard_Real d) {

	Part Shelf;

	// base
	Standard_Real slotW = slotThicknessLoose;
	Standard_Real thicknessDiff = (slotW - thickness);
	Rect base(inWidth - thicknessDiff, d);
	args.Append(base);

	// tabs
	TopoDS_Shape shelfTab = tab(tabWidth, base.h / 2 + tabWidth, slotThicknessLoose, tabWidth, true);

	Standard_Real tabX = base.w / 2;
	Standard_Real tabY = -base.h / 2 + base.h / 2 + tabWidth;
	TopoDS_Shape shelfTabMoved = translate(shelfTab, vec(tabX, tabY, 0));
	args.Append(shelfTabMoved);
	args.Append(mirror(shelfTabMoved, vec(1,0,0)));

	args.Append(fuse(&args));

	// shelflBackPins
	for (int i = 0; i < backPinsQ; i++) {
		Standard_Real w = pinLength;
		Standard_Real h = thickness * 1.5;
		Standard_Real spacing = (base.w - backPinsQ * pinLength) / (backPinsQ + 1);
		Standard_Real x = base.w / 2 - pinLength / 2 - spacing * (i + 1) - i * pinLength;
		Standard_Real y = base.h / 2 + h / 2;
		args.Append(Rect(w, h, x, y));
	}

	// shelfFrontPins
	int pinsQ = 1;
	Standard_Real w = pinLength;
	Standard_Real h = thickness;
	Standard_Real y = -base.h / 2 - h / 2;
	if (pinsQ * pinLength < base.w / 7) {
		pinsQ++;
		Standard_Real x = -base.w / 7;
		args.Append(Rect(w, h, x, y));
		args.Append(Rect(w, h, -x, y));
	}
	else args.Append(Rect(w, h, 0, y));

	TopoDS_Shape shelfFace = fuse(&args);

	if (doFillet) {
		std::vector<TopoDS_Vertex> vv1;
		std::vector<TopoDS_Vertex> vv2;

		auto gx = groupBy(vertices(shelfFace), Axis::x);
		auto gy = groupBy(vertices(shelfFace), Axis::y);

		vv2.push_back(gx[0][0]);
		vv2.push_back(gx[0][1]);
		vv2.push_back(gx[2][0]);
		vv2.push_back(gx[gx.size() - 1][0]);
		vv2.push_back(gx[gx.size() - 1][1]);
		vv2.push_back(gx[gx.size() - 3][1]);

		vv1.push_back(gx[1][1]);
		vv1.push_back(gx[gx.size() - 2][0]);
		vv1.push_back(gy[0][0]);
		vv1.push_back(gy[0][1]);
		vv1.push_back(gy[gy.size() - 1][0]);
		vv1.push_back(gy[gy.size() - 1][1]);
		vv1.push_back(gy[gy.size() - 1][2]);
		vv1.push_back(gy[gy.size() - 1][3]);
		
		shelfFace = fillet(shelfFace, vv1, thickness/3);
		shelfFace = fillet(shelfFace, vv2, thickness);
	}

	Shelf.shape = extrude(shelfFace, thickness);

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
	args.Append(backBase);

	// backTabs
	TopoDS_Shape backTab = tab(tabWidth, slotLength, slotThicknessLoose, slotLength/3);

	std::vector<vec> backTabsLocs;
	for (int i = 0; i < tabs; i++) {
		Standard_Real x = backBase.w / 2;
		Standard_Real ySpacing = (sideHeight - slotLength * 2.5f) / (tabs - 1);
		Standard_Real y = -backBase.h / 2 + slotLength / 2 + i * ySpacing;
		backTabsLocs.push_back(vec(x, y));

		TopoDS_Shape backTabMoved = translate(backTab, vec(x, y));
		args.Append(backTabMoved);
		args.Append(mirror(backTabMoved, vec(1, 0, 0)));

		Back.addJoint("tab" + std::to_string(i), x + slotThicknessLoose / 2, y + slotLength/3, thickness / 2, -90.f);
		Back.addJoint("tab" + std::to_string(i*2), -x - slotThicknessLoose / 2, y + slotLength/3, thickness / 2, -90.f);
	}

	TopoDS_Shape backFace = fuse(&args);

	args.Append(backFace);

	// backSlots
	Rect backSlot(pinLength, slotThicknessLoose);
	std::vector<vec> backSlotsLocs;
	for (int i = 0; i < levels; i++) {
		Standard_Real y = -backBase.h / 2 + i * shelvesSpacing + tabWidth + slotThicknessLoose / 2 + thickness * i;
		for (int j = 0; j < backPinsQ; j++) {
			Standard_Real spacing = (backBase.w - backPinsQ * pinLength) / (backPinsQ + 1);
			Standard_Real x = backBase.w / 2 - pinLength / 2 - spacing * (j + 1) - j * pinLength;
			tools.Append(translate(backSlot, vec(x, y)));
			backSlotsLocs.push_back(vec(x, y));
		}
	}

	backFace = cut(&args, &tools);

	if (doFillet)
	{
		std::vector<TopoDS_Vertex> vv1;
		std::vector<TopoDS_Vertex> vv2;
		std::vector<TopoDS_Vertex> vvSign;

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
		vvSign.insert(vvSign.end(), gy[gy.size() - 1].begin(), gy[gy.size() - 1].end());

		backFace = fillet(backFace, vv1, thickness/3);
		backFace = fillet(backFace, vv2, thickness);
		backFace = fillet(backFace, vvSign, thickness);
	}

	Back.shape = extrude(backFace, thickness);

#pragma endregion

#pragma region Lateral

	Part Lateral;
	std::vector<TopoDS_Vertex> lateralVV1;

	// lateralBase
	Rect lateralBaseRect(depth, sideHeight, Align::lh);
	args.Append(lateralBaseRect);

	Rect lateralStraightRect(topShelfDepth + tabWidth + thickness, lateralBaseRect.h, Align::lh);

	// lateralElipseEdge
	Ellipse ellipse(depth - lateralStraightRect.w, sideHeight, -lateralStraightRect.w);
	tools.Append(ellipse);
	args.Append(intersect(&args, &tools));

	args.Append(lateralStraightRect);
	TopoDS_Shape lateralBase(fuse(&args));

	args.Append(lateralBase);

	// lateralBackSlots
	for (int i = 0; i < backTabsLocs.size(); i++) {
		Standard_Real w = slotThicknessLoose;
		Standard_Real h = slotLength;
		Standard_Real backSlotX = -tabWidth - thickness / 2;
		Standard_Real backSlotY = slotLength * 5 / 6 + backTabsLocs[i].y + backBase.h / 2;
		tools.Append(Rect(w, h, backSlotX, backSlotY));
		Lateral.addJoint("backSlot" + std::to_string(i), backSlotX, backSlotY - h / 2, thickness / 2, -90, -90, 0);
	}

	// lateralSlides / lateralFrontSlots
	for (int i = 0; i < levels; i++) {
		Standard_Real slideY = backSlotsLocs[i * backPinsQ].y + backBase.h / 2;
		Standard_Real slideX = -lateralStraightRect.w - ellipse.getRadAtY(slideY);
		Standard_Real slideH = slotThicknessLoose;
		Standard_Real slotW = slotThicknessLoose;
		Standard_Real slotH = slotLength;
		Standard_Real slotY = slideY + slideH / 2 + tabWidth / 2;
		Standard_Real spacing = tabWidth;

		Standard_Real slotX;
		Standard_Real slideW;


		Standard_Real vx1 = -lateralStraightRect.w - ellipse.getRadAtY(slideY + slideH / 2);
		Standard_Real vy1 = slideY + slideH / 2;
		Standard_Real vx2 = -lateralStraightRect.w - ellipse.getRadAtY(slideY - slideH / 2);
		Standard_Real vy2 = slideY - slideH / 2;
		lateralVV1.push_back(makeVertex(vx1, vy1));
		lateralVV1.push_back(makeVertex(vx2, vy2));

		if (i != levels - 1) {
			slotX = -lateralStraightRect.w - ellipse.getRadAtY(slotY + slotLength) + slotW / 2 + spacing;
		}
		else {
			slotX = -lateralStraightRect.w + slotW / 2;
		}
		Standard_Real x1 = -slotX - thickness / 2;
		Standard_Real x2 = Standard_Real(-Lateral.joints["backSlot0"].local.TranslationPart().X() + thickness / 2);
		slideW = -slideX - tabWidth - thickness - (x1 - x2) / 2;

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

	TopoDS_Shape lateralShape = fusecut(&args, &tools);

	if (doFillet) {
		std::vector<TopoDS_Vertex> lateralBaseVV = vertices(lateralBase);
		std::vector<TopoDS_Vertex> lateralVV2;

		lateralVV2.insert(lateralVV2.end(), lateralBaseVV.begin(), lateralBaseVV.end() - 1);

		auto gx = groupBy(vertices(backFace), Axis::x);
		auto gy = groupBy(vertices(backFace), Axis::y);
		lateralShape = fillet(lateralShape, lateralVV2, thickness);
		lateralShape = fillet(lateralShape, lateralVV1, thickness/3);
	}

	Lateral.shape = extrude(lateralShape, thickness);

#pragma endregion

#pragma region Shelves
	
	std::vector<Part> shelves;
	for (int i = 0; i < levels; i++) {
		Standard_Real x1 = Standard_Real(- Lateral.joints["frontSlot" + std::to_string(i)].global.TranslationPart().X() - thickness / 2);
		Standard_Real x2 = Standard_Real(- Lateral.joints["backSlot0"].global.TranslationPart().X() + thickness / 2);
		Standard_Real shelfDepth = x1 - x2 ;
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

	Standard_Real x = frontBase.w / 2 + thickness/2;
	Standard_Real y = -slotLength / 2 + slotLength / 2;
	Front.addJoint("tab0", vec(x, y, thickness / 2),-90);
	Front.addJoint("tab1", vec(-x, y, thickness / 2),-90);

	// frontCut
	Standard_Real cutX = frontBase.w / 2 - tabWidth;
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

		frontShape = fillet(frontShape, vv1, thickness/3);
		frontShape = fillet(frontShape, vv2, thickness);

	}

	Front.shape = extrude(frontShape,thickness);

#pragma endregion

#pragma region Assembly

	// Creating parts
	Part Lateral1(Lateral);
	Part Lateral2(Lateral);
	std::vector<Part> fronts;
	for (int i = 0; i < levels; i++)
	{
		fronts.push_back(Part(Front));
	}

	// vector of every part to arrange in the plane and get the section
	std::vector<Part> partsList;
	partsList.push_back(Lateral1);
	partsList.push_back(Lateral2.mirrored(false));
	partsList.push_back(Back);
	partsList.insert(partsList.end(), fronts.begin(), fronts.end());
	partsList.insert(partsList.end(), shelves.begin(), shelves.end());

	TopoDS_Compound packed = pack(partsList);

	// Assembling parts
	Back.rotate(90, 0, 0);

	Lateral1.connect(Lateral1.joints["backSlot1"], Back.joints["tab1"]);
	Lateral2.connect(Lateral2.joints["backSlot1"], Back.joints["tab2"]);
	
	for (int i = 0; i < levels; i++)
	{
		shelves[i].connect(shelves[i].joints["slide0"], Lateral1.joints["slide" + std::to_string(i)]);
		fronts[i].connect(fronts[i].joints["tab0"], Lateral1.joints["frontSlot" + std::to_string(i)]);
	}

#pragma endregion

	parts.push_back(Lateral1);
	parts.push_back(Lateral2);
	parts.push_back(Back);
	for (int i = 0; i < levels; i++) {
		parts.push_back(shelves[i]);
		parts.push_back(fronts[i]);
	}

	TopoDS_Shape outlines = section(packed);
	parts.push_back(outlines);

	Exporter exporter; 
	exporter.setShape(outlines);
	exporter.exportToFile("C:\\Users\\Ezequiel\\Desktop\\output.svg");
}