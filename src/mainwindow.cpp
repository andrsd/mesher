#include "mainwindow.h"
#include "mesherconfig.h"
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QProgressDialog>
#include <QFileSystemWatcher>
#include <QActionGroup>
#include <QSettings>
#include <QEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPushButton>
#include <QFileInfo>
#include <QApplication>
#include <QShortcut>

static const int MAX_RECENT_FILES = 10;

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    settings(new QSettings("David Andrs", "Mesher")),
    menu_bar(new QMenuBar(nullptr)),
    recent_menu(nullptr),
    new_action(nullptr),
    open_action(nullptr),
    close_action(nullptr),
    clear_recent_file(nullptr),
    minimize(nullptr),
    bring_all_to_front(nullptr),
    show_main_window(nullptr),
    windows_action_group(nullptr)
{
    QSize default_size = QSize(1000, 700);
    QVariant geom = this->settings->value("window/geometry", default_size);
    if (!this->restoreGeometry(geom.toByteArray()))
        this->resize(default_size);
    this->recent_files = this->settings->value("recent_files", QStringList()).toStringList();

    setupWidgets();
    setupMenuBar();
    updateWindowTitle();
    updateMenuBar();

    setAcceptDrops(true);

    connectSignals();

    clear();
    show();
}

MainWindow::~MainWindow()
{
    clear();
    delete this->settings;
    delete this->menu_bar;
    delete this->windows_action_group;
}

QSettings *
MainWindow::getSettings()
{
    return this->settings;
}

void
MainWindow::setupWidgets()
{
}

void
MainWindow::setupMenuBar()
{
    setMenuBar(this->menu_bar);
    QMenu * file_menu = this->menu_bar->addMenu("File");
    this->new_action =
        file_menu->addAction("New", this, &MainWindow::onNewFile, QKeySequence("Ctrl+N"));
    this->open_action =
        file_menu->addAction("Open", this, &MainWindow::onOpenFile, QKeySequence("Ctrl+O"));
    this->recent_menu = file_menu->addMenu("Open Recent");
    buildRecentFilesMenu();
    file_menu->addSeparator();
    this->close_action =
        file_menu->addAction("Close", this, &MainWindow::onClose, QKeySequence("Ctrl+W"));

    file_menu->addAction("Quit", this, &QCoreApplication::quit, QKeySequence("Ctrl+Q"));

    QMenu * window_menu = this->menu_bar->addMenu("Window");
    this->minimize =
        window_menu->addAction("Minimize", this, &MainWindow::onMinimize, QKeySequence("Ctrl+M"));
    window_menu->addSeparator();
    this->bring_all_to_front =
        window_menu->addAction("Bring All to Front", this, &MainWindow::onBringAllToFront);
    window_menu->addSeparator();
    this->show_main_window = window_menu->addAction("Mesher", this, &MainWindow::onShowMainWindow);
    this->show_main_window->setCheckable(true);

    this->windows_action_group = new QActionGroup(this);
    this->windows_action_group->addAction(this->show_main_window);
}

void
MainWindow::updateMenuBar()
{
    auto * active_window = QApplication::activeWindow();
    this->show_main_window->setChecked(active_window == this);
}

void
MainWindow::updateWindowTitle()
{
    setWindowTitle(MESHER_APP_NAME);
}

void
MainWindow::connectSignals()
{
}

void
MainWindow::clear()
{
}

void
MainWindow::loadFile(const QString & file_name)
{
}

void
MainWindow::buildRecentFilesMenu()
{
    assert(this->recent_menu != nullptr);
    this->recent_menu->clear();
    if (this->recent_files.length() > 0) {
        for (auto it = this->recent_files.rbegin(); it != this->recent_files.rend(); ++it) {
            QString f = *it;
            QFileInfo fi(f);
            auto * action =
                this->recent_menu->addAction(fi.fileName(), this, &MainWindow::onOpenRecentFile);
            action->setData(f);
        }
        this->recent_menu->addSeparator();
    }
    this->clear_recent_file =
        this->recent_menu->addAction("Clear Menu", this, &MainWindow::onClearRecentFiles);
}

void
MainWindow::addToRecentFiles(const QString & file_name)
{
    QStringList rf_list;
    for (auto & f : this->recent_files) {
        if (f.compare(file_name) != 0)
            rf_list.append(f);
    }
    rf_list.append(file_name);
    if (rf_list.length() > MAX_RECENT_FILES)
        rf_list.removeFirst();
    this->recent_files = rf_list;
}

bool
MainWindow::event(QEvent * event)
{
    if (event->type() == QEvent::WindowActivate)
        updateMenuBar();

    return QMainWindow::event(event);
}

void
MainWindow::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent(event);
}

void
MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData() && event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void
MainWindow::dropEvent(QDropEvent * event)
{
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();

        QStringList file_names;
        for (auto & url : event->mimeData()->urls())
            file_names.append(url.toLocalFile());
        if (file_names.length() > 0)
            loadFile(file_names[0]);
    }
    else
        event->ignore();
}

void
MainWindow::closeEvent(QCloseEvent * event)
{
    this->settings->setValue("window/geometry", this->saveGeometry());
    this->settings->setValue("recent_files", this->recent_files);
    QMainWindow::closeEvent(event);
}

void
MainWindow::onClose()
{
    clear();
    hide();
    updateMenuBar();
}

void
MainWindow::onOpenFile()
{
}

void
MainWindow::onOpenRecentFile()
{
    auto action = dynamic_cast<QAction *>(this->sender());
    if (action != nullptr) {
        auto file_name = action->data();
        loadFile(file_name.toString());
    }
}

void
MainWindow::onClearRecentFiles()
{
    this->recent_files = QStringList();
    buildRecentFilesMenu();
}

void
MainWindow::onNewFile()
{
    this->clear();
    this->updateWindowTitle();
    showNormal();
}

void
MainWindow::onUpdateWindow()
{
}

void
MainWindow::onMinimize()
{
    showMinimized();
}

void
MainWindow::onBringAllToFront()
{
    showNormal();
}

void
MainWindow::onShowMainWindow()
{
    showNormal();
    activateWindow();
    raise();
    updateMenuBar();
}
