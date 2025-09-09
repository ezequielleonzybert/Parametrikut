#include "Parametrikut.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QLabel>

Parametrikut::Parametrikut(QWidget *parent)
    : QMainWindow(parent)
{
    assembly = new Assembly();
    assembly->build();



    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    int margin = 20;
    int spacing = 20;

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

    centralLayout->addWidget(leftWidget, 0);

    viewer = new OcctQWidgetViewer(centralWidget);
    viewer->displayAssembly(*assembly); //first display

    centralLayout->addWidget(viewer, 1);

    qApp->setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
        } 
    )");
}

void Parametrikut::buildGrid(std::vector<Param> params) {

    QRegularExpression rxFloat(R"(^([0-9]+([.,][0-9]*)?|[.,][0-9]+)?$)");
    QRegularExpressionValidator* validFloat = new QRegularExpressionValidator(rxFloat, this);
    QRegularExpression rxInt("[0-9]+");
    QRegularExpressionValidator* validInt = new QRegularExpressionValidator(rxInt, this);

    for(int i = 0; i< params.size(); i++) {

        Param param = params[i];
		QLabel* label = new QLabel(param.name,leftWidget);
        QLineEdit* lineEdit = new QLineEdit(leftWidget);

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
            viewer->displayAssembly(*assembly);
        });
	}
}