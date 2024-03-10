#include "labelledwidget.h"
#include <QLabel>
#include <QVBoxLayout>

LabelledWidget::LabelledWidget(QWidget * parent) : QWidget(parent), label(nullptr), child(nullptr)
{
    this->layout = new QVBoxLayout();

    this->label = new QLabel();
    this->label->setStyleSheet("font-weight: bold");

    this->layout->addWidget(this->label);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

LabelledWidget::~LabelledWidget()
{
    delete this->label;
    delete this->layout;
}

void
LabelledWidget::setLabelText(const QString & str)
{
    this->label->setText(str);
}

void
LabelledWidget::setWidget(QWidget * widget, int stretch, Qt::Alignment alignment)
{
    this->layout->insertWidget(1, widget, stretch, alignment);
}
