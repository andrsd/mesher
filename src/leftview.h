#pragma once

#include <QTreeWidget>

class BaseTool;

class LeftView : public QTreeWidget {
public:
    LeftView(QWidget * parent = nullptr);

    void add(BaseTool * tool);
    void remove(BaseTool * tool);

protected slots:
    void onItemDoubleClicked(QTreeWidgetItem * item, int column);

private:
    QMap<BaseTool *, QTreeWidgetItem *> tool2item;
};
