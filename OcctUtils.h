#include <TopoDS_Shape.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
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

TopTools_ListOfShape args;
TopTools_ListOfShape tools;

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
};

TopoDS_Shape translate(TopoDS_Shape shape, vec v) {
	gp_Trsf tr;
	tr.SetTranslation(gp_Vec(v.x,v.y,v.z));
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape();
}

TopoDS_Shape mirror(TopoDS_Shape shape, vec dir, vec pnt = vec::vec(0,0,0) ) {
	gp_Trsf tr;
	gp_Ax2 ax(gp_Pnt(pnt.x, pnt.y, pnt.z), gp_Dir(dir.x, dir.y, dir.z));
	tr.SetMirror(ax);
	BRepBuilderAPI_Transform result(shape, tr, true);
	return result.Shape().Reversed();
}

TopoDS_Shape fuse(TopTools_ListOfShape* args) {

	BRepAlgoAPI_Fuse fuse;
	fuse.SetRunParallel(true);

	fuse.SetArguments(*args);
	fuse.SetTools(*args);
	fuse.Build();
	fuse.SimplifyResult();

	return TopoDS_Shape(fuse.Shape());
}

TopoDS_Shape fusecut(TopTools_ListOfShape *args, TopTools_ListOfShape *tools) {

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

TopoDS_Shape extrude(TopoDS_Shape face, float thickness) {
	return BRepPrimAPI_MakePrism(face, gp_Vec(0,0,thickness));
}

TopoDS_Shape fillet(TopoDS_Face face, float r) {
	BRepFilletAPI_MakeFillet2d makeFillet(face);
	TopExp_Explorer exp(face, TopAbs_VERTEX);

	// Use TShape pointer as hash key
	std::unordered_set<const void*> unique_vertices;


	while (exp.More()) {
		TopoDS_Vertex v = TopoDS::Vertex(exp.Current());
		const void* tshape_ptr = v.TShape().get(); // get() gives raw pointer

		if (unique_vertices.insert(tshape_ptr).second) {
			// Only add if not already present
			makeFillet.AddFillet(v, r);
		}
		exp.Next();
	}
	return makeFillet.Shape();
}

std::vector<TopoDS_Vertex> vertices(TopoDS_Face face) {
	TopTools_IndexedMapOfShape vertexMap;
	TopExp::MapShapes(face, TopAbs_VERTEX, vertexMap);

	std::vector<TopoDS_Vertex> vertices;
	for (int i = 1; i <= vertexMap.Extent(); ++i) {
		vertices.push_back(TopoDS::Vertex(vertexMap(i)));
	}

	return vertices;
}


TopoDS_Shape FilletFace(const Standard_Real a,
	const Standard_Real  b,
	const Standard_Real c,
	const Standard_Real  r)

{
	TopoDS_Solid Box = BRepPrimAPI_MakeBox(a, b, c);
	TopExp_Explorer  ex1(Box, TopAbs_FACE);

	const  TopoDS_Face& F = TopoDS::Face(ex1.Current());
	BRepFilletAPI_MakeFillet2d MF(F);
	TopExp_Explorer ex2(F, TopAbs_VERTEX);
	int i = 0;
	while (ex2.More())
	{
		i++;
		MF.AddFillet(TopoDS::Vertex(ex2.Current()), r);
		ex2.Next();
		ex2.Next();
	}
	return MF.Shape();
}

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