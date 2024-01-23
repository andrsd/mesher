#include "floatwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

FloatWidget::FloatWidget(const QString & text, QWidget * parent) : QLineEdit(parent)
{
    this->validator = new QDoubleValidator();

    setValidator(this->validator);
    setFixedWidth(80);
    connect(this, &QLineEdit::textChanged, this, &FloatWidget::onTextChanged);
}

FloatWidget::~FloatWidget()
{
    delete this->validator;
}

void
FloatWidget::setLabelText(const QString & txt)
{
}

double
FloatWidget::value() const
{
    return text().toDouble();
}

void
FloatWidget::setValue(double value)
{
    auto txt = QString("%1").arg(value);
    setText(txt);
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
