#include "integerwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>

IntegerWidget::IntegerWidget(const QString & text, QWidget * parent) : QLineEdit(parent)
{
    this->validator = new QIntValidator();

    setFixedWidth(80);
    setValidator(this->validator);
    connect(this, &QLineEdit::textChanged, this, &IntegerWidget::onTextChanged);
}

IntegerWidget::~IntegerWidget()
{
    delete this->validator;
}

void
IntegerWidget::setLabelText(const QString & txt)
{
}

int
IntegerWidget::value() const
{
    return text().toInt();
}

void
IntegerWidget::setValue(int value)
{
    auto s = QString("%1").arg(value);
    setText(s);
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
