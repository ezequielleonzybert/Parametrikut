#include <TopoDS_Shape.hxx>
#include <BrepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>

struct vec {
	float x;
	float y;
	float z;
	vec() :
		x(0), y(0), z(0) {}
	vec(float x, float y, float z = 0) : 
		x(x), y(y), z(z) {}
};

//
//inline TopoDS_Shape operator+(const TopoDS_Shape& a, const TopoDS_Shape& b) {
//    return BRepAlgoAPI_Fuse(a, b);
//}

TopoDS_Shape translate(TopoDS_Shape shape, vec v) {
	gp_Trsf tr;
	tr.SetTranslation(gp_Vec(v.x,v.y,v.z));
	BRepBuilderAPI_Transform result(shape, tr);
	return result.Shape();
}

TopoDS_Shape mirror(TopoDS_Shape shape, vec pnt, vec dir) {
	gp_Trsf tr;
	gp_Ax2 ax(gp_Pnt(pnt.x, pnt.y, pnt.z), gp_Dir(dir.x, dir.y, dir.z));
	tr.SetMirror(ax);
	BRepBuilderAPI_Transform result(shape, tr);
	return result.Shape();
}