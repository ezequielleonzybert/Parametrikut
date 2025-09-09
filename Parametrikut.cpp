#include "Parametrikut.h"

Parametrikut::Parametrikut(QWidget *parent)
    : QMainWindow(parent)
{
    assembly = new Assembly();
    assembly->build();

    leThickness = new QLineEdit("3", leftWidget);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* centralLayout = new QHBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralWidget->setLayout(centralLayout);

    leftWidget = new QWidget(centralWidget);
    QGridLayout* gridLayout = new QGridLayout(leftWidget);
    gridLayout->setSpacing(25);
    gridLayout->setContentsMargins(25, 25, 25, 25);
    gridLayout->addWidget(new QLabel("Thickness", leftWidget), 0, 0);
    gridLayout->addWidget(leThickness, 0, 1);

    connect(leThickness, &QLineEdit::editingFinished, [this]() {
        float t = leThickness->text().toFloat();
        assembly->thickness = t;
        assembly->build();
        viewer->displayAssembly(*assembly);
        });

    centralLayout->addWidget(leftWidget, 1);

    rightWidget = new QWidget(centralWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightWidget->setLayout(rightLayout);

    viewer = new OcctQWidgetViewer(rightWidget);
    viewer->displayAssembly(*assembly); //first display
    rightLayout->addWidget(viewer, 1);

    btnExport = new QPushButton("Export plan", rightWidget);
    rightLayout->addWidget(btnExport, 0);

    centralLayout->addWidget(rightWidget, 1);

    resize(600, 400);
}

Parametrikut::~Parametrikut()
{}