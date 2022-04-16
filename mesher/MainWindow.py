import os
import io
import vtk
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QMenuBar, QActionGroup, QApplication, \
    QFileDialog
from PyQt5.QtCore import QEvent, QSettings
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from mesher import consts
from mesher.AboutDialog import AboutDialog
from mesher.MesherInteractorStyle2D import MesherInteractorStyle2D


class MainWindow(QtWidgets.QMainWindow):
    """
    Main window
    """
    WINDOW_TITLE = "Mesher"

    MAX_RECENT_FILES = 10

    def __init__(self):
        super().__init__()
        self.settings = QtCore.QSettings("Mesher")
        self.about_dlg = None
        self.recent_files = []
        self.clear_recent_file = None
        self.file_name = None

        self.recent_files = self.settings.value("recent_files", [])

        self.setupWidgets()
        self.setupMenuBar()
        self.updateWindowTitle()
        self.updateMenuBar()

        self.setAcceptDrops(True)
        self.connectSignals()
        self.setupVtk()

        self.vtk_interactor.Initialize()
        self.vtk_interactor.Start()

        geom = self.settings.value("window/geometry")
        default_size = QtCore.QSize(1000, 700)
        if geom is None:
            self.resize(default_size)
        else:
            if not self.restoreGeometry(geom):
                self.resize(default_size)

    def setupWidgets(self):
        self.vtk_widget = QVTKRenderWindowInteractor(self)
        self.vtk_renderer = vtk.vtkRenderer()
        self.vtk_widget.GetRenderWindow().AddRenderer(self.vtk_renderer)
        self.setCentralWidget(self.vtk_widget)

    def setupMenuBar(self):
        self.menubar = QMenuBar(self)

        file_menu = self.menubar.addMenu("File")
        self.new_action = file_menu.addAction(
            "New", self.onNewFile, "Ctrl+N")
        self.open_action = file_menu.addAction(
            "Open", self.onOpenFile, "Ctrl+O")
        self.recent_menu = file_menu.addMenu("Open Recent")
        self.buildRecentFilesMenu()

        # The "About" item is fine here, since we assume Mac and that will
        # place the item into different submenu but this will need to be fixed
        # for linux and windows
        file_menu.addSeparator()
        self.about_box_action = file_menu.addAction("About", self.onAbout)

        self.window_menu = self.menubar.addMenu("Window")
        self.minimize = self.window_menu.addAction(
            "Minimize", self.onMinimize, "Ctrl+M")
        self.window_menu.addSeparator()
        self.bring_all_to_front = self.window_menu.addAction(
            "Bring All to Front", self.onBringAllToFront)

        self.window_menu.addSeparator()
        self.show_main_window = self.window_menu.addAction(
            "Mesher", self.onShowMainWindow)
        self.show_main_window.setCheckable(True)

        self.action_group_windows = QActionGroup(self)
        self.action_group_windows.addAction(self.show_main_window)

        self.setMenuBar(self.menubar)

    def updateMenuBar(self):
        qapp = QApplication.instance()
        active_window = qapp.activeWindow()
        if active_window == self:
            self.show_main_window.setChecked(True)

    def connectSignals(self):
        pass

    def setupVtk(self):
        self.vtk_render_window = self.vtk_widget.GetRenderWindow()
        self.vtk_interactor = self.vtk_render_window.GetInteractor()

        self.vtk_interactor.SetInteractorStyle(MesherInteractorStyle2D(self))

        bkgnd = [0.95, 0.95, 0.95]
        self.vtk_renderer.SetGradientBackground(True)
        self.vtk_renderer.SetBackground(bkgnd)
        self.vtk_renderer.SetBackground2(bkgnd)
        # set anti-aliasing on
        self.vtk_renderer.SetUseFXAA(True)
        self.vtk_render_window.SetMultiSamples(1)

    def onNewFile(self):
        self.file_name = None
        self.updateWindowTitle()

    def openFile(self, file_name):
        """
        @param file_name[str] Name of the file to open
        """
        self.file_name = file_name
        self.updateWindowTitle()
        self.addToRecentFiles(self.file_name)
        self.updateMenuBar()

    def onOpenFile(self):
        file_name, f = QFileDialog.getOpenFileName(
            self,
            'Open File',
            "",
            "poly files (*.poly)")
        if file_name:
            self.openFile(file_name)

    def onAbout(self):
        if self.about_dlg is None:
            self.about_dlg = AboutDialog(self)
        self.about_dlg.show()

    def onMinimize(self):
        self.showMinimized()

    def onBringAllToFront(self):
        self.showNormal()

    def onShowMainWindow(self):
        self.showNormal()
        self.activateWindow()
        self.raise_()
        self.updateMenuBar()

    def event(self, event):
        if event.type() == QEvent.WindowActivate:
            self.updateMenuBar()
        return super().event(event)

    def closeEvent(self, event):
        self.writeSettings()
        event.accept()

    def writeSettings(self):
        self.settings.setValue("window/geometry", self.saveGeometry())
        self.settings.setValue("recent_files", self.recent_files)

    def buildRecentFilesMenu(self):
        if self.recent_menu is None:
            return

        self.recent_menu.clear()
        if len(self.recent_files) > 0:
            for f in reversed(self.recent_files):
                unused_path, file_name = os.path.split(f)
                action = self.recent_menu.addAction(file_name,
                                                     self.onOpenRecentFile)
                action.setData(f)
            self.recent_menu.addSeparator()
        self.clear_recent_file = self.recent_menu.addAction(
            "Clear Menu", self.onClearRecentFiles)

    def addToRecentFiles(self, file_name):
        """
        @param file_name[str] Name of the file
        """
        self.recent_files = [f for f in self.recent_files if f != file_name]
        self.recent_files.append(file_name)
        if len(self.recent_files) > self.MAX_RECENT_FILES:
            del self.recent_files[0]
        self.buildRecentFilesMenu()

    def onOpenRecentFile(self):
        action = self.sender()
        file_name = action.data()
        self.openFile(file_name)

    def onClearRecentFiles(self):
        self.recent_files = []
        self.buildRecentFilesMenu()
        self.recent_tab.clear()

    def updateWindowTitle(self):
        if self.file_name is None:
            self.setWindowTitle(self.WINDOW_TITLE)
        else:
            self.setWindowTitle("{} \u2014 {}".format(
                self.WINDOW_TITLE, os.path.basename(self.file_name)))

    def onClicked(self, pt):
        pass
