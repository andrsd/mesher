#pragma once

#include <QDialog>
#include <vector>

class QVBoxLayout;
class QListWidget;
class QListWidgetItem;
class GEntity;
class View;
class ToolTitleWidget;
class CancelButton;

class SelectOthersDialog : public QDialog {
    Q_OBJECT
public:
    SelectOthersDialog(View * view);
    ~SelectOthersDialog() override;

    void setEntities(const std::vector<GEntity *> & ents);

protected slots:
    void onCancel();
    void onItemEntered(QListWidgetItem * item);
    void onItemClicked(QListWidgetItem * item);

protected:
    View * view;
    QVBoxLayout * layout;
    QVBoxLayout * body_layout;
    ToolTitleWidget * title;
    QListWidget * entities;
    CancelButton * cancel;
};
