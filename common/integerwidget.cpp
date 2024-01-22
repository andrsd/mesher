#include "integerwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>

IntegerWidget::IntegerWidget(const QString & text, QWidget * parent) : QWidget(parent)
{
    this->validator = new QIntValidator();

    this->lbl = new QLabel(text);

    this->num = new QLineEdit();
    this->num->setFixedWidth(80);
    this->num->setValidator(this->validator);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->lbl);
    this->layout->addWidget(this->num);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

IntegerWidget::~IntegerWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->lbl;
    delete this->num;
}

void
IntegerWidget::setLabelText(const QString & txt)
{
    this->lbl->setWindowTitle(txt);
}

int
IntegerWidget::value() const
{
    auto txt = this->num->text();
    return txt.toInt();
}

void
IntegerWidget::setValue(int value)
{
    auto s = QString("%1").arg(value);
    this->num->setText(s);
}

void
IntegerWidget::bindToSettings(QSettings * settings,
                              const QString & key,
                              const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).toDouble());
}

void
IntegerWidget::onTextChanged(const QString & text)
{
    storeSetting(text);
}
