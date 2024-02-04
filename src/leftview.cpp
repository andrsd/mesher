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
}
