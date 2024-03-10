#pragma once

#include "widgets/toolwindow.h"
#include <vector>

class QVBoxLayout;
class QListWidget;
class QListWidgetItem;
class GEntity;
class View;
class ToolTitleWidget;
class CancelButton;

class SelectOthersDialog : public ToolWindow {
    Q_OBJECT
public:
    SelectOthersDialog(View * view);
    ~SelectOthersDialog() override;

    void setEntities(const std::vector<GEntity *> & ents);

protected slots:
    void onItemEntered(QListWidgetItem * item);
    void onItemClicked(QListWidgetItem * item);

protected:
    View * view;
    QVBoxLayout * layout;
    QListWidget * entities;
};
