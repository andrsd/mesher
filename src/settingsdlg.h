#pragma once

#include <QDialog>

class QHBoxLayout;
class QVBoxLayout;
class QTreeWidget;
class QStackedWidget;
class QSettings;
class QTreeWidgetItem;

class SettingsDialog : public QDialog {
public:
    explicit SettingsDialog(QSettings * settings, QWidget * parent = nullptr);
    ~SettingsDialog() override;

protected slots:
    void onCategorySelected();

private:
    QTreeWidgetItem * addPane(QWidget * pane, const QString & name, QTreeWidgetItem * parent);
    void createPanes();
    std::tuple<QWidget *, QVBoxLayout *> createPane();
    QWidget * createGeneralPane();
    QWidget * createAppearancePane();
    QWidget * createAppearanceMeshPane();
    QWidget * createAppearanceGeometryPane();
    QWidget * createOpenCASCADEPane();

    QSettings * settings;
    QHBoxLayout * layout;
    QTreeWidget * categories;
    QStackedWidget * pane;
};
