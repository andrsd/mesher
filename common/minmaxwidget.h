#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QDoubleValidator;

class MinMaxWidget : public QWidget {
    Q_OBJECT
public:
    explicit MinMaxWidget(QWidget * parent = nullptr);
    ~MinMaxWidget() override;

    double minimum() const;
    void setMinimum(double value);
    double maximum() const;
    void setMaximum(double value);

private:
    QDoubleValidator * validator;
    QHBoxLayout * layout;
    QLabel * min_lbl;
    QLineEdit * min;
    QLabel * max_lbl;
    QLineEdit * max;
};
