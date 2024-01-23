#pragma once

#include <QLineEdit>
#include "settingsstorable.h"

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QIntValidator;

class IntegerWidget : public QLineEdit, protected SettingsStorable {
    Q_OBJECT
public:
    explicit IntegerWidget(const QString & text, QWidget * parent = nullptr);
    ~IntegerWidget() override;

    void setLabelText(const QString & txt);

    int value() const;
    void setValue(int value);

    void bindToSettings(QSettings * settings,
                        const QString & key,
                        const QVariant & default_value) override;

protected slots:
    void onTextChanged(const QString & text);

private:
    QIntValidator * validator;
};
