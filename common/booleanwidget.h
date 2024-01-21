#pragma once

#include <QWidget>
#include "settingsstorable.h"

class QCheckBox;
class QHBoxLayout;
class QLabel;
class QSettings;

class BooleanWidget : public QWidget, protected SettingsStorable {
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

private:
    QHBoxLayout * layout;
    QLabel * label;
    QCheckBox * check;
};
