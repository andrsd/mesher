#pragma once

#include <QDialog>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class ToolTitleWidget;
class NameWidget;

class ToolWindow : public QDialog {
    Q_OBJECT
public:
    explicit ToolWindow(QWidget * parent);
    ToolWindow(const QString & name, QWidget * parent);

    void setWindowTitle(const QString title);
    void setLayout(QLayout * layout);

protected:
    void enableOkButton(bool state = true);

protected slots:
    void onOK();
    void onCancel();
    void onTitleChanged();

private:
    void setUpStyle();

    QVBoxLayout * base_layout;
    QHBoxLayout * title_layout;

    ToolTitleWidget * title;
    NameWidget * editable_title;
    QPushButton * ok;
    QPushButton * cancel;
};
