#pragma once

#include "OcctQWidgetViewer.h"
#include "Assembly.h"

#include <QMainWindow>
#include <QApplication>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
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
    QGridLayout* gridLayout = nullptr;
    QPushButton* btnExport = nullptr;

    Parametrikut(QWidget *parent = nullptr);
    ~Parametrikut() {};

    void buildGrid(std::vector<Param> params);
};

