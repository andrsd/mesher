#pragma once

#include "widgets/toolwindow.h"

class QVBoxLayout;
class QLineEdit;
class QFormLayout;
class QPushButton;
class NameWidget;

class BaseTool : public ToolWindow {
    Q_OBJECT
public:
    explicit BaseTool(const QString & name, QWidget * parent);

protected:
    virtual void updateWidgets() = 0;

    void showEvent(QShowEvent * event) override;

    QFormLayout * layout();

private:
    QFormLayout * form_layout;
};
