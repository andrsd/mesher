#pragma once

#include <QLineEdit>
#include "settingsstorable.h"

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QDoubleValidator;

class FloatWidget : public QLineEdit, protected SettingsStorable {
    Q_OBJECT
public:
    explicit FloatWidget(const QString & text, QWidget * parent = nullptr);
    ~FloatWidget() override;

    void setLabelText(const QString & txt);

    double value() const;
    void setValue(double value);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

protected slots:
    void onTextChanged(const QString & text);

private:
    QDoubleValidator * validator;
    QHBoxLayout * layout;
    QLabel * lbl;
    QLineEdit * num;
};
