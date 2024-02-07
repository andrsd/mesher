#include "listwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QShortcut>

ListWidget::ListWidget(QWidget * parent) : QListWidget(parent)
{
    this->del_kbd = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(this->del_kbd, &QShortcut::activated, this, &ListWidget::onDeletePressed);
}

void
ListWidget::addItem(QListWidgetItem * item)
{
    QListWidget::addItem(item);
}

void
ListWidget::onDeletePressed()
{
    auto sel = selectedItems();
    if (sel.length() == 1) {
        auto r = currentRow();
        auto * lwi = takeItem(r);
        delete lwi;
    }
}
