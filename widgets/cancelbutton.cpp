#include "cancelbutton.h"

CancelButton::CancelButton(QWidget * parent) : QPushButton("\u2A09", parent)
{
    setFixedSize(28, 28);
    setStyleSheet("QPushButton {"
                  "  background-color: rgb(255, 255, 255);"
                  "  color: rgb(174, 60, 62);"
                  "  font-weight: bold;"
                  "  font-size: 15pt;"
                  "  border: none;"
                  "  border-bottom: 1px solid rgb(221, 221, 221);"
                  "}"
                  "QPushButton:hover {"
                  "  background-color: rgb(244, 245, 246);"
                  "}");
}
