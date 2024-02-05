#pragma once

#include <QTreeWidget>

class BaseTool;

class LeftView : public QTreeWidget {
public:
    LeftView(QWidget * parent = nullptr);

    void add(BaseTool * tool);
    void remove(BaseTool * tool);

private:
    QMap<BaseTool *, QTreeWidgetItem *> tool2item;
};
