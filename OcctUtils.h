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
#include <ShapeFix.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_EdgeConnect.hxx>
#include <TopOpeBRepBuild_FuseFace.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ReShape.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <BRepLib.hxx>

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
	Standard_Real x;
	Standard_Real y;
	Standard_Real z;
	vec() :
		x(0), y(0), z(0) {
	}
	vec(Standard_Real x, Standard_Real y, Standard_Real z = 0) :
		x(x), y(y), z(z) {
	}
	void set(Standard_Real x, Standard_Real y, Standard_Real z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class Triangle {
public:
	TopoDS_Face face;
	TopoDS_Shape shape;
	Triangle(Standard_Real height, Standard_Real width, Standard_Real x = 0, Standard_Real y = 0) {
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
	Standard_Real w, h, x, y;
	TopoDS_Shape shape;
	TopoDS_Face face;
	TopoDS_Wire wire;

private:

	Align align;

	void RectAlgo() {

		Standard_Real xo = x;
		Standard_Real yo = y;

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

		TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(gp_Pnt(xo, yo, 0), gp_Pnt(xo+w, yo, 0));
		TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(gp_Pnt(xo+w, yo, 0), gp_Pnt(xo+w, yo+h, 0));
		TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(gp_Pnt(xo+w, yo+h, 0), gp_Pnt(xo, yo+h, 0));
		TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(gp_Pnt(xo, yo+h, 0), gp_Pnt(xo, yo, 0));

		wire = BRepBuilderAPI_MakeWire(e1, e2, e3, e4).Wire();

		face = BRepBuilderAPI_MakeFace(wire).Face();

		shape = face;
	}

public:

	Rect(Standard_Real w, Standard_Real h, Standard_Real x = 0, Standard_Real y = 0, Align align = Align::cc) :
		w(w), h(h), x(x), y(y), align(align)
	{
		RectAlgo();
	}
	Rect(Standard_Real w, Standard_Real h, Align align) :
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
	Standard_Real w, h, x, y;
	gp_Dir dir = gp_Dir(1, 0, 0);

	Ellipse(Standard_Real w, Standard_Real h, Standard_Real x = 0, Standard_Real y = 0) : w(w), h(h), x(x), y(y) {
		if (h > w) {
			Standard_Real aux = h;
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

	Standard_Real getRadAtY(Standard_Real y) {
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

	void translate(Standard_Real x = 0, Standard_Real y = 0, Standard_Real z = 0) {
		translateLogic(x, y, z);
	}

	void translate(gp_XYZ coords) {
		translateLogic(coords.X(), coords.Y(), coords.Z());
	}

	void rotate(Standard_Real x = 0, Standard_Real y = 0, Standard_Real z = 0) {
		gp_Trsf tr;
		Standard_Real a = x * M_PI / 180.0f;
		Standard_Real b = y * M_PI / 180.0f;
		Standard_Real c = z * M_PI / 180.0f;
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

	void addJoint(std::string label, Standard_Real x, Standard_Real y, Standard_Real z = 0, Standard_Real xr = 0, Standard_Real yr = 0, Standard_Real zr = 0) {
		addJointLogic(label, x, y, z, xr, yr, zr);
	}

	void addJoint(std::string label, vec pos, Standard_Real xr = 0, Standard_Real yr = 0, Standard_Real zr = 0) {
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
	void translateLogic(Standard_Real x, Standard_Real y, Standard_Real z) {
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

	void addJointLogic(std::string label, Standard_Real x, Standard_Real y, Standard_Real z, Standard_Real xr, Standard_Real yr, Standard_Real zr) {
		gp_Trsf tr;

		Standard_Real a = xr * M_PI / 180.0f;
		Standard_Real b = yr * M_PI / 180.0f;
		Standard_Real c = zr * M_PI / 180.0f;
		gp_Quaternion q;
		q.SetEulerAngles(gp_Extrinsic_XYZ, a, b, c);
		tr.SetRotationPart(q);

		tr.SetTranslationPart(gp_Vec(x, y, z));

		Joint j(tr, transformation * tr, label, this);

		joints.insert({ label, j });
	}
};

inline bool equal(Standard_Real a, Standard_Real b, Standard_Real tolerance = 1e-6f) {
	return std::fabs(a - b) < tolerance;
}

inline Standard_Real distFast(const gp_Pnt& a, const gp_Pnt& b) {
	Standard_Real dx = a.X() - b.X();
	Standard_Real dy = a.Y() - b.Y();
	Standard_Real dz = a.Z() - b.Z();
	return dx * dx + dy * dy + dz * dz;
}

inline bool samePoint(const gp_Pnt& a, const gp_Pnt& b) {
	Standard_Real tolerance = 1.0E-5;
	return distFast(a, b) < tolerance;
}

inline TopoDS_Vertex makeVertex(Standard_Real x, Standard_Real y, Standard_Real z = 0) {
	gp_Pnt p(x, y, z);
	BRepBuilderAPI_MakeVertex mkVertex(p);
	return mkVertex.Vertex();
}

inline TopoDS_Edge deepReverse(const TopoDS_Edge& edge) {
	Standard_Real f, l;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

	Handle(Geom_Curve) reversed = curve->Reversed();

	BRepBuilderAPI_MakeEdge mk(reversed, -l, -f);
	TopoDS_Edge newEdge = mk.Edge();

	return newEdge;
}

inline TopoDS_Edge redrawEdge(const TopoDS_Edge& edge) {
	Standard_Real f, l;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
	BRepBuilderAPI_MakeEdge mkEdge(curve, f, l);
	return mkEdge.Edge();
}

inline TopoDS_Shape regenShape(const TopoDS_Shape& shape) {

	TopExp_Explorer expFace(shape, TopAbs_FACE);
	TopoDS_Face face = TopoDS::Face(expFace.Current());

	TopoDS_Wire outerWire = BRepTools::OuterWire(face);
	std::vector <std::vector<TopoDS_Edge>> wires;

	for (TopExp_Explorer expWire(face, TopAbs_WIRE); expWire.More(); expWire.Next()) {

		std::vector<TopoDS_Edge> wire;
		for (BRepTools_WireExplorer expEdge(TopoDS::Wire(expWire.Current())); expEdge.More(); expEdge.Next()) {

			TopoDS_Edge newEdge = TopoDS::Edge(expEdge.Current());
			if (newEdge.Orientation() == TopAbs_REVERSED) {
				newEdge = deepReverse(newEdge);
			}
			else {
				newEdge = redrawEdge(newEdge);
			}
			wire.push_back(newEdge);
		}
		if (expWire.Current().IsEqual(outerWire))
			wires.insert(wires.begin(), wire);
		else
			wires.push_back(wire);
	}

	std::vector<TopoDS_Wire> newWires;
	for (auto& wire : wires) {
		BRepBuilderAPI_MakeWire mkWire;
		for (auto& edge : wire) {
			mkWire.Add(edge);
		}
		newWires.push_back(mkWire.Wire());
	}

	BRepBuilderAPI_MakeFace mkFace(newWires[0]);
	newWires.erase(newWires.begin());
	for (auto& wire : newWires) {
		mkFace.Add(wire);
	}

	return mkFace.Shape();
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
	return result.Shape().Reversed();
}

inline TopoDS_Shape fuse(TopTools_ListOfShape* args) {

	BRepAlgoAPI_Fuse fuse;
	fuse.SetRunParallel(true);
	fuse.SetFuzzyValue(1);
	//the original tolerance was 1e-7 and sometimes the parts didn't collide to each other so they don't fuse

	fuse.SetArguments(*args);
	fuse.SetTools(*args);
	fuse.Build();
	fuse.SimplifyResult();
	args->Clear();

	TopoDS_Shape shape = fuse.Shape();

	return regenShape(shape);
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
	TopoDS_Shape shape = cut.Shape();
	
	return regenShape(shape);
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

	TopoDS_Shape shape = common.Shape();

	return regenShape(shape);
}

inline Part extrude(TopoDS_Shape face, Standard_Real thickness) {
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

inline TopoDS_Shape filletSimple(TopoDS_Shape shape, std::vector<gp_Pnt> locs, Standard_Real r) {
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

inline TopoDS_Shape filletSimple(TopoDS_Shape shape, std::vector<TopoDS_Vertex> vv, Standard_Real r) {
	TopExp_Explorer exp(shape, TopAbs_FACE);
	TopoDS_Face face(TopoDS::Face(exp.Current()));
	BRepFilletAPI_MakeFillet2d makeFillet(face);

	for (TopoDS_Vertex v : vv) {
		makeFillet.AddFillet(v, r);
	}
	return makeFillet.Shape();
}

inline TopoDS_Shape fillet(TopoDS_Shape& shape, std::vector<TopoDS_Vertex> vv, Standard_Real radius) {
		
	// Generate a vector of vectors with with wires of edges
	std::vector<std::vector<TopoDS_Edge>> wires;
	for (TopExp_Explorer expFace(shape, TopAbs_WIRE); expFace.More(); expFace.Next()) {
		TopoDS_Wire wire = TopoDS::Wire(expFace.Current());

		std::vector<TopoDS_Edge> edges;
		for (BRepTools_WireExplorer expWire(wire); expWire.More(); expWire.Next()) {
			TopoDS_Edge edge = TopoDS::Edge(expWire.Current());
			edges.push_back(edge);
		}
		wires.push_back(edges);
	}

	int edgeIdxPair[2];

	// Apply the fillet to the selected vertices
	for (int targetIdx = 0; targetIdx < vv.size(); targetIdx++) {
		bool done = false;
		for (int wireIdx = 0; wireIdx < wires.size() && !done; wireIdx++) {
			int count = 0;
			for (int edgeIdx = 0; edgeIdx < wires[wireIdx].size(); edgeIdx++) {

				gp_Pnt pTarget = BRep_Tool::Pnt(vv[targetIdx]);
				gp_Pnt pFirst = BRep_Tool::Pnt(TopExp::FirstVertex(wires[wireIdx][edgeIdx]));
				gp_Pnt pLast = BRep_Tool::Pnt(TopExp::LastVertex(wires[wireIdx][edgeIdx]));

				if (samePoint(pTarget, pFirst) || samePoint(pTarget, pLast)) {

					edgeIdxPair[count] = edgeIdx;
					count++;

					// Make the fillet when finding the second edge
					if (count == 2) {

						TopoDS_Edge* e1 = &wires[wireIdx][edgeIdxPair[0]];
						TopoDS_Edge* e2 = &wires[wireIdx][edgeIdxPair[1]];

						TopoDS_Edge m1 = *e1;
						TopoDS_Edge m2 = *e1;//testing

						ChFi2d_FilletAPI filletApi(*e1, *e2, gp_Pln(0, 0, 1, 0));
						filletApi.Perform(radius);
						TopoDS_Edge filletedEdge = filletApi.Result(pTarget, *e1, *e2);

						wires[wireIdx].insert(wires[wireIdx].begin() + edgeIdxPair[1], filletedEdge);

						done = true;
						break;
					}
				}
			}
		}
	}

	//this is just for testing the edges that are giving problems
	//TopoDS_Compound comp;
	//BRep_Builder builder;
	//builder.MakeCompound(comp);
	//for(int i = 0; i < wires[0].size(); i++){
	//	builder.Add(comp, wires[0][i]);
	//}
	//return comp;

	// Build the wires
	TopoDS_Wire outerWire;
	std::vector<TopoDS_Wire> innerWires;
	Standard_Real lastArea = 0;
	for (int i = 0; i < wires.size(); i++) {
		BRepBuilderAPI_MakeWire mkWire;

		for (int j = 0; j < wires[i].size(); j++) {
			mkWire.Add(wires[i][j]);
		}

		// Make wires and Identify the outerWire
		TopoDS_Wire wire = mkWire.Wire();
		const Standard_Real currentArea = ShapeAnalysis::ContourArea(wire);
		if (currentArea > lastArea) {
			lastArea = currentArea;
			if (!outerWire.IsNull()) {
				innerWires.push_back(outerWire);
			}
			outerWire = wire;
		}
		else {
			innerWires.push_back(TopoDS::Wire(wire));
		}
	}

	// Make the resultant face from the built wires
	BRepBuilderAPI_MakeFace mkFace(outerWire);
	for (auto& wire : innerWires) {
		mkFace.Add(wire);
	}
	return mkFace.Shape();
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

inline TopoDS_Shape tab(Standard_Real tabWidth, Standard_Real tabHeight, Standard_Real slideThickness, Standard_Real slideLength, bool turn=false) {
	TopTools_ListOfShape args;
	TopTools_ListOfShape tools;

	Align align = Align::hh;
	if (turn) align = Align::hl;

	Rect tabBase(slideThickness + tabWidth, tabHeight, align);
	Rect tabCut(slideThickness, slideLength, align);

	args.Append(tabBase);
	tools.Append(tabCut);

	return cut(&args, &tools);
}

inline TopoDS_Compound pack(std::vector<TopoDS_Shape> shapes) {
	return TopoDS_Compound();
}