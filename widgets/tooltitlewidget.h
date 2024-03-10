#pragma once

#include <QLabel>

class QAction;

class ToolTitleWidget : public QLabel {
public:
    ToolTitleWidget(QWidget * parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

private:
    bool dragging;
    QPoint drag_rel_pos;
};
