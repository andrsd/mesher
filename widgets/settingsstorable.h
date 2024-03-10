#pragma once

#include <QString>

class QSettings;

class SettingsStorable {
public:
    SettingsStorable();

    void bindSettings(QSettings * settings, const QString & key);
    QVariant readSetting(const QVariant & default_value);
    void storeSetting(const QVariant & value);

    virtual void
    bindToSettings(QSettings * settings, const QString & key, const QVariant & default_value) = 0;

protected:
    QSettings * settings;
    QString key;
};
