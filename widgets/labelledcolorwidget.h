#pragma once

#include <QWidget>
#include <QColor>
#include "settingsstorable.h"

class QHBoxLayout;
class QLabel;
class ColorButton;

class LabelledColorWidget : public QWidget, protected SettingsStorable {
    Q_OBJECT
public:
    LabelledColorWidget(const QString & name, QWidget * parent = nullptr);
    ~LabelledColorWidget() override;

    QColor value() const;
    void setValue(const QColor & value);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

signals:
    void changed();

protected slots:
    void onColorPicked(const QColor & qcolor);

private:
    QHBoxLayout * layout;
    QLabel * label;
    ColorButton * color_button;
};
