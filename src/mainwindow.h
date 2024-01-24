#pragma once

#include <QColor>
#include <QMainWindow>
#include <QTimer>

class QSettings;
class QMenu;
class QActionGroup;
class QResizeEvent;
class QDragEnterEvent;
class AboutDialog;
class QSplitter;
class QTreeWidget;
class View;
class SettingsDialog;
class GModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget * parent = nullptr);
    ~MainWindow() override;

    template <typename T>
    inline qreal
    HIDPI(T value) const
    {
        return devicePixelRatio() * value;
    }

protected:
    QSettings * getSettings();
    void setupWidgets();
    void setupMenuBar();
    void updateMenuBar();
    void updateWindowTitle();
    void connectSignals();
    void clear();
    void loadFile(const QString & file_name);
    void buildRecentFilesMenu();
    void addToRecentFiles(const QString & file_name);

    bool event(QEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

public slots:
    void onClose();
    void onOpenFile();
    void onOpenRecentFile();
    void onClearRecentFiles();
    void onNewFile();
    void onUpdateWindow();
    void onMinimize();
    void onBringAllToFront();
    void onShowMainWindow();
    void onAbout();
    void onSettings();

protected:
    QSettings * settings;
    QMenuBar * menu_bar;
    QMenu * recent_menu;
    QStringList recent_files;

    QAction * new_action;
    QAction * open_action;
    QAction * close_action;
    QAction * clear_recent_file;
    QAction * minimize;
    QAction * bring_all_to_front;
    QAction * show_main_window;

    QActionGroup * windows_action_group;

    AboutDialog * about_dlg;
    QSplitter * splitter;
    QTreeWidget * left;
    View * view;
    SettingsDialog * prefs_dlg;

    //
    GModel * gmodel;
};
