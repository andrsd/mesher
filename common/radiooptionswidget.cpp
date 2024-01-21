#include "radiooptionswidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>

RadioOptionsWidget::RadioOptionsWidget(const QStringList & str_options, QWidget * parent) :
    QWidget(parent)
{
    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    setLayout(this->layout);

    for (auto & o : str_options) {
        auto * rb = new QRadioButton(o, this);
        this->options.append(rb);
        this->layout->addWidget(rb);
    }
}

RadioOptionsWidget::~RadioOptionsWidget()
{
    for (auto & it : this->options)
        delete it;
    delete this->layout;
}

int
RadioOptionsWidget::value() const
{
    for (int i = 0; i < this->options.size(); i++)
        if (this->options[i]->isChecked())
            return i;
    return -1;
}

void
RadioOptionsWidget::setValue(int value)
{
    if ((value >= 0) && (value < this->options.size()))
        this->options[value]->setChecked(Qt::CheckState::Checked);
}
