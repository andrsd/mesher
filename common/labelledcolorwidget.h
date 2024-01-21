#pragma once

#include <QWidget>

class QHBoxLayout;
class QLabel;
class ColorButton;

class LabelledColorWidget : public QWidget {
    Q_OBJECT
public:
    LabelledColorWidget(const QString & name, QWidget * parent = nullptr);
    ~LabelledColorWidget() override;

private:
    QHBoxLayout * layout;
    QLabel * label;
    ColorButton * color_button;
};
