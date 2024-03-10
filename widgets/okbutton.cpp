#include "okbutton.h"

OkButton::OkButton(QWidget * parent) : QPushButton("\u2713", parent)
{
    setFixedSize(22, 22);
    setStyleSheet("QPushButton {"
                  "  background-color: rgb(65, 147, 41);"
                  "  color: rgb(255, 255, 255);"
                  "  font-weight: bold;"
                  "  font-size: 15pt;"
                  "  border: none;"
                  "}"
                  "QPushButton:hover {"
                  "  background-color: rgb(97, 185, 75);"
                  "}"
                  "QPushButton:disabled {"
                  "  background-color: rgb(217, 233, 212);"
                  "}"
                  "QPushButton:disabled:hover {"
                  "  background-color: rgb(223, 241, 219);"
                  "}");
}
