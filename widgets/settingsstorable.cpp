#include "settingsstorable.h"
#include <QSettings>

SettingsStorable::SettingsStorable() : settings(nullptr) {}

void
SettingsStorable::bindSettings(QSettings * settings, const QString & key)
{
    this->settings = settings;
    this->key = key;
}

QVariant
SettingsStorable::readSetting(const QVariant & default_value)
{
    if (this->settings)
        return this->settings->value(this->key, default_value);
    else
        return QVariant();
}

void
SettingsStorable::storeSetting(const QVariant & value)
{
    if (this->settings)
        this->settings->setValue(this->key, value);
}
