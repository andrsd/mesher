#include "toolwindow.h"
#include "okbutton.h"
#include "cancelbutton.h"
#include "tooltitlewidget.h"
#include "namewidget.h"
#include <QVBoxLayout>

ToolWindow::ToolWindow(QWidget * parent) : QDialog(parent)
{
    setUpStyle();

    this->title = new ToolTitleWidget(this);
    //    this->title->setText("Select other");
    this->title_layout->addWidget(this->title);

    this->cancel = new CancelButton();
    this->title_layout->addWidget(this->cancel);

    connect(this->cancel, &QPushButton::clicked, this, &ToolWindow::onCancel);
}

ToolWindow::ToolWindow(const QString & name, QWidget * parent) : QDialog(parent)
{
    setUpStyle();
    setWindowTitle(name);

    this->editable_title = new NameWidget(this);
    this->editable_title->setReadOnly(true);
    this->editable_title->setText(name);
    this->title_layout->addWidget(this->editable_title);

    this->ok = new OkButton();
    this->title_layout->addWidget(this->ok);

    this->cancel = new CancelButton();
    this->title_layout->addWidget(this->cancel);

    connect(this->editable_title, &QLineEdit::editingFinished, this, &ToolWindow::onTitleChanged);
    connect(this->ok, &QPushButton::clicked, this, &ToolWindow::onOK);
    connect(this->cancel, &QPushButton::clicked, this, &ToolWindow::onCancel);
}

void
ToolWindow::setUpStyle()
{
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Tool);
    setStyleSheet("QDialog {"
                  "  background-color: rgb(255, 255, 255);"
                  "}"
                  "QLabel {"
                  "  color: rgb(128, 128, 128);"
                  "  font-size: 12pt;"
                  "}");

    this->base_layout = new QVBoxLayout();
    this->base_layout->setContentsMargins(0, 0, 0, 0);
    this->base_layout->setSpacing(0);
    QDialog::setLayout(this->base_layout);

    this->title_layout = new QHBoxLayout();
    this->title_layout->setContentsMargins(0, 0, 0, 0);
    this->title_layout->setSpacing(0);
    this->base_layout->addLayout(this->title_layout);
}

void
ToolWindow::setWindowTitle(const QString title)
{
    QDialog::setWindowTitle(title);
    if (this->title)
        this->title->setText(title);
    if (this->editable_title)
        this->editable_title->setText(title);
}

void
ToolWindow::setLayout(QLayout * layout)
{
    this->base_layout->addLayout(layout);
}

void
ToolWindow::enableOkButton(bool state)
{
    this->ok->setEnabled(state);
}

void
ToolWindow::onOK()
{
    accept();
}

void
ToolWindow::onCancel()
{
    reject();
}

void
ToolWindow::onTitleChanged()
{
    QDialog::setWindowTitle(this->editable_title->text());
    emit titleChanged(this);
}
