#pragma once
#include <QString>
#include <TopoDS_Wire.hxx>
#include <HLRBRep_CurveTool.hxx>

class Exporter {
public:
    Exporter() = default;
    ~Exporter() = default;

    void add(const std::vector<TopoDS_Wire>& wires);
    bool exportToFile(const QString& filename) const;

private:
    std::vector<TopoDS_Wire> wires;
    float width;
    float height;
};
