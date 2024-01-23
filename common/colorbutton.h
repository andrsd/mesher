#pragma once

#include <QPushButton>

class ColorPicker;

class ColorButton : public QPushButton {
    Q_OBJECT
public:
    ColorButton(const QString & name = "", QWidget * parent = nullptr);
    ~ColorButton() override;

    QColor color() const;
    void setColor(const QColor & color);

signals:
    void colorPicked(const QColor & color);

protected slots:
    void onClick();
    void onColorPicked(const QColor & qcolor);
    void onOpacityChanged(double opacity);

private:
    QColor clr;
    ColorPicker * color_picker;
};
