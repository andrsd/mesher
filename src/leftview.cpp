#include "leftview.h"

LeftView::LeftView(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setStyleSheet("QTreeView { border: none; }");
    this->setHeaderHidden(true);
}
