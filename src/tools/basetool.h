#pragma once

#include <QDialog>

class QVBoxLayout;
class QLineEdit;
class QFormLayout;
class QPushButton;
class NameWidget;

class BaseTool : public QDialog {
    Q_OBJECT
public:
    explicit BaseTool(const QString & name, QWidget * parent);

    QString name() const;
    void setName(const QString & new_name);

protected:
    virtual void updateWidgets() = 0;

    void showEvent(QShowEvent * event) override;
    void enableOkButton(bool state = true);

    QFormLayout * layout();

protected slots:
    void onOK();
    void onCancel();

private:
    QVBoxLayout * laot;
    NameWidget * nm;
    QFormLayout * form_layout;
    QPushButton * ok;
    QPushButton * cancel;
};
