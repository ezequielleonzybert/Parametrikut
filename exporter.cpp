#include "Exporter.h"
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Bnd_Box.hxx>
#include <QFile>
#include <QTextStream>

Exporter::Exporter(Assembly* assembly, QString filepath) {
    add(assembly);
    exportToFile(filepath);
}

void Exporter::add(Assembly* assembly) {
    for (auto& part : assembly->parts) {
        this->edges.push_back(part.edges);

        PartSketch ps;
        ps.edges = part.edges;
        for (auto& ee : ps.edges) {
            for (auto& e : ee) {
                BRepBndLib::Add(e, ps.bb);
            }
        }
        gp_Pnt min = ps.bb.CornerMin();
        gp_Pnt max = ps.bb.CornerMax();
        ps.width = max.X() - min.X();
        ps.height = max.Y() - min.Y();

        ps.translation = gp_Pnt(-min.X(), -max.Y(), 0);
        partSketches.push_back(ps);
    }

    // Sort from largest to smallest part
    std::sort(partSketches.begin(), partSketches.end(),
        [](const PartSketch& a, const PartSketch& b) {
            return std::max(a.width, a.height) > std::max(b.width, b.height);
        }
    );

    // Arrange
    Standard_Real totalW = partSketches[0].width;
    Standard_Real totalH = partSketches[0].height;
    Standard_Real placeX = 0, placeY = 0;
    Standard_Real spaceX = 0, spaceY = 0;
    Standard_Real margin = 10;
    char lastMargin;

    for (int i = 1; i < partSketches.size(); i++) {

        Standard_Real w = partSketches[i].width;
        Standard_Real h = partSketches[i].height;

        if (spaceX > w && spaceY > h) {
            placeX = totalW - spaceX;
            placeY = totalH - spaceY;
            if (lastMargin == 'x') placeY += margin;
            if (lastMargin == 'y') placeX += margin;
            if (w > h) {
                spaceY -= h + margin;
            }
            else {
                spaceX -= w + margin;
            }
        }
        else if (totalW < totalH) {
            placeX = totalW + margin;
            placeY = 0;
            totalW += w + margin;
            spaceY = totalH - h;
            spaceX = w;
            lastMargin = 'x';
        }
        else {
            placeY = totalH + margin;
            placeX = 0;
            totalH += h + margin;
            spaceX = totalW - w;
            spaceY = h;
            lastMargin = 'y';
        }
        Standard_Real x = partSketches[i].translation.X();
        Standard_Real y = partSketches[i].translation.Y();
        partSketches[i].translation.SetX(x + placeX);
        partSketches[i].translation.SetY(y - placeY);
    }

    Bnd_Box globalBox;

    for (auto& ps : partSketches) {
        for (auto& ee : ps.edges) {
            for (auto& e : ee) { 
                gp_Trsf tr;
                tr.SetTranslation(gp_Vec(ps.translation.X(), ps.translation.Y(), 0));
                TopLoc_Location loc = e.Location();
                loc = loc * TopLoc_Location(tr);
                e.Location(loc);
                BRepBndLib::Add(e, globalBox);
            }
        }
        gp_Pnt pMin = globalBox.CornerMin();
        gp_Pnt pMax = globalBox.CornerMax();
    }

    gp_Pnt pMin = globalBox.CornerMin();
    gp_Pnt pMax = globalBox.CornerMax();

    minX = static_cast<float>(pMin.X());
    maxX = static_cast<float>(pMax.X());
    minY = static_cast<float>(pMin.Y());
    maxY = static_cast<float>(pMax.Y());

    width = totalW;
    height = totalH;
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

    for (auto& ps : partSketches) {
        out << "<g style=\"fill:none;stroke:black;stroke-width:0.5px;vector-effect:non-scaling-stroke\" >\n";
        for (auto& ee : ps.edges) {
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
                    gp_Circ circle = adaptor.Circle();

                    float r = static_cast<float>(circle.Radius());

                    int largeArc = ((l - f) > M_PI) ? 1 : 0;
                    int sweep = 0;

                    out << "A" << r << " " << r << " 0 "
                        << largeArc << " " << sweep << " "
                        << x2 << " " << y2;
                    break;
                }
                case GeomAbs_Ellipse: {
                    gp_Elips ellipse = adaptor.Ellipse();

                    gp_Dir dir = ellipse.XAxis().Direction();
                    Standard_Real rx = static_cast<float>(ellipse.MajorRadius());
                    Standard_Real ry = static_cast<float>(ellipse.MinorRadius());

                    if (dir.IsEqual(gp_Dir(0, 1, 0), 0.1E-5)) std::swap(rx, ry);

                    int largeArc = ((l - f) > M_PI) ? 1 : 0;
                    int sweep = 0;

                    out << "A" << rx << " " << ry << " 0 "
                        << largeArc << " " << sweep << " "
                        << x2 << " " << y2;
                    break;
                }
                default:
                    break;
                }
            }
            out << "Z\"/>\n";
        }
        out << "</g>\n";
    }

    out << "</svg>";
    file.close();
    return true;
}
