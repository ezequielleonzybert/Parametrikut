#pragma once
#include <QString>
#include <TopoDS_Wire.hxx>
#include <HLRBRep_CurveTool.hxx>
#include "Assembly.h"

class Exporter {
    struct PartSketch{
        std::vector<std::vector<TopoDS_Edge>> edges;
        Bnd_Box bb;
        Standard_Real width, height;
        gp_Pnt translation;
    };

private:
    std::vector<PartSketch> partSketches;
    std::vector<std::vector<std::vector<TopoDS_Edge>>> edges;
    float width;
    float height;
    float minX, maxX, minY, maxY;

    void add(Assembly* assembly);
    bool exportToFile(const QString& filename) const;

public:
    Exporter(Assembly* assembly, QString filepath);
};
