#include "cancelbutton.h"

CancelButton::CancelButton(QWidget * parent) : QPushButton("\u2A09", parent)
{
    setFixedSize(22, 22);
    setStyleSheet("QPushButton {"
                  "  background-color: rgb(255, 255, 255);"
                  "  color: rgb(174, 60, 62);"
                  "  font-weight: bold;"
                  "  font-size: 15pt;"
                  "  border: none;"
                  "}"
                  "QPushButton:hover {"
                  "  background-color: rgb(244, 245, 246);"
                  "}");
}
