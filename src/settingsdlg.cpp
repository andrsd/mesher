#include "settingsdlg.h"
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget * parent) : QDialog(parent)
{
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    setWindowTitle("Settings");
    setMinimumSize(600, 700);

    this->layout = new QVBoxLayout();
    this->layout->addSpacing(8);

    setLayout(this->layout);
}

SettingsDialog::~SettingsDialog()
{
    delete this->layout;
}
