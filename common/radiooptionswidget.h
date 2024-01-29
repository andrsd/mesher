#pragma once

#include <QWidget>
#include <QList>
#include "settingsstorable.h"

class QVBoxLayout;
class QLabel;
class QRadioButton;
class QSignalMapper;

class RadioOptionsWidget : public QWidget, protected SettingsStorable {
    Q_OBJECT
public:
    explicit RadioOptionsWidget(const QStringList & str_options, QWidget * parent = nullptr);
    ~RadioOptionsWidget() override;

    int value() const;
    void setValue(int value);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

    void enableOption(int idx, bool state);

signals:
    void changed();
    void valueChanged(int id);

protected slots:
    void onValueChanged(int id);

private:
    QVBoxLayout * layout;
    QList<QRadioButton *> options;
    QSignalMapper * signal_mapper;
};
