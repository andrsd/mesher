#include "labelledcolorwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include "colorbutton.h"

LabelledColorWidget::LabelledColorWidget(const QString & name, QWidget * parent) : QWidget(parent)
{
    this->layout = new QHBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    setLayout(this->layout);

    this->label = new QLabel(name);
    this->layout->addWidget(this->label);

    this->color_button = new ColorButton(name);
    this->layout->addWidget(this->color_button);
}

LabelledColorWidget::~LabelledColorWidget()
{
    delete this->label;
    delete this->color_button;
    delete this->layout;
}
