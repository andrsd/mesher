#pragma once

#include <QCheckBox>
#include "settingsstorable.h"

class QHBoxLayout;
class QLabel;
class QSettings;

class BooleanWidget : public QCheckBox, protected SettingsStorable {
    Q_OBJECT
public:
    explicit BooleanWidget(const QString & label, QWidget * parent = nullptr);
    ~BooleanWidget() override;

    bool value();
    void setValue(bool state);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

protected:
    void onStateChanged(int state);
};
