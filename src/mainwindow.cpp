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
#include <QTreeWidget>
#include <QSplitter>
#include <QFileDialog>
#include "aboutdlg.h"
#include "view.h"
#include "settingsdlg.h"
#include "GModel.h"
#include "GModelIO_GEO.h"
#if defined(HAVE_PARSER)
    #include "Parser.h"
    #include "FunctionManager.h"
#endif

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
    windows_action_group(nullptr),
    about_dlg(nullptr),
    splitter(nullptr),
    gmodel(nullptr)
{
    QSize default_size = QSize(1000, 700);
    QVariant geom = this->settings->value("window/geometry", default_size);
    if (!this->restoreGeometry(geom.toByteArray()))
        this->resize(default_size);
    this->recent_files = this->settings->value("recent_files", QStringList()).toStringList();

    setupWidgets();
    setupMenuBar();
    updateMenuBar();

    setAcceptDrops(true);

    connectSignals();

    clear();
    show();
    updateWindowTitle();
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
    this->left = new QTreeWidget(this);
    this->left->setHeaderHidden(true);

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
    auto file_name = this->gmodel->getFileName();
    if (file_name.empty()) {
        setWindowTitle(MESHER_APP_NAME);
    }
    else {
        QFileInfo fi(file_name.c_str());
        QString title = QString("%1 \u2014 %2").arg(MESHER_APP_NAME).arg(fi.fileName());
        setWindowTitle(title);
    }
}

void
MainWindow::connectSignals()
{
}

void
MainWindow::clear()
{
    if (this->gmodel != nullptr) {
        this->gmodel->destroy();
        this->gmodel->getGEOInternals()->destroy();
        delete this->gmodel;
    }
    this->gmodel = new GModel();
}

void
MainWindow::loadFile(const QString & file_name)
{
    QFileInfo fi(file_name);
    if (fi.exists()) {
        this->clear();

        Msg::ResetErrorCounter();
        this->gmodel->setFileName(file_name.toStdString());
        this->gmodel->setName("ASDF");

#if defined(HAVE_PARSER)
        gmsh_yysymbols.clear();
        gmsh_yystringsymbols.clear();
        std::map<std::string, std::vector<double>> cln(Msg::GetCommandLineNumbers());
        for (auto it = cln.begin(); it != cln.end(); it++)
            gmsh_yysymbols[it->first].value = it->second;
        std::map<std::string, std::string> cls(Msg::GetCommandLineStrings());
        for (auto it = cls.begin(); it != cls.end(); it++)
            gmsh_yystringsymbols[it->first] = std::vector<std::string>(1, it->second);
        gmsh_yyfactory.clear();
        gmsh_yynamespaces.clear();
        FunctionManager::Instance()->clear();
#endif
        // FIXME: merge the file
        // MergeFile(fileName, errorIfMissing);
        qDebug() << "load file" << file_name;

        updateWindowTitle();
        addToRecentFiles(file_name);
        buildRecentFilesMenu();
    }
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

    this->gmodel->setFileName("");
    this->gmodel->setName("");
    this->updateWindowTitle();
    Msg::ResetErrorCounter();
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
        this->prefs_dlg = new SettingsDialog(this->settings, this);
    this->prefs_dlg->show();
}
