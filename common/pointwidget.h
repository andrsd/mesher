#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QDoubleValidator;

class PointWidget : public QWidget {
    Q_OBJECT
public:
    explicit PointWidget(QWidget * parent = nullptr);
    ~PointWidget() override;

    double userX() const;
    double userY() const;
    double userZ() const;

private:
    QDoubleValidator * validator;
    QHBoxLayout * layout;
    QLabel * x_lbl;
    QLineEdit * x_edit;
    QLabel * y_lbl;
    QLineEdit * y_edit;
    QLabel * z_lbl;
    QLineEdit * z_edit;
};
