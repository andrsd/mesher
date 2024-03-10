#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QIntValidator;

class SizeWidget : public QWidget {
    Q_OBJECT
public:
    explicit SizeWidget(QWidget * parent = nullptr);
    ~SizeWidget() override;

    int userWidth() const;
    void setUserWidth(int value);
    int userHeight() const;
    void setUserHeight(int value);

private:
    QIntValidator * validator;
    QHBoxLayout * layout;
    QLabel * wd_lbl;
    QLineEdit * wd;
    QLabel * ht_lbl;
    QLineEdit * ht;
};
