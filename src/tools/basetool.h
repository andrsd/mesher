#pragma once

#include <QDialog>

class QVBoxLayout;
class QLineEdit;
class QFormLayout;
class QPushButton;

class BaseTool : public QDialog {
    Q_OBJECT
public:
    explicit BaseTool(const QString & name, QWidget * parent);

    QString name() const;

protected:
    virtual void updateWidgets() = 0;

    void showEvent(QShowEvent * event) override;
    void enableOkButton(bool state = true);

    QFormLayout * layout();

private:
    QVBoxLayout * laot;
    QLineEdit * nm;
    QFormLayout * form_layout;
    QPushButton * ok;
};
