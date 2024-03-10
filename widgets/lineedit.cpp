#include "lineedit.h"

LineEdit::LineEdit(QWidget * parent) : QLineEdit(parent)
{
    setUpStyle();
}

LineEdit::LineEdit(const QString & contents, QWidget * parent) : QLineEdit(contents, parent)
{
    setUpStyle();
}

void
LineEdit::setUpStyle()
{
    this->setStyleSheet("QLineEdit {"
                        "  border: none;"
                        "  border-bottom: 1px solid rgb(221, 221, 221);"
                        "}"
                        "QLineEdit:hover {"
                        "  border: none;"
                        "  border-bottom: 1px solid rgb(40, 80, 170);"
                        "}"
                        "QLineEdit:focus {"
                        "  border: none;"
                        "  border-bottom: 2px solid rgb(40, 80, 170);"
                        "}");
    this->setAttribute(Qt::WA_MacShowFocusRect, 0);
}
