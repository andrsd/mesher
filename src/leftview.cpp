#include "leftview.h"
#include "tools/basetool.h"
#include <QVariant>

LeftView::LeftView(QWidget * parent) : QTreeWidget(parent)
{
    setMinimumWidth(150);
    setStyleSheet("QTreeView {"
                  "  border: none;"
                  "}"
                  "QTreeView::item {"
                  "  background-color: rgb(255, 255, 255);"
                  "  color: rgb(51, 51, 51);"
                  "}"
                  "QTreeView::item:selected {"
                  "  background-color: rgb(186, 219,243);"
                  "}"
                  "QTreeView::item:selected:hover {"
                  "  background-color: rgb(186, 219,243);"
                  "}"
                  "QTreeView::item:hover {"
                  "  background-color: rgb(226, 240, 250);"
                  "}");
    setHeaderHidden(true);

    connect(this, &QTreeWidget::itemDoubleClicked, this, &LeftView::onItemDoubleClicked);
}

void
LeftView::add(BaseTool * tool)
{
    auto ti = new QTreeWidgetItem(this);
    ti->setText(0, tool->windowTitle());
    ti->setData(0, Qt::UserRole, QVariant::fromValue(tool));
    this->tool2item.insert(tool, ti);
    connect(tool, &ToolWindow::titleChanged, this, &LeftView::onToolTitleChanged);
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

void
LeftView::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    if (item != nullptr) {
        auto * tool = item->data(0, Qt::UserRole).value<BaseTool *>();
        tool->show();
    }
}

void
LeftView::onToolTitleChanged(ToolWindow * tool_wnd)
{
    auto tool = dynamic_cast<BaseTool *>(tool_wnd);
    if (tool) {
        auto it = this->tool2item.find(tool);
        if (it != this->tool2item.end()) {
            auto item = it.value();
            item->setText(0, tool->windowTitle());
        }
    }
}
