#pragma once

#include <QDialog>

class QHBoxLayout;
class QVBoxLayout;
class QFormLayout;
class QTreeWidget;
class QStackedWidget;
class QSettings;
class QTreeWidgetItem;
class QLabel;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget * parent = nullptr);
    ~SettingsDialog() override;

signals:
    void changed();

protected slots:
    void onCategorySelected();
    void onChanged();

private:
    QTreeWidgetItem * addPane(QWidget * pane, const QString & name, QTreeWidgetItem * parent);
    void createPanes();
    std::tuple<QWidget *, QFormLayout *> createPane();
    QWidget * createGeneralPane();
    QWidget * createAppearancePane();
    QWidget * createAppearanceMeshPane();
    QWidget * createAppearanceGeometryPane();
    QWidget * createOpenCASCADEPane();

    QHBoxLayout * layout;
    QTreeWidget * categories;
    QLabel * pane_title;
    QStackedWidget * pane;
};
