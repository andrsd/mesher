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
    setStyleSheet("QListWidget {"
                  "  border: 1px solid rgb(221, 221, 221);"
                  "  border-radius: 2;"
                  "  background-color: rgb(255, 255, 255);"
                  "}"
                  "QListWidget:focus {"
                  "  border: 1px solid rgb(40, 80, 170);"
                  "  border-radius: 2;"
                  "  background-color: rgb(223, 240, 249);"
                  "}"
                  "QListWidget::item {"
                  "  background-color: rgb(255, 255, 255);"
                  "  color: rgb(50, 50, 50);"
                  "}"
                  "QListWidget::item:active, QListWidget::item:focus {"
                  "  background-color: rgb(223, 240, 249);"
                  "  color: rgb(50, 50, 50);"
                  "}");
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
