#pragma once

#include <QLineEdit>

class QAction;

class NameWidget : public QLineEdit {
public:
    NameWidget(QWidget * parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent * event) override;

protected slots:
    void onEdit();
    void onEditingFinished();

private:
    QAction * edit_action;

    QString previous_text;
};
