#pragma once

#include <TopoDS_Shape.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <GC_MakeEllipse.hxx>
#include <GCE2d_MakeEllipse.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepPrimAPI_MakeBox.hxx> 
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Elips.hxx>
#include <unordered_set>
#include <unordered_map>
#include <ChFi2d_FilletAPI.hxx>
#include <ShapeAnalysis.hxx>

inline TopTools_ListOfShape args;
inline TopTools_ListOfShape tools;

enum class Axis {
	x,
	y,
	z
};

enum class Align {
	cc,
	cl,
	lc,
	ch,
	hc,
	ll,
	lh,
	hh,
	hl
};

struct vec {
	float x;
	float y;
	float z;
	vec() :
		x(0), y(0), z(0) {
	}
	vec(float x, float y, float z = 0) :
		x(x), y(y), z(z) {
	}
	void set(float x, float y, float z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class Triangle {
public:
	TopoDS_Face face;
	TopoDS_Shape shape;
	Triangle(float height, float width, float x = 0, float y = 0) {
		gp_Pnt p1(-width / 2 + x, y, 0);
		gp_Pnt p2(width / 2 + x, y, 0);
		gp_Pnt p3(x, height + y, 0);

		BRepBuilderAPI_MakePolygon poly(p1, p2, p3, true);
		TopoDS_Wire wire = poly.Wire();
		face = BRepBuilderAPI_MakeFace(wire).Face();
		shape = BRepBuilderAPI_MakeFace(wire).Shape();
	}

	operator TopoDS_Shape() const {
		return TopoDS_Shape(shape);
	}
	operator TopoDS_Face() const {
		return TopoDS_Face(face);
	}
};

class Rect {

public:
	float w, h, x, y;

private:

	Align align;
	TopoDS_Shape shape;
	TopoDS_Face face;

	void RectAlgo() {

		float xo = x;
		float yo = y;

		switch (align) {
		case Align::cc:
			xo -= w / 2;
			yo -= h / 2;
			break;
		case Align::cl:
			xo -= w / 2;
			yo -= h;
			break;
		case Align::lc:
			xo -= w;
			yo -= h / 2;
			break;
		case Align::ch:
			xo -= w / 2;
			break;
		case Align::hc:
			yo -= h / 2;
			break;
		case Align::ll:
			xo -= w;
			yo -= h;
			break;
		case Align::lh:
			xo -= w;
			break;
		case Align::hl:
			yo -= h;
			break;
		case Align::hh:
			break;
		}

		shape = BRepBuilderAPI_MakeFace(gp_Pln(), xo, xo + w, yo, yo + h).Shape();
		face = BRepBuilderAPI_MakeFace(gp_Pln(), xo, xo + w, yo, yo + h).Face();
	}

public:

	Rect(float w, float h, float x = 0, float y = 0, Align align = Align::cc) :
		w(w), h(h), x(x), y(y), align(align)
	{
		RectAlgo();
	}
	Rect(float w, float h, Align align) :
		w(w), h(h), x(0), y(0), align(align)
	{
		RectAlgo();
	}

	operator TopoDS_Shape() const {
		return TopoDS_Shape(shape);
	}
	operator TopoDS_Face() const {
		return TopoDS_Face(face);
	}

};

class Ellipse {
public:
	TopoDS_Shape shape;
	TopoDS_Face face;
	float w, h, x, y;
	gp_Dir dir = gp_Dir(1, 0, 0);

	Ellipse(float w, float h, float x = 0, float y = 0) : w(w), h(h), x(x), y(y) {
		if (h > w) {
			float aux = h;
			h = w;
			w = aux;
			dir.SetXYZ(gp_XYZ(0, 1, 0));
		}
		gp_Ax2 axis(gp_Pnt(x, y, 0), gp_Dir(0, 0, 1), dir);
		gp_Elips elips(axis, w, h);
		Handle(Geom_Ellipse) handle = GC_MakeEllipse(elips).Value();
		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(handle);
		TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);
		face = BRepBuilderAPI_MakeFace(wire).Face();
		shape = BRepBuilderAPI_MakeFace(wire).Shape();
	};

	float getRadAtY(float y) {
		return w * sqrt(1 - (y * y) / (h * h));
	}

	operator TopoDS_Shape() const {
		return TopoDS_Shape(shape);
	}
	operator TopoDS_Face() const {
		return TopoDS_Face(face);
	}

};

class Part; //forward declaration
struct Joint {
	Joint() {};
	Joint(gp_Trsf local, gp_Trsf global, std::string label, Part* part) : local(local), global(global), label(label), part(part) {};

	gp_Trsf local;
	gp_Trsf global;
	std::string label;
	Part* part;
};

class Part {

public:
	const char* label = nullptr;
	TopoDS_Shape shape;
	std::unordered_map<std::string, Joint> joints;
	gp_Trsf transformation;
	gp_Trsf connection;
	std::vector<Part*> connectedParts;

	Part() {}

	Part(const Part& other) {
		label = other.label;
		shape = other.shape;
		transformation = other.transformation;

		// deep copy de los joints
		for (const auto& [jointLabel, joint] : other.joints) {
			joints[jointLabel] = Joint(
				joint.local,
				joint.global,
				joint.label,
				this  // <- apunta al Part copiado
			);
		}

		// connectedParts NO se copia porque los hijos siguen siendo del original
		// Si quer�s copiar la jerarqu�a completa, habr�a que hacer un deep copy recursivo
		connectedParts.clear();
	}

	Part(TopoDS_Shape s) : shape(s) {}

	TopoDS_Shape Shape() {
		return shape;
	}

	void translate(float x = 0, float y = 0, float z = 0) {
		translateLogic(x, y, z);
	}

	void translate(gp_XYZ coords) {
		translateLogic(coords.X(), coords.Y(), coords.Z());
	}

	void rotate(float x = 0, float y = 0, float z = 0) {
		gp_Trsf tr;
		float a = x * M_PI / 180.0f;
		float b = y * M_PI / 180.0f;
		float c = z * M_PI / 180.0f;
		gp_Quaternion q;
		q.SetEulerAngles(gp_Extrinsic_XYZ, a, b, c);
		tr.SetRotationPart(q);

		transformation *= tr;
		shape = shape.Located(TopLoc_Location(transformation));

		for (auto& [label, j] : joints) {
			j.global = transformation * j.local;
		}

		applyParentTransform(transformation);
	}

	void applyParentTransform(const gp_Trsf& parentGlobal) {
		transformation = parentGlobal * connection;
		shape = shape.Located(TopLoc_Location(transformation));
		for (auto& [label, j] : joints) {
			j.global = transformation * j.local;
		}
		for (Part* p : connectedParts) {
			p->applyParentTransform(transformation);
		}
	}

	void addJoint(std::string label, float x, float y, float z = 0, float xr = 0, float yr = 0, float zr = 0) {
		addJointLogic(label, x, y, z, xr, yr, zr);
	}

	void addJoint(std::string label, vec pos, float xr = 0, float yr = 0, float zr = 0) {
		addJointLogic(label, pos.x, pos.y, pos.z, xr, yr, zr);
	}

	void connect(Joint& j1, Joint& j2) {
		Part* parent = j2.part;
		parent->connectedParts.push_back(this);

		transformation = parent->transformation * j2.local * j1.local.Inverted();
		connection = parent->transformation.Inverted() * transformation;
		shape = shape.Located(TopLoc_Location(transformation));

		for (auto& [label, j] : joints)
			j.global = transformation * j.local;
	}

	operator TopoDS_Shape() const {
		return TopoDS_Shape(shape);
	}

private:
	void translateLogic(float x, float y, float z) {
		gp_Trsf tr;
		gp_Vec pos(x, y, z);
		tr.SetTranslationPart(pos);

		transformation *= tr;
		shape = shape.Located(transformation);

		for (auto& [label, j] : joints) {
			j.global = transformation * j.local;
		}
		applyParentTransform(transformation);
		//for (Part* p : connectedParts) {
		//	p->applyParentTransform(transformation);
		//}
	}

	void addJointLogic(std::string label, float x, float y, float z, float xr, float yr, float zr) {
		gp_Trsf tr;

		float a = xr * M_PI / 180.0f;
		float b = yr * M_PI / 180.0f;
		float c = zr * M_PI / 180.0f;
		gp_Quaternion q;
		q.SetEulerAngles(gp_Extrinsic_XYZ, a, b, c);
		tr.SetRotationPart(q);

		tr.SetTranslationPart(gp_Vec(x, y, z));

		Joint j(tr, transformation * tr, label, this);

		joints.insert({ label, j });
	}
};

inline bool equal(float a, float b, float tolerance = 1e-6f) {
	return std::fabs(a - b) < tolerance;
}

inline float distFast(const gp_Pnt& a, const gp_Pnt& b) {
	float dx = a.X() - b.X();
	float dy = a.Y() - b.Y();
	float dz = a.Z() - b.Z();
	return dx * dx + dy * dy + dz * dz;
}

inline bool samePoint(const gp_Pnt& a, const gp_Pnt& b) {
	float tolerance = 1.0E-5;
	return distFast(a, b) < tolerance;
}

inline TopoDS_Shape translate(TopoDS_Shape shape, vec v) {
	gp_Trsf tr;
	tr.SetTranslation(gp_Vec(v.x, v.y, v.z));
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape();
}

inline TopoDS_Shape mirror(TopoDS_Shape shape, vec dir, vec pnt = vec::vec(0, 0, 0)) {
	gp_Trsf tr;
	gp_Ax2 ax(gp_Pnt(pnt.x, pnt.y, pnt.z), gp_Dir(dir.x, dir.y, dir.z));
	tr.SetMirror(ax);
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape();
}

inline TopoDS_Shape fuse(TopTools_ListOfShape* args) {

	BRepAlgoAPI_Fuse fuse;
	fuse.SetRunParallel(true);
	fuse.SetFuzzyValue(1e-5);
	//the original tolerance was 1e-7 and sometimes the parts didn't collide to each other so they don't fuse

	fuse.SetArguments(*args);
	fuse.SetTools(*args);
	fuse.Build();
	fuse.SimplifyResult();
	args->Clear();

	return TopoDS_Shape(fuse.Shape());
}

inline TopoDS_Shape cut(TopTools_ListOfShape* args, TopTools_ListOfShape* tools) {
	BRepAlgoAPI_Cut cut;
	cut.SetRunParallel(true);
	cut.SetArguments(*args);
	cut.SetTools(*tools);
	cut.Build();
	cut.SimplifyResult();
	args->Clear();
	tools->Clear();

	return TopoDS_Shape(cut.Shape());
}

inline TopoDS_Shape fusecut(TopTools_ListOfShape* args, TopTools_ListOfShape* tools) {

	TopoDS_Shape fused = fuse(args);
	args->Append(fused);
	TopoDS_Shape cuted = cut(args, tools);

	return TopoDS_Shape(cuted);
}

inline TopoDS_Shape intersect(TopTools_ListOfShape* args, TopTools_ListOfShape* tools) {
	BRepAlgoAPI_Common common;

	common.SetArguments(*args);
	common.SetTools(*tools);
	common.Build();
	common.SimplifyResult();
	args->Clear();
	tools->Clear();

	return TopoDS_Shape(common.Shape());
}

inline Part extrude(TopoDS_Shape face, float thickness) {
	TopoDS_Shape p = BRepPrimAPI_MakePrism(face, gp_Vec(0, 0, thickness));
	return Part(p);
}

inline std::vector<TopoDS_Vertex> vertices(const TopoDS_Shape& shape) {
	TopExp_Explorer exp(shape, TopAbs_FACE);
	TopoDS_Face face(TopoDS::Face(exp.Current()));
	BRepFilletAPI_MakeFillet2d makeFillet(face);

	TopTools_IndexedMapOfShape vertexMap;
	TopExp::MapShapes(face, TopAbs_VERTEX, vertexMap);

	std::vector<TopoDS_Vertex> uniqueVertices;
	std::unordered_set<const void*> seen;

	for (int i = 1; i <= vertexMap.Extent(); ++i) {
		TopoDS_Vertex v = TopoDS::Vertex(vertexMap(i));
		const void* key = v.TShape().get();

		if (seen.insert(key).second) {
			uniqueVertices.push_back(v);
		}
	}

	return uniqueVertices;
}

inline TopoDS_Shape fillet(TopoDS_Shape shape, std::vector<gp_Pnt> locs, float r) {
	TopExp_Explorer exp(shape, TopAbs_FACE);
	TopoDS_Face face(TopoDS::Face(exp.Current()));
	BRepFilletAPI_MakeFillet2d makeFillet(face);

	const std::vector<TopoDS_Vertex>& vv = vertices(face);

	for (int i = 0; i < vv.size(); i++) {
		gp_Pnt p(BRep_Tool::Pnt(vv[i]));
		for (int j = 0; j < locs.size(); j++) {
			if (p.IsEqual(locs[j], 1e-6f)) {
				makeFillet.AddFillet(vv[i], r);
				break;
			}
		}
	}
	return makeFillet.Shape();
}

inline TopoDS_Shape fillet(TopoDS_Shape shape, std::vector<TopoDS_Vertex> vv, float r) {
	TopExp_Explorer exp(shape, TopAbs_FACE);
	TopoDS_Face face(TopoDS::Face(exp.Current()));
	BRepFilletAPI_MakeFillet2d makeFillet(face);

	for (TopoDS_Vertex v : vv) {
		makeFillet.AddFillet(v, r);
	}
	return makeFillet.Shape();
}

inline void fillet2(TopoDS_Shape& shape, std::vector<TopoDS_Vertex> vv, float r) {

	// getting outer wire
	TopoDS_Wire outerWire;

	for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next()) {
		TopoDS_Face face = TopoDS::Face(expFace.Current());
		TopoDS_Wire outer = ShapeAnalysis::OuterWire(face);
		outerWire = outer;
	}

	//TopTools_ListOfShape outerResult;
	//std::vector<TopTools_ListOfShape*> innerResults;
	TopTools_IndexedMapOfShape outerResultMap;
	std::vector<TopTools_IndexedMapOfShape> innerResultsMap;
	std::vector<int> filletedIndexes;
	std::vector<int> notFilletedIndexes;

	// iterating shape wires
	TopExp_Explorer expWire(shape, TopAbs_WIRE);
	while (expWire.More()) {

		ChFi2d_FilletAPI filletApi;
		//TopTools_ListOfShape *innerResult = new TopTools_ListOfShape();
		TopTools_IndexedMapOfShape innerResultMap;

		// iterating target vertices
		for (int i = 0; i < vv.size(); i++) {

			TopoDS_Edge pair[2];
			gp_Pnt pTarget = BRep_Tool::Pnt(vv[i]);
			int j = 0;
			TopoDS_Edge edge;
			TopExp_Explorer expEdge(expWire.Current(), TopAbs_EDGE);

			// iterating wire edges
			while (expEdge.More()) {

				edge = TopoDS::Edge(expEdge.Current());
				gp_Pnt pfEdge = BRep_Tool::Pnt(TopExp::FirstVertex(edge));
				gp_Pnt plEdge = BRep_Tool::Pnt(TopExp::LastVertex(edge));
				float tol = 1.0E-5;

				// target vertex and any edge end coincide?
				if (distFast(pTarget, pfEdge) < tol || distFast(pTarget, plEdge) < tol) {

					pair[j] = edge;
					j++;

					if (j == 2) {
						filletApi.Init(pair[0], pair[1], gp_Pln(0, 0, 1, 0));
						filletApi.Perform(r);
						if (expWire.Current().IsEqual(outerWire)) {
							outerResultMap.Add(filletApi.Result(pTarget, pair[0], pair[1]));
							outerResultMap.Add(pair[0]);
							outerResultMap.Add(pair[1]);
						}
						else {
							innerResultMap.Add(filletApi.Result(pTarget, pair[0], pair[1]));
							innerResultMap.Add(pair[0]);
							innerResultMap.Add(pair[1]);
						}
						break;
					}
				}
				else {
					if (expWire.Current().IsEqual(outerWire))
						outerResultMap.Add(edge);
					//notFilletedIndexes.push_back({outer,2});
					else {
						innerResultMap.Add(edge);
						notFilletedIndexes.push_back({ innerResultMap.FindIndex(edge) });
					}
				}
				expEdge.Next();
			}
		}
		if (!expWire.Current().IsEqual(outerWire))
			//innerResults.push_back(innerResult);
			innerResultsMap.push_back(innerResultMap);
		expWire.Next();
	}

	//BRepBuilderAPI_MakeWire mkOWire;
	//mkOWire.Add(outerResult);
	//if (!mkOWire.IsDone()) {
	//	return;
	//}

	//BRepBuilderAPI_MakeFace mkFace(mkOWire.Wire());
	//if (!mkFace.IsDone()) {
	//	return;
	//}

	//for (int i = 0; i < innerResults.size(); i++) {
	//	BRepBuilderAPI_MakeWire mkIWire;
	//	mkIWire.Add(*innerResults[i]);
	//	if (mkIWire.IsDone()) {
	//		mkFace.Add(mkIWire.Wire());
	//	}
	//}

	//shape = mkFace.Shape();

	//for (auto p : innerResults)
	//	delete p;
	//innerResults.clear();
}

inline void fillet3(TopoDS_Shape& shape, std::vector<TopoDS_Vertex> vv, float r) {

	std::vector<std::vector<TopoDS_Edge>> edges;
	for (TopExp_Explorer expWire(shape, TopAbs_WIRE); expWire.More(); expWire.Next()) {
		std::vector<TopoDS_Edge> edge;
		for (TopExp_Explorer expEdge(expWire.Current(), TopAbs_EDGE); expEdge.More(); expEdge.Next()) {
			edge.push_back(TopoDS::Edge(expEdge.Current()));
		}
		edges.push_back(edge);
	}

	TopoDS_Edge pair[2];
	std::vector<TopoDS_Edge*> toDelete;
	std::vector<std::vector<TopoDS_Edge>> results;

	for (int i = 0; i < vv.size(); i++) {
		for (int j = 0; j < edges.size(); j++) {
			std::vector<TopoDS_Edge> result;
			int count = 0;
			for (int k = 0; k < edges[j].size(); k++) {

				gp_Pnt pTarget = BRep_Tool::Pnt(vv[i]);
				gp_Pnt pFirst = BRep_Tool::Pnt(TopExp::FirstVertex(edges[j][k]));
				gp_Pnt pLast = BRep_Tool::Pnt(TopExp::LastVertex(edges[j][k]));

				if (samePoint(pTarget, pFirst) || samePoint(pTarget, pLast)) {

					pair[count] = edges[j][k];

					// Don't duplicate same edge in toDelete:
					bool duplicated = false;
					for (auto ptr : toDelete) {
						if (ptr == &edges[j][k]) {
							duplicated = true;
							break;
						}
					}
					if (!duplicated)
						toDelete.push_back(&edges[j][k]);

					count++;
					if (count == 2) {
						ChFi2d_FilletAPI filletApi(pair[0], pair[1], gp_Pln(0, 0, 1, 0));
						filletApi.Perform(r);
						result.push_back(filletApi.Result(pTarget, pair[0], pair[1]));
						result.push_back(pair[0]);
						result.push_back(pair[1]);
						break;
					}
				}
			}
			results.push_back(result);
		}
	}

	// Erase original edges that have now a modified replacement in "results"
	for (int i = 0; i < edges.size(); i++) {
		for (int j = 0; j < edges[i].size(); j++) {
			for (auto ptr : toDelete) {
				if (ptr == &edges[i][j]) {
					edges[i].erase(edges[i].begin() + j);
					j--;
					break;
				}
			}
		}
	}

	// Combine resultant edges with original not modified edges
	std::vector<TopoDS_Wire> wires;
	BRepBuilderAPI_MakeWire mkWire;
	for (int i = 0; i < edges.size(); i++) {
		for(int j = 0; j < edges[i].size(); j++) {
			mkWire.Add(edges[i][j]);
		}
		for (int j = 0; j < results[i].size(); j++) {
			mkWire.Add(results[i][j]);
		}
		wires.push_back(TopoDS::Wire(mkWire.Shape()));
	}
}

inline std::vector<std::vector<TopoDS_Vertex>> groupBy(std::vector<TopoDS_Vertex> vv, Axis axis) {

	std::vector<std::vector<TopoDS_Vertex>> outList;

	std::sort(vv.begin(), vv.end(), [axis](const TopoDS_Vertex& a, const TopoDS_Vertex& b) {
		gp_Pnt ap = BRep_Tool::Pnt(a);
		gp_Pnt bp = BRep_Tool::Pnt(b);
		switch (axis)
		{
		case Axis::x:
			return ap.X() < bp.X();
			break;
		case Axis::y:
			return ap.Y() < bp.Y();
			break;
		case Axis::z:
			return ap.Z() < bp.Z();
			break;
		default:
			break;
		}
		});

	std::vector<TopoDS_Vertex> group;
	for (int i = 0; i < vv.size() - 1; i++) {
		gp_Pnt p1 = BRep_Tool::Pnt(vv[i]);
		gp_Pnt p2 = BRep_Tool::Pnt(vv[i + 1]);

		if (group.empty())
			group.push_back(vv[i]);

		switch (axis)
		{
		case Axis::x:
			if (equal(p1.X(), p2.X())) {
				group.push_back(vv[i + 1]);
				if (i + 2 == vv.size())
					outList.push_back(group);
				continue;
			}
			break;
		case Axis::y:
			if (equal(p1.Y(), p2.Y())) {
				group.push_back(vv[i + 1]);
				if (i + 2 == vv.size())
					outList.push_back(group);
				continue;
			}
			break;
		case Axis::z:
			if (equal(p1.Z(), p2.Z())) {
				group.push_back(vv[i + 1]);
				if (i + 2 == vv.size())
					outList.push_back(group);
				continue;
			}
			break;
		default:
			break;
		}
		// Here we could detect which Axis is the group in, and then sortBy() according to that, so the groupBY also performs a sorting for each group.

		outList.push_back(group);
		group.clear();
	}

	return outList;
}

inline void sortBy(std::vector<TopoDS_Vertex>& vv, Axis axis) {
	std::sort(vv.begin(), vv.end(), [axis](const TopoDS_Vertex& a, const TopoDS_Vertex& b) {
		gp_Pnt ap = BRep_Tool::Pnt(a);
		gp_Pnt bp = BRep_Tool::Pnt(b);
		switch (axis)
		{
		case Axis::x:
			return ap.X() < bp.X();
			break;
		case Axis::y:
			return ap.Y() < bp.Y();
			break;
		case Axis::z:
			return ap.Z() < bp.Z();
			break;
		default:
			break;
		}
		});
}

inline TopoDS_Shape tab(float tabWidth, float tabHeight, float slideThickness, float slideLength) {
	Rect tabBase(slideThickness + tabWidth, tabHeight, Align::hh);
	Rect tabCut(slideThickness, slideLength, Align::hh);

	args.Append(tabBase);
	tools.Append(tabCut);

	return fusecut(&args, &tools);
}
