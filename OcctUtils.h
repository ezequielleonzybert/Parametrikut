#include <TopoDS_Shape.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

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

TopoDS_Shape OuterWire(std::vector<TopoDS_Wire> wires) {

	std::vector<TopoDS_Edge> edges;

	for (TopoDS_Wire w : wires) {
		BRepTools_WireExplorer wireExp;
		for (wireExp.Init(w); wireExp.More(); wireExp.Next()) {
			edges.push_back(wireExp.Current());
		}
	}
	
	BRepBuilderAPI_Sewing sewing;

	for (TopoDS_Edge e : edges) {
		sewing.Add(e);
	}
	sewing.Perform();
	sewing.Dump();

	return sewing.SewedShape();
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

TopoDS_Shape fusecut(TopTools_ListOfShape args, TopTools_ListOfShape tools) {

	BRepAlgoAPI_Fuse fuse;
	BRepAlgoAPI_Cut cut;
	cut.SetRunParallel(true);
	fuse.SetRunParallel(true);

	fuse.SetArguments(args);
	fuse.SetTools(args);
	fuse.Build();
	fuse.SimplifyResult();
	TopoDS_Shape fused = fuse.Shape();
	args.Clear();
	args.Append(fused);
	cut.SetArguments(args);
	cut.SetTools(tools);
	cut.Build();

	return TopoDS_Shape(cut.Shape());
}

class Rectangle {

public:
	float w, h, x, y;

private:

	Align align;
	TopoDS_Shape shape;

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

		shape = BRepBuilderAPI_MakeFace(gp_Pln(), xo, xo + w, yo, yo + h).Face();
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
};