#pragma once

#include <QWidget>
#include <QVector3D>
#include "settingsstorable.h"

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QDoubleValidator;

class PointWidget : public QWidget, protected SettingsStorable {
    Q_OBJECT
public:
    explicit PointWidget(QWidget * parent = nullptr);
    ~PointWidget() override;

    QVector3D value() const;
    void setValue(const QVector3D & value);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

signals:
    void changed();

protected slots:
    void onXTextChanged(const QString & text);
    void onYTextChanged(const QString & text);
    void onZTextChanged(const QString & text);

private:
    QDoubleValidator * validator;
    QHBoxLayout * layout;
    QLabel * x_lbl;
    QLineEdit * x_edit;
    QLabel * y_lbl;
    QLineEdit * y_edit;
    QLabel * z_lbl;
    QLineEdit * z_edit;
};
