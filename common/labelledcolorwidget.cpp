#include "labelledcolorwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include "colorbutton.h"

LabelledColorWidget::LabelledColorWidget(const QString & name, QWidget * parent) : QWidget(parent)
{
    this->layout = new QHBoxLayout();
    this->layout->setContentsMargins(30, 0, 30, 0);
    setLayout(this->layout);

    this->label = new QLabel(name);
    this->layout->addWidget(this->label);

    this->color_button = new ColorButton(name);
    this->layout->addWidget(this->color_button);

    connect(this->color_button,
            &ColorButton::colorPicked,
            this,
            &LabelledColorWidget::onColorPicked);
}

LabelledColorWidget::~LabelledColorWidget()
{
    delete this->label;
    delete this->color_button;
    delete this->layout;
}

QColor
LabelledColorWidget::value() const
{
    return this->color_button->color();
}

void
LabelledColorWidget::setValue(const QColor & value)
{
    this->color_button->setColor(value);
}

void
LabelledColorWidget::bindToSettings(QSettings * settings,
                                    const QString & key,
                                    const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).value<QColor>());
}

void
LabelledColorWidget::onColorPicked(const QColor & qcolor)
{
    qDebug() << "color" << qcolor;
    storeSetting(qcolor);
}
