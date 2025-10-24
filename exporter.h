#pragma once
#include <QString>
#include <TopoDS_Compound.hxx>

class Exporter {
public:
    Exporter() = default;
    ~Exporter() = default;

    void setShape(const TopoDS_Shape shape);
    bool exportToFile(const QString& filename) const;

private:
    TopoDS_Shape shape;
};
