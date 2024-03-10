#pragma once

#include <QLineEdit>

class LineEdit : public QLineEdit {
public:
    LineEdit(QWidget * parent = nullptr);
    LineEdit(const QString & contents, QWidget * parent = nullptr);

private:
    void setUpStyle();
};
