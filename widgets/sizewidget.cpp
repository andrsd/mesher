#include "sizewidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>

SizeWidget::SizeWidget(QWidget * parent) : QWidget(parent)
{
    this->validator = new QIntValidator();
    this->validator->setBottom(0);

    this->wd_lbl = new QLabel("Width");

    this->wd = new QLineEdit();
    this->wd->setValidator(this->validator);

    this->ht_lbl = new QLabel("Height");

    this->ht = new QLineEdit();
    this->ht->setValidator(this->validator);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->wd_lbl);
    this->layout->addWidget(this->wd);
    this->layout->addWidget(this->ht_lbl);
    this->layout->addWidget(this->ht);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

SizeWidget::~SizeWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->wd_lbl;
    delete this->wd;
    delete this->ht_lbl;
    delete this->ht;
}

int
SizeWidget::userWidth() const
{
    auto txt = this->wd->text();
    return txt.toInt();
}

void
SizeWidget::setUserWidth(int value)
{
    auto str = QString("%1").arg(value);
    this->wd->setText(str);
}

int
SizeWidget::userHeight() const
{
    auto txt = this->ht->text();
    return txt.toInt();
}

void
SizeWidget::setUserHeight(int value)
{
    auto str = QString("%1").arg(value);
    this->ht->setText(str);
}
