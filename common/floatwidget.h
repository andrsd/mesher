#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QDoubleValidator;

class FloatWidget : public QWidget {
    Q_OBJECT
public:
    explicit FloatWidget(const QString & text, QWidget * parent = nullptr);
    ~FloatWidget() override;

    void setLabelText(const QString & txt);

    double value() const;
    void setValue(double value);

private:
    QDoubleValidator * validator;
    QHBoxLayout * layout;
    QLabel * lbl;
    QLineEdit * num;
};
