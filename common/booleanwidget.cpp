#include "booleanwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>

BooleanWidget::BooleanWidget(const QString & txt, QWidget * parent) : QCheckBox(txt, parent)
{
    connect(this, &QCheckBox::stateChanged, this, &BooleanWidget::onStateChanged);
}

BooleanWidget::~BooleanWidget() {}

bool
BooleanWidget::value()
{
    return this->checkState() == Qt::CheckState::Checked;
}

void
BooleanWidget::setValue(bool state)
{
    this->setCheckState(state ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
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
    emit changed();
}
