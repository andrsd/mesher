#include "pointwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

PointWidget::PointWidget(QWidget * parent) : QWidget(parent)
{
    const int wd = 50;

    this->validator = new QDoubleValidator();

    this->x_lbl = new QLabel("X");
    this->x_edit = new QLineEdit();
    this->x_edit->setValidator(this->validator);
    this->x_edit->setFixedWidth(wd);

    this->y_lbl = new QLabel("Y");
    this->y_edit = new QLineEdit();
    this->y_edit->setValidator(this->validator);
    this->y_edit->setFixedWidth(wd);

    this->z_lbl = new QLabel("Z");
    this->z_edit = new QLineEdit();
    this->z_edit->setValidator(this->validator);
    this->z_edit->setFixedWidth(wd);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->x_lbl);
    this->layout->addWidget(this->x_edit);
    this->layout->addWidget(this->y_lbl);
    this->layout->addWidget(this->y_edit);
    this->layout->addWidget(this->z_lbl);
    this->layout->addWidget(this->z_edit);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

PointWidget::~PointWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->x_lbl;
    delete this->x_edit;
    delete this->y_lbl;
    delete this->y_edit;
    delete this->z_lbl;
    delete this->z_edit;
}

double
PointWidget::userX() const
{
    auto txt = this->x_edit->text();
    return txt.toDouble();
}

double
PointWidget::userY() const
{
    auto txt = this->y_edit->text();
    return txt.toDouble();
}

double
PointWidget::userZ() const
{
    auto txt = this->z_edit->text();
    return txt.toDouble();
}
