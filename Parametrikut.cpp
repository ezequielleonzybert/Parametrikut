#include "Parametrikut.h"

Parametrikut::Parametrikut(QWidget *parent)
    : QMainWindow(parent)
{
    assembly = new Assembly();
    assembly->build();

    qApp->setStyleSheet("QLineEdit { width: 20px; }");

    leThickness = new QLineEdit("3", leftWidget);
    leLevels = new QLineEdit("3", leftWidget);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* centralLayout = new QHBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralWidget->setLayout(centralLayout);

    leftWidget = new QWidget(centralWidget);
    QGridLayout* gridLayout = new QGridLayout(leftWidget);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    gridLayout->addWidget(new QLabel("Thickness:", leftWidget), 0, 0);
    gridLayout->addWidget(leThickness, 0, 1);
    gridLayout->addWidget(new QLabel("Levels:", leftWidget), 0, 2);
    gridLayout->addWidget(leLevels, 0, 3);

    connect(leThickness, &QLineEdit::editingFinished, [this]() {
        float t = leThickness->text().toFloat();
        assembly->thickness = t;
        assembly->build();
        viewer->displayAssembly(*assembly);
        });

    connect(leLevels, &QLineEdit::editingFinished, [this]() {
        float t = leLevels->text().toFloat();
        assembly->levels = t;
        assembly->build();
        viewer->displayAssembly(*assembly);
        });

    centralLayout->addWidget(leftWidget, 0);

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