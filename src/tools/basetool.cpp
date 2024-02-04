#include "basetool.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

BaseTool::BaseTool(const QString & name, QWidget * parent) : QDialog(parent)
{
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::CustomizeWindowHint);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    setFixedWidth(150);

    this->laot = new QVBoxLayout();
    this->laot->setContentsMargins(8, 8, 8, 8);
    setLayout(this->laot);

    this->nm = new QLineEdit(this);
    this->nm->setText(name);
    this->laot->addWidget(this->nm);

    this->form_layout = new QFormLayout();
    this->laot->addLayout(this->form_layout);
    this->form_layout->setSpacing(2);

    this->ok = new QPushButton("OK");
    this->laot->addWidget(this->ok);
}

QString
BaseTool::name() const
{
    return this->nm->text();
}

QFormLayout *
BaseTool::layout()
{
    return this->form_layout;
}

void
BaseTool::showEvent(QShowEvent * event)
{
    updateWidgets();
}

void
BaseTool::enableOkButton(bool state)
{
    this->ok->setEnabled(state);
}
