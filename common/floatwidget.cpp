#include "floatwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

FloatWidget::FloatWidget(const QString & text, QWidget * parent) : QWidget(parent)
{
    this->validator = new QDoubleValidator();

    this->lbl = new QLabel(text);

    this->num = new QLineEdit();
    this->num->setValidator(this->validator);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->lbl);
    this->layout->addWidget(this->num);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);
}

FloatWidget::~FloatWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->lbl;
    delete this->num;
}

void
FloatWidget::setLabelText(const QString & txt)
{
    this->lbl->setWindowTitle(txt);
}

double
FloatWidget::value() const
{
    auto txt = this->num->text();
    return txt.toDouble();
}

void
FloatWidget::setValue(double value)
{
    auto txt = QString("%1").arg(value);
    this->num->setText(txt);
}

void
FloatWidget::bindToSettings(QSettings * settings,
                            const QString & key,
                            const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).toDouble());
}

void
FloatWidget::onTextChanged(const QString & text)
{
    storeSetting(text);
}
