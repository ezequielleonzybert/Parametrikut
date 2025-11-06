#include "Exporter.h"
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Bnd_Box.hxx>
#include <QFile>
#include <QTextStream>

void Exporter::add(const std::vector<std::vector<TopoDS_Edge>>& edges) {
    this->edges = edges;

    Bnd_Box globalBox;
    for (const auto& ee : edges) {
        for (auto& e : ee) {
            BRepBndLib::Add(e, globalBox);
        }
    }

    gp_Pnt pMin = globalBox.CornerMin();
    gp_Pnt pMax = globalBox.CornerMax();

    minX = static_cast<float>(pMin.X());
    maxX = static_cast<float>(pMax.X());
    minY = static_cast<float>(pMin.Y());
    maxY = static_cast<float>(pMax.Y());

    width = maxX - minX;
    height = maxY - minY;
}

bool Exporter::exportToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&file);
    // SVG con tamaño y viewBox correctos
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
        << "width=\"" << width << "mm\" height=\"" << height << "mm\" "
        << "viewBox=\"0 0 " << width << " " << height << "\">\n";

    for (auto& ee : edges) {
        out << "<path d=\"M";
        for (int i = 0; i < ee.size(); i++) {

            Standard_Real f, l;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(ee[i], f, l);
            if (curve.IsNull()) continue;

            GeomAdaptor_Curve adaptor(curve, f, l);
            GeomAbs_CurveType type = adaptor.GetType();

            gp_Pnt p1 = adaptor.Value(f);
            gp_Pnt p2 = adaptor.Value(l);

            float x1 = static_cast<float>(p1.X() - minX);
            float y1 = static_cast<float>(height - (p1.Y() - minY));
            float x2 = static_cast<float>(p2.X() - minX);
            float y2 = static_cast<float>(height - (p2.Y() - minY));

            if (i == 0) {
                out << x1 << " " << y1;
            }

            switch (type) {
            case GeomAbs_Line: {
                out << "L" << x2 << " " << y2;
                break;
            }
            case GeomAbs_Circle: {
                gp_Circ circ = adaptor.Circle();

                float r = static_cast<float>(circ.Radius());

                int largeArc = ((l - f) > M_PI) ? 1 : 0;
                int sweep = 0;

                out << "A" << r << " " << r << " 0 "
                    << largeArc << " " << sweep << " "
                    << x2 << " " << y2;
                break;
            }
            case GeomAbs_Ellipse:
                break;
            default:
                break;
            }
        }
        out << "Z\" style=\"fill:none;stroke:black;stroke-width:0.5\"/>\n";
    }

    out << "</svg>";
    file.close();
    return true;
}
