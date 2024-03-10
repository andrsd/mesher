#pragma once

#include <QLineEdit>

class QAction;

class NameWidget : public QLineEdit {
public:
    NameWidget(QWidget * parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

protected slots:
    void onEdit();
    void onEditingFinished();

private:
    QAction * edit_action;

    QString previous_text;
    bool dragging;
    QPoint drag_rel_pos;
};
