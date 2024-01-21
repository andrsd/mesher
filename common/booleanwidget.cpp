#include "booleanwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>

BooleanWidget::BooleanWidget(const QString & txt, QWidget * parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    this->layout = new QHBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setSpacing(0);
    setLayout(this->layout);

    this->label = new QLabel(txt);
    this->layout->addWidget(this->label);

    this->check = new QCheckBox("");
    this->layout->addWidget(this->check);
}

BooleanWidget::~BooleanWidget()
{
    delete this->layout;
    delete this->label;
    delete this->check;
}

bool
BooleanWidget::value()
{
    return this->check->checkState() == Qt::CheckState::Checked;
}

void
BooleanWidget::setValue(bool state)
{
    this->check->setCheckState(state ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}
