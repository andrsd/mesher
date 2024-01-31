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
class LeftView;
class SettingsDialog;
class QProgressDialog;
class Document;
class LoggerDialog;
class ToolBar;
class VisibilitySettingsDialog;

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
    void setupWidgets();
    void setupMenuBar();
    void setupToolBar();
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
    void showProgressDialog(const QString & text);
    void hideProgressBar();

public slots:
    void onClose();
    void onOpenFile();
    void onOpenRecentFile();
    void onClearRecentFiles();
    void onNewFile();
    void onFileSave();
    void onFileSaveAs();
    void onUpdateWindow();
    void onMinimize();
    void onBringAllToFront();
    void onShowMainWindow();
    void onAbout();
    void onSettings();
    void onLoadFinished();
    void onSaveFinished();
    void onShowMessages();
    void onToggleVisibilitySettings(bool checked);
    void onVisibilitySettingsChanged();
    void onSetX();
    void onSetY();
    void onSetZ();
    void onSet1to1();

protected:
    static QSettings * settings;
    QMenuBar * menu_bar;
    QMenu * recent_menu;
    ToolBar * tool_bar;
    QStringList recent_files;

    QAction * new_action;
    QAction * open_action;
    QAction * save_action;
    QAction * save_as_action;
    QAction * close_action;
    QAction * clear_recent_file;
    QAction * minimize;
    QAction * bring_all_to_front;
    QAction * show_main_window;

    QActionGroup * windows_action_group;

    AboutDialog * about_dlg;
    QSplitter * splitter;
    LeftView * left;
    View * view;
    SettingsDialog * prefs_dlg;
    QProgressDialog * progress;
    VisibilitySettingsDialog * visibility_settings_dialog;

    Document * doc;

    LoggerDialog * logger;

public:
    static QSettings * getSettings();
};
