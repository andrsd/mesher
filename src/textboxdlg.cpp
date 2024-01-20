#include "textboxdlg.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFile>
#include <QString>

TextBoxDialog::TextBoxDialog(QWidget * parent) : QDialog(parent)
{
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    this->layout = new QVBoxLayout();
    this->layout->addSpacing(8);

    this->text = new QTextEdit();
    this->text->setReadOnly(true);
    this->layout->addWidget(this->text);

    setLayout(this->layout);
}

TextBoxDialog::~TextBoxDialog()
{
    delete this->layout;
}

void
TextBoxDialog::loadTextFromFile(const QString & file_name)
{
    QString text;
    QFile file(file_name);
    if (file.open(QIODevice::ReadOnly))
        text = file.readAll();
    this->text->setText(text);
}
