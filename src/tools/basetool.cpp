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

    setFixedWidth(200);

    this->laot = new QVBoxLayout();
    this->laot->setContentsMargins(8, 8, 8, 8);
    this->laot->setSpacing(0);
    setLayout(this->laot);

    auto btn_layout = new QHBoxLayout();
    btn_layout->setContentsMargins(0, 0, 0, 0);
    btn_layout->setSpacing(0);

    this->nm = new QLineEdit(this);
    this->nm->setReadOnly(true);
    this->nm->setText(name);
    this->nm->setFocusPolicy(Qt::ClickFocus);
    btn_layout->addWidget(this->nm);

    this->ok = new QPushButton("\u2713");
    this->ok->setFixedWidth(25);
    btn_layout->addWidget(this->ok);

    this->cancel = new QPushButton("\u2A09");
    this->cancel->setFixedWidth(25);
    btn_layout->addWidget(this->cancel);

    this->laot->addLayout(btn_layout);

    this->form_layout = new QFormLayout();
    this->form_layout->setSizeConstraint(QLayout::SetMaximumSize);
    this->form_layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->form_layout->setContentsMargins(0, 0, 0, 0);
    this->form_layout->setSpacing(2);
    this->form_layout->setLabelAlignment(Qt::AlignLeft);
    this->laot->addLayout(this->form_layout);

    connect(this->ok, &QPushButton::clicked, this, &BaseTool::onOK);
    connect(this->cancel, &QPushButton::clicked, this, &BaseTool::onCancel);
}

QString
BaseTool::name() const
{
    return this->nm->text();
}

void
BaseTool::setName(const QString & new_name)
{
    this->nm->setText(new_name);
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

void
BaseTool::onOK()
{
    accept();
}

void
BaseTool::onCancel()
{
    reject();
}
