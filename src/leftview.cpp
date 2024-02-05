#include "leftview.h"
#include "tools/basetool.h"
#include <QVariant>

LeftView::LeftView(QWidget * parent) : QTreeWidget(parent)
{
    this->setStyleSheet("QTreeView { border: none; }");
    this->setHeaderHidden(true);
}

void
LeftView::add(BaseTool * tool)
{
    auto ti = new QTreeWidgetItem(this);
    ti->setText(0, tool->name());
    ti->setData(0, Qt::UserRole, QVariant::fromValue(tool));
    this->tool2item.insert(tool, ti);
}

void
LeftView::remove(BaseTool * tool)
{
    auto it = this->tool2item.find(tool);
    if (it != this->tool2item.end()) {
        auto tree_item = *it;
        auto * tool = tree_item->data(0, Qt::UserRole).value<BaseTool *>();
        auto root = invisibleRootItem();
        root->removeChild(tree_item);
        this->tool2item.remove(tool);
        tool->deleteLater();
    }
}
