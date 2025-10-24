#include "exporter.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <gp_Pnt.hxx>

void Exporter::setShape(const TopoDS_Shape shape) {
    this->shape = shape;
}

bool Exporter::exportToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&file);
    out << "<svg xmlns='http://www.w3.org/2000/svg' version='1.1'>\n";

    for (TopExp_Explorer expWire(shape, TopAbs_WIRE); expWire.More(); expWire.Next()) {
        TopoDS_Wire wire = TopoDS::Wire(expWire.Current());
        out << "<path d='";
        for (BRepTools_WireExplorer we(wire); we.More(); we.Next()) {
            TopoDS_Edge edge = we.Current();
            Standard_Real f, l;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
            if (curve.IsNull()) continue;

            BRepAdaptor_Curve adapt(edge);
            gp_Pnt p1 = adapt.Value(f);
            gp_Pnt p2 = adapt.Value(l);
            out << "M " << p1.X() << " " << p1.Y() << " L " << p2.X() << " " << p2.Y() << " ";
        }
        out << "' stroke='black' fill='none'/>\n";
    }

    out << "</svg>";
    file.close();
    return true;
}
