#include "tooltitlewidget.h"
#include <QKeyEvent>

ToolTitleWidget::ToolTitleWidget(QWidget * parent) : QLabel(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setFixedHeight(28);
    setStyleSheet("QLabel {"
                  "  background-color: rgb(255, 255, 255);"
                  "  border: 1px solid rgb(255, 255, 255);"
                  "  border-bottom: 1px solid rgb(221, 221, 221);"
                  "  font-weight: 500;"
                  "}");
}

void
ToolTitleWidget::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        return;
    }

    QLabel::keyPressEvent(event);
}

void
ToolTitleWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (this->dragging) {
        auto glob_pos = mapToGlobal(event->pos());
        auto parent = parentWidget();
        parent->move(glob_pos - this->drag_rel_pos);
    }
    event->accept();
}

void
ToolTitleWidget::mousePressEvent(QMouseEvent * event)
{
    this->dragging = true;
    this->drag_rel_pos = event->pos();
    event->accept();
}

void
ToolTitleWidget::mouseReleaseEvent(QMouseEvent * event)
{
    this->dragging = false;
    event->accept();
}
