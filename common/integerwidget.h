#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QIntValidator;

class IntegerWidget : public QWidget {
    Q_OBJECT
public:
    explicit IntegerWidget(const QString & text, QWidget * parent = nullptr);
    ~IntegerWidget() override;

    void setLabelText(const QString & txt);

    int value() const;
    void setValue(int value);

private:
    QIntValidator * validator;
    QHBoxLayout * layout;
    QLabel * lbl;
    QLineEdit * num;
};
