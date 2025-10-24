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
#include <BRepBndLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_BSplineCurve.hxx>

void Exporter::add(const std::vector<TopoDS_Wire>& wires) {
    this->wires = wires;

    Bnd_Box globalBox;

    for (const auto& wire : wires) {
        BRepBndLib::AddClose(wire, globalBox);
    }

    gp_Pnt pMin = globalBox.CornerMin();
    gp_Pnt pMax = globalBox.CornerMax();

    float minX = pMin.X();
    float maxX = pMax.X();
    float minY = pMin.Y();
    float maxY = pMax.Y();

    width = maxX - minX;
    height = maxY - minY;
}

bool Exporter::exportToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&file);
    out << "<svg xmlns='http://www.w3.org/2000/svg' version='1.1'>\n";

    for(auto& wire : wires){
        out << "<path d='";
        for (BRepTools_WireExplorer we(wire); we.More(); we.Next()) {
            TopoDS_Edge edge = we.Current();
            Standard_Real f, l;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
            if (curve.IsNull()) continue;

            BRepAdaptor_Curve adapt(edge);
            gp_Pnt p1 = adapt.Value(f);
            gp_Pnt p2 = adapt.Value(l);
            out << "M " << p1.X() << " " << -p1.Y()+height << " L " << p2.X() << " " << -p2.Y()+height << " ";
        }
        out << "' stroke='black' fill='none'/>\n";
    }

    out << "</svg>";
    file.close();
    return true;
}
