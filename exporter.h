#pragma once
#include <QString>
#include <TopoDS_Wire.hxx>
#include <HLRBRep_CurveTool.hxx>

class Exporter {
public:
    Exporter() = default;
    ~Exporter() = default;

    void add(const std::vector<TopoDS_Wire>& wires);

    /// Receives the edges to export to an SVG file. They must be already ordered by wires and continuity
    void add(const std::vector<std::vector<TopoDS_Edge>>& edges);
    bool exportToFile(const QString& filename) const;

private:
    std::vector<TopoDS_Wire> wires;
    std::vector<std::vector<TopoDS_Edge>> edges;
    float width;
    float height;
    float minX, maxX, minY, maxY;
};
