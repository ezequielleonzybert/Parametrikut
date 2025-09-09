#pragma once

#include "OcctQWidgetViewer.h"
#include "Assembly.h"

#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

class Parametrikut : public QMainWindow
{
    Q_OBJECT

public:

    Assembly* assembly = nullptr;

    OcctQWidgetViewer* viewer = nullptr;

    QWidget* centralWidget = nullptr;
    QWidget* leftWidget = nullptr;
    QWidget* rightWidget = nullptr;
    QPushButton* btnExport = nullptr;
    QLineEdit* leThickness = nullptr;
    QLineEdit* leLevels = nullptr;

    Parametrikut(QWidget *parent = nullptr);
    ~Parametrikut();
};

