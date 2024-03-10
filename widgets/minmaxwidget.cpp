#include "minmaxwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

MinMaxWidget::MinMaxWidget(QWidget * parent) : QWidget(parent)
{
    this->validator = new QDoubleValidator();

    this->min_lbl = new QLabel("Min");

    this->min = new QLineEdit();
    this->min->setValidator(this->validator);

    this->max_lbl = new QLabel("Max");

    this->max = new QLineEdit();
    this->max->setValidator(this->validator);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->min_lbl);
    this->layout->addWidget(this->min);
    this->layout->addWidget(this->max_lbl);
    this->layout->addWidget(this->max);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

MinMaxWidget::~MinMaxWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->min;
    delete this->min_lbl;
    delete this->max;
    delete this->max_lbl;
}

double
MinMaxWidget::minimum() const
{
    auto txt = this->min->text();
    return txt.toDouble();
}

void
MinMaxWidget::setMinimum(double value)
{
    auto str = QString("%1").arg(value);
    this->min->setText(str);
}

double
MinMaxWidget::maximum() const
{
    auto txt = this->max->text();
    return txt.toDouble();
}

void
MinMaxWidget::setMaximum(double value)
{
    auto str = QString("%1").arg(value);
    this->max->setText(str);
}
