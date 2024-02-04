#pragma once

#include <QTreeWidget>

class BaseTool;

class LeftView : public QTreeWidget {
public:
    LeftView(QWidget * parent = nullptr);

    void add(BaseTool * tool);
};
