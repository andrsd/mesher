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
#include <QSplitter>
#include <QFileDialog>
#include "aboutdlg.h"
#include "leftview.h"
#include "view.h"
#include "settingsdlg.h"
#include "document.h"
#include "loggerdialog.h"
#include "common/toolbar.h"
#include "visibilitysettingsdialog.h"
#include "GmshMessage.h"

static const int MAX_RECENT_FILES = 10;

QSettings * MainWindow::settings = nullptr;

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    menu_bar(new QMenuBar(nullptr)),
    recent_menu(nullptr),
    new_action(nullptr),
    open_action(nullptr),
    close_action(nullptr),
    clear_recent_file(nullptr),
    minimize(nullptr),
    bring_all_to_front(nullptr),
    show_main_window(nullptr),
    windows_action_group(nullptr),
    about_dlg(nullptr),
    splitter(nullptr),
    left(nullptr),
    view(nullptr),
    prefs_dlg(nullptr),
    progress(nullptr),
    doc(new Document()),
    logger(new LoggerDialog(this)),
    visibility_settings_dialog(new VisibilitySettingsDialog(this))
{
    Msg::SetCallback(this->logger);

    QSize default_size = QSize(1000, 700);
    QVariant geom = this->settings->value("window/geometry", default_size);
    if (!this->restoreGeometry(geom.toByteArray()))
        this->resize(default_size);
    this->recent_files = this->settings->value("recent_files", QStringList()).toStringList();

    setupWidgets();
    setupMenuBar();
    setupToolBar();
    updateMenuBar();

    setAcceptDrops(true);

    connectSignals();

    clear();
    show();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete this->menu_bar;
    delete this->windows_action_group;
    delete this->logger;
    delete this->visibility_settings_dialog;
}

QSettings *
MainWindow::getSettings()
{
    if (settings == nullptr)
        settings = new QSettings("David Andrs", "Mesher");
    return settings;
}

void
MainWindow::setupWidgets()
{
    this->left = new LeftView(this);

    this->view = new View(this);

    this->splitter = new QSplitter(Qt::Orientation::Horizontal, this);
    this->splitter->addWidget(this->left);
    this->splitter->addWidget(this->view);
    this->splitter->setHandleWidth(2);
    this->splitter->setStretchFactor(0, 0.);
    this->splitter->setStretchFactor(1, 1.);
    this->splitter->setStyleSheet(QString("QSplitter::handle {"
                                          "    image: none;"
                                          "}"));

    setCentralWidget(this->splitter);
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
    this->save_action =
        file_menu->addAction("Save", this, &MainWindow::onFileSave, QKeySequence("Ctrl+S"));
    this->save_as_action = file_menu->addAction("Save as...", this, &MainWindow::onFileSaveAs);
    file_menu->addSeparator();
    this->close_action =
        file_menu->addAction("Close", this, &MainWindow::onClose, QKeySequence("Ctrl+W"));

    // The "About" item is fine here, since we assume Mac and that will
    // place the item into different submenu but this will need to be fixed
    // for linux and windows
    file_menu->addSeparator();
    file_menu->addAction("About", this, &MainWindow::onAbout);

    file_menu->addSeparator();
    file_menu->addAction("Settings...", this, &MainWindow::onSettings, QKeySequence("Ctrl+,"));

    file_menu->addAction("Quit", this, &QCoreApplication::quit, QKeySequence("Ctrl+Q"));

    QMenu * window_menu = this->menu_bar->addMenu("Window");
    this->minimize =
        window_menu->addAction("Minimize", this, &MainWindow::onMinimize, QKeySequence("Ctrl+M"));
    window_menu->addSeparator();
    this->bring_all_to_front =
        window_menu->addAction("Bring All to Front", this, &MainWindow::onBringAllToFront);
    window_menu->addSeparator();
    window_menu->addAction("Messages", this, &MainWindow::onShowMessages);
    window_menu->addSeparator();
    this->show_main_window = window_menu->addAction("Mesher", this, &MainWindow::onShowMainWindow);
    this->show_main_window->setCheckable(true);

    this->windows_action_group = new QActionGroup(this);
    this->windows_action_group->addAction(this->show_main_window);
}

void
MainWindow::setupToolBar()
{
    this->tool_bar = new ToolBar(this);
    this->tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    this->tool_bar->setFloatable(false);
    this->tool_bar->setMovable(false);
    this->tool_bar->setAllowedAreas(Qt::TopToolBarArea);

    auto visible_action = new QAction("V");
    visible_action->setCheckable(true);
    connect(visible_action, &QAction::toggled, this, &MainWindow::onToggleVisibilitySettings);
    this->tool_bar->addAction(visible_action);
}

void
MainWindow::updateMenuBar()
{
    auto * active_window = QApplication::activeWindow();
    this->show_main_window->setChecked(active_window == this);

    auto has_file = this->doc->hasFile();
    this->save_action->setEnabled(has_file);
}

void
MainWindow::updateWindowTitle()
{
    if (this->doc->hasFile()) {
        QFileInfo fi(this->doc->getFileName());
        QString title = QString("%1 \u2014 %2").arg(MESHER_APP_NAME, fi.fileName());
        setWindowTitle(title);
    }
    else
        setWindowTitle(MESHER_APP_NAME);
}

void
MainWindow::connectSignals()
{
    connect(this->doc, &Document::loadFinished, this, &MainWindow::onLoadFinished);
    connect(this->doc, &Document::saveFinished, this, &MainWindow::onSaveFinished);

    connect(this->visibility_settings_dialog,
            &VisibilitySettingsDialog::changed,
            this,
            &MainWindow::onVisibilitySettingsChanged);
}

void
MainWindow::clear()
{
    this->doc->destroy();
}

void
MainWindow::showProgressDialog(const QString & text)
{
    this->progress = new QProgressDialog(text, QString(), 0, 0, this);
    this->progress->setWindowModality(Qt::WindowModal);
    this->progress->show();
}

void
MainWindow::loadFile(const QString & file_name)
{
    QFileInfo fi(file_name);
    if (fi.exists()) {
        this->clear();
        showProgressDialog(QString("Loading %1...").arg(fi.fileName()));
        this->doc->load(file_name);
    }
}

void
MainWindow::buildRecentFilesMenu()
{
    assert(this->recent_menu != nullptr);
    this->recent_menu->clear();
    if (!this->recent_files.empty()) {
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
        if (!file_names.empty())
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
    QString file_name = QFileDialog::getOpenFileName(this,
                                                     "Open File",
                                                     "",
                                                     "Supported files (*.msh *.geo);;"
                                                     "All files (*);;"
                                                     "GMSH mesh file (*.msh);;"
                                                     "GMSH geometry files (*.geo)");
    if (!file_name.isNull())
        loadFile(file_name);
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
    showNormal();
    this->doc->create();
    this->updateWindowTitle();
    this->view->update();
}

void
MainWindow::onFileSave()
{
    QFileInfo fi(this->doc->getFileName());
    showProgressDialog(QString("Writing %1...").arg(fi.fileName()));
    this->doc->save();
    this->updateWindowTitle();
}

void
MainWindow::onFileSaveAs()
{
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     "Save File As",
                                                     "",
                                                     "Supported files (*.msh *.geo);;"
                                                     "All files (*);;"
                                                     "GMSH mesh file (*.msh);;"
                                                     "GMSH geometry files (*.geo)");
    if (!file_name.isNull()) {
        QFileInfo fi(file_name);
        showProgressDialog(QString("Writing %1...").arg(fi.fileName()));
        this->doc->saveAs(file_name);
        this->updateWindowTitle();
    }
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

void
MainWindow::onAbout()
{
    if (this->about_dlg == nullptr)
        this->about_dlg = new AboutDialog(this);
    this->about_dlg->show();
}

void
MainWindow::onSettings()
{
    if (this->prefs_dlg == nullptr)
        this->prefs_dlg = new SettingsDialog(this);
    this->prefs_dlg->show();
}

void
MainWindow::hideProgressBar()
{
    this->progress->hide();
    delete this->progress;
    this->progress = nullptr;
}

void
MainWindow::onLoadFinished()
{
    hideProgressBar();
    if (this->doc->hasFile()) {
        auto file_name = this->doc->getFileName();
        updateWindowTitle();
        addToRecentFiles(file_name);
        buildRecentFilesMenu();
        showNormal();
        this->view->update();
    }
    else {
        // TODO
        // auto fi = QFileInfo(this->doc->getFileName());
        // showNotification(QString("Unsupported file '%1'.").arg(fi.fileName()));
    }
    this->updateMenuBar();
}

void
MainWindow::onSaveFinished()
{
    hideProgressBar();
    // TODO: check that file was actually saved
    if (true) {
        auto file_name = this->doc->getFileName();
        updateWindowTitle();
        showNormal();
    }
    else {
        // TODO: report that save failed
    }
    this->updateMenuBar();
}

void
MainWindow::onShowMessages()
{
    this->logger->show();
}

void
MainWindow::onToggleVisibilitySettings(bool checked)
{
    if (checked)
        this->visibility_settings_dialog->show();
    else
        this->visibility_settings_dialog->hide();
}

void
MainWindow::onVisibilitySettingsChanged()
{
    this->view->update();
}
