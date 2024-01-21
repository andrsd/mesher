#include "booleanwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>

BooleanWidget::BooleanWidget(const QString & txt, QWidget * parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    this->layout = new QHBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setSpacing(0);
    setLayout(this->layout);

    this->label = new QLabel(txt);
    this->layout->addWidget(this->label);

    this->check = new QCheckBox("");
    this->layout->addWidget(this->check);

    connect(this->check, &QCheckBox::stateChanged, this, &BooleanWidget::onStateChanged);
}

BooleanWidget::~BooleanWidget()
{
    delete this->layout;
    delete this->label;
    delete this->check;
}

bool
BooleanWidget::value()
{
    return this->check->checkState() == Qt::CheckState::Checked;
}

void
BooleanWidget::setValue(bool state)
{
    this->check->setCheckState(state ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

void
BooleanWidget::bindToSettings(QSettings * settings,
                              const QString & key,
                              const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).toBool());
}

void
BooleanWidget::onStateChanged(int state)
{
    bool checked = state == Qt::CheckState::Checked;
    storeSetting(checked);
}
