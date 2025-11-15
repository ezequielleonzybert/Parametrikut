#include "Parametrikut.h"
#include "exporter.h"  
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QLabel>
#include <QFileDialog>

Parametrikut::Parametrikut(QWidget *parent)
    : QMainWindow(parent)
{
    resize(600, 400);

    assembly = new Assembly();
    assembly->build();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    int margin = 10;
    int spacing = 10;

    QHBoxLayout* centralLayout = new QHBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralWidget->setLayout(centralLayout);

    leftWidget = new QWidget(centralWidget);

    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->setContentsMargins(margin, margin, margin, margin);
	leftLayout->setSpacing(spacing);

    gridLayout = new QGridLayout(leftWidget);
	gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setSpacing(spacing);
    gridLayout->setContentsMargins(0, 0, 0, 0);

	buildGrid(assembly->params);

	leftLayout->addLayout(gridLayout,1);

    btnExport = new QPushButton("Export plan", leftWidget);
    leftLayout->addWidget(btnExport, 0);

    centralLayout->addWidget(leftWidget, 1);

    viewer = new OcctQWidgetViewer(assembly, centralWidget);
    viewer->displayAssembly(); //first display

    centralLayout->addWidget(viewer, 4);
    viewer->update();
    viewer->setFocus();

    qApp->setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
        } 
    )");

    connect(btnExport, &QPushButton::clicked, this, [this]() {
        QString filepath = QFileDialog::getSaveFileName(
            this,
            "Export to SVG",
            QDir::homePath(),
            "SVG Files (*.svg);;All Files (*)"
        );
        Exporter exporter(assembly, filepath);
    });
}

void Parametrikut::buildGrid(std::vector<Param> &params) {

    QRegularExpression rxFloat(R"(^([0-9]+([.,][0-9]*)?|[.,][0-9]+)?$)");
    QRegularExpressionValidator* validFloat = new QRegularExpressionValidator(rxFloat, this);
    QRegularExpression rxInt("[0-9]+");
    QRegularExpressionValidator* validInt = new QRegularExpressionValidator(rxInt, this);

    for(int i = 0; i < params.size(); i++) {

        Param param = params[i];
		QLabel* label = new QLabel(param.name,leftWidget);
        myLineEdit* lineEdit = new myLineEdit(leftWidget);

        lineEdit->setAlignment(Qt::AlignRight);

        bool isFloat;
		param.vali == -1 ? isFloat = true : isFloat = false;

        if (isFloat) {
            lineEdit->setText(QString::number(param.valf));
            lineEdit->setValidator(validFloat);
        }
        else {
            lineEdit->setText(QString::number(param.vali));
            lineEdit->setValidator(validInt);
        }

        gridLayout->addWidget(label, i, (i * 2)%2);
        gridLayout->addWidget(lineEdit, i, (i * 2 + 1)%2);

        connect(lineEdit, &QLineEdit::editingFinished, [=]() {

            int vali = lineEdit->text().toInt();
            float valf = lineEdit->text().toFloat();

            if ((!isFloat && assembly->params[i].vali != vali) ||
                (isFloat && assembly->params[i].valf != valf)) {

                QString text = lineEdit->text();
                text.replace(',', '.');
                if (isFloat) {
                    float valf = text.toFloat();
                    assembly->params[i].valf = valf;
                }
                else {
                    int vali = text.toInt();
                    assembly->params[i].vali = vali;
                }
                assembly->build();
                viewer->displayAssembly();
            }
        });

        connect(lineEdit, &QLineEdit::hasFocus, [=]() {
            lineEdit->selectAll();
        });
	}
}