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

inline TopTools_ListOfShape args;
inline TopTools_ListOfShape tools;

enum class Axis {
	x,
	y,
	z
};

enum class Align{
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
		x(0), y(0), z(0) {}
	vec(float x, float y, float z = 0) : 
		x(x), y(y), z(z) {}
	void set(float x, float y, float z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class Rectangle {

public:
	float w, h, x, y;

private:

	Align align;
	TopoDS_Shape shape;
	TopoDS_Face face;

	void RectangleAlgo() {

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

	Rectangle(float w, float h, float x = 0, float y = 0, Align align = Align::cc) :
		w(w), h(h), x(x), y(y), align(align)
	{
		RectangleAlgo();
	}
	Rectangle(float w, float h, Align align) :
		w(w), h(h), x(0), y(0), align(align)
	{
		RectangleAlgo();
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
	gp_Dir dir = gp_Dir(1,0,0);

	Ellipse(float w, float h, float x = 0, float y = 0) : w(w), h(h), x(x), y(y) {
		if (h > w) {
			float aux = h;
			h = w;
			w = aux;
			dir.SetXYZ(gp_XYZ(0, 1, 0));
		}
		gp_Ax2 axis(gp_Pnt(), gp_Dir(0,0,1), dir);
		gp_Elips elips(axis,w,h);
		Handle(Geom_Ellipse) handle = GC_MakeEllipse(elips).Value();
		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(handle);
		TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);
		face = BRepBuilderAPI_MakeFace(wire).Face();
		shape = BRepBuilderAPI_MakeFace(wire).Shape();
	};

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
	TopoDS_Shape shape;
	std::unordered_map<std::string, Joint> joints;
	gp_Trsf transformation;
	gp_Trsf connection;
	std::vector<Part*> connectedParts;

	Part() {}

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

		for (auto& j : joints) {
			j.second.global = transformation * j.second.local;
		}

		for (Part* p : connectedParts) {
			p->applyParentTransform(transformation);
		}
	}

	void applyParentTransform(const gp_Trsf& parentGlobal) {
		// recalcula la transform del hijo en base a la transform global del padre
		transformation = parentGlobal * connection;
		shape = shape.Located(TopLoc_Location(transformation));

		for (auto& j : joints) {
			j.second.global = transformation * j.second.local;
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
		// j1 = joint en este (hijo), j2 = joint en el otro (padre)
		Part* parent = j2.part;
		parent->connectedParts.push_back(this);

		gp_Trsf T_p = parent->transformation;
		// T_child_new = T_parent * j2.local * inverse(j1.local)
		gp_Trsf T_c_new = T_p * j2.local * j1.local.Inverted();

		// connection = inverse(T_parent) * T_child_new
		connection = T_p.Inverted() * T_c_new;

		// aplicar la transform resultante al hijo
		transformation = T_c_new;
		shape = shape.Located(TopLoc_Location(transformation));

		for (auto& j : joints) {
			j.second.global = transformation * j.second.local;
		}
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

		for (auto& j : joints) {
			j.second.global = transformation * j.second.local;
		}

		for (Part* p : connectedParts) {
			p->applyParentTransform(transformation);
		}
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

		joints.insert({label, j});
	}
};

inline bool equal(float a, float b, float tolerance = 1e-6f) {
	return std::fabs(a - b) < tolerance;
}

inline TopoDS_Shape translate(TopoDS_Shape shape, vec v) {
	gp_Trsf tr;
	tr.SetTranslation(gp_Vec(v.x,v.y,v.z));
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape();
}

inline TopoDS_Shape mirror(TopoDS_Shape shape, vec dir, vec pnt = vec::vec(0,0,0) ) {
	gp_Trsf tr;
	gp_Ax2 ax(gp_Pnt(pnt.x, pnt.y, pnt.z), gp_Dir(dir.x, dir.y, dir.z));
	tr.SetMirror(ax);
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape().Reversed();
}

inline TopoDS_Shape fuse(TopTools_ListOfShape* args) {

	BRepAlgoAPI_Fuse fuse;
	fuse.SetRunParallel(true);

	fuse.SetArguments(*args);
	fuse.SetTools(*args);
	fuse.Build();
	fuse.SimplifyResult();
	args->Clear();

	return TopoDS_Shape(fuse.Shape());
}

inline TopoDS_Shape fusecut(TopTools_ListOfShape *args, TopTools_ListOfShape *tools) {

	BRepAlgoAPI_Fuse fuse;
	BRepAlgoAPI_Cut cut;
	cut.SetRunParallel(true);
	fuse.SetRunParallel(true);

	fuse.SetArguments(*args);
	fuse.SetTools(*args);
	fuse.Build();
	fuse.SimplifyResult();
	TopoDS_Shape fused = fuse.Shape();
	args->Clear();
	args->Append(fused);
	cut.SetArguments(*args);
	cut.SetTools(*tools);
	cut.Build();
	cut.SimplifyResult();
	args->Clear();
	tools->Clear();

	return TopoDS_Shape(cut.Shape());
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
	TopoDS_Shape p = BRepPrimAPI_MakePrism(face, gp_Vec(0,0,thickness));
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
	for (int i = 0; i < vv.size()-1; i++) {
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