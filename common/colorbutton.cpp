#include "colorbutton.h"
#include "colorpicker.h"

ColorButton::ColorButton(const QString & name, QWidget * parent) :
    QPushButton(parent),
    clr(QColor(0, 0, 0))
{
    this->color_picker = new ColorPicker(parent);
    if (name.length() > 0)
        this->color_picker->setWindowTitle(name);

    this->setFixedWidth(50);
    this->setFixedHeight(20);
    this->setStyleSheet("border: 1px solid #000");

    connect(this, &QPushButton::clicked, this, &ColorButton::onClick);
    connect(this->color_picker, &ColorPicker::colorChanged, this, &ColorButton::onColorPicked);
    connect(this->color_picker, &ColorPicker::opacityChanged, this, &ColorButton::onOpacityChanged);
}

ColorButton::~ColorButton()
{
    delete this->color_picker;
}

void
ColorButton::onClick()
{
    this->color_picker->setColor(this->clr);
    this->color_picker->show();
}

void
ColorButton::onColorPicked(const QColor & qcolor)
{
    this->clr = qcolor;
}

void
ColorButton::onOpacityChanged(double opacity)
{
    this->clr.setAlphaF(opacity);
}
