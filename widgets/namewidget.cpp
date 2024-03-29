#include "namewidget.h"
#include <QKeyEvent>
#include <QIcon>
#include <QAction>

NameWidget::NameWidget(QWidget * parent) : QLineEdit(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setFixedHeight(28);
    setReadOnly(true);
    setStyleSheet("QLineEdit {"
                  "  border: 1px solid rgb(255, 255, 255);"
                  "  border-bottom: 1px solid rgb(221, 221, 221);"
                  "  font-weight: 600;"
                  "}"
                  "QLineEdit:read-only:focus {"
                  "  border: 1px solid rgb(255, 255, 255);"
                  "  border-bottom: 1px solid rgb(221, 221, 221);"
                  "}"
                  "QLineEdit:focus {"
                  "  font-weight: normal;"
                  "  border: 1px solid rgb(40, 80, 170);"
                  "}");
    setAttribute(Qt::WA_MacShowFocusRect, 0);
    setTextMargins(2, 2, 2, 2);

    QIcon icon(QPixmap(":/i/pencil"));
    this->edit_action = this->addAction(icon, QLineEdit::TrailingPosition);

    connect(this->edit_action, &QAction::triggered, this, &NameWidget::onEdit);
    connect(this, &QLineEdit::editingFinished, this, &NameWidget::onEditingFinished);
}

void
NameWidget::onEdit()
{
    this->previous_text = text();
    setReadOnly(false);
    setFocus(Qt::MouseFocusReason);
    setSelection(0, text().length());
}

void
NameWidget::onEditingFinished()
{
    setReadOnly(true);
    clearFocus();
}

void
NameWidget::keyPressEvent(QKeyEvent * event)
{
    if (!isReadOnly() && event->key() == Qt::Key_Escape) {
        event->accept();
        setText(this->previous_text);
        onEditingFinished();
        return;
    }

    QLineEdit::keyPressEvent(event);
}

void
NameWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (isReadOnly()) {
        if (this->dragging) {
            auto glob_pos = mapToGlobal(event->pos());
            auto parent = parentWidget();
            parent->move(glob_pos - this->drag_rel_pos);
        }
        event->accept();
    }
    else
        QLineEdit::mouseMoveEvent(event);
}

void
NameWidget::mousePressEvent(QMouseEvent * event)
{
    if (isReadOnly()) {
        this->dragging = true;
        this->drag_rel_pos = event->pos();
        event->accept();
    }
    else
        QLineEdit::mousePressEvent(event);
}

void
NameWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if (isReadOnly()) {
        this->dragging = false;
        event->accept();
    }
    else
        QLineEdit::mouseReleaseEvent(event);
}
