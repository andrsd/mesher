#pragma once

#include <QDialog>

class QTabWidget;
class QWidget;
class QVBoxLayout;
class QSettings;

class VisibilitySettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit VisibilitySettingsDialog(QWidget * parent);

signals:
    void changed();

protected slots:
    void onChanged();

private:
    std::tuple<QWidget *, QVBoxLayout *> createTab();
    QWidget * createGeoWidget();
    QWidget * createMeshWidget();

    QTabWidget * tabs;
};
