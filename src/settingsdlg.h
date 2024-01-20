#pragma once

#include <QDialog>

class QVBoxLayout;

class SettingsDialog : public QDialog {
public:
    explicit SettingsDialog(QWidget * parent = nullptr);
    ~SettingsDialog() override;

protected slots:

protected:
    QVBoxLayout * layout;
};
