import os
import vtk
import meshio
import pyvista
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QMenuBar, QActionGroup, QApplication, \
    QFileDialog
from PyQt5.QtCore import QEvent, QSettings
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from mesher.AboutDialog import AboutDialog
from mesher.MesherInteractorStyle2D import MesherInteractorStyle2D
from mesher.MesherInteractorStyle3D import MesherInteractorStyle3D
import triangle as tr
import tetgen


class MainWindow(QtWidgets.QMainWindow):
    """
    Main window
    """
    WINDOW_TITLE = "Mesher"

    MAX_RECENT_FILES = 10

    def __init__(self):
        super().__init__()
        self.settings = QSettings("Mesher")
        self.about_dlg = None
        self.recent_files = []
        self.clear_recent_file = None
        self.file_name = None
        self.mesh = None

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

        self.clear()

        self.update_timer = QtCore.QTimer()
        self.update_timer.timeout.connect(self.onUpdateWindow)
        self.update_timer.start(250)

        QtCore.QTimer.singleShot(1, self.updateWidgets)

    def setupWidgets(self):
        self.vtk_widget = QVTKRenderWindowInteractor(self)
        self.vtk_renderer = vtk.vtkRenderer()
        self.vtk_widget.GetRenderWindow().AddRenderer(self.vtk_renderer)
        self.setCentralWidget(self.vtk_widget)

        self.mesh_button = QtWidgets.QPushButton("Mesh", self)
        self.mesh_button.setFixedSize(160, 32)
        self.mesh_button.show()
        self.mesh_button.clicked.connect(self.onMesh)

    def updateWidgets(self):
        geom = self.geometry()
        self.mesh_button.move(
            geom.width() - 5 - self.mesh_button.width(),
            geom.height() - 10 - self.mesh_button.height())
        self.mesh_button.setEnabled(self.poly is not None)

    def setupMenuBar(self):
        self.menubar = QMenuBar(self)

        file_menu = self.menubar.addMenu("File")
        self.new_action = file_menu.addAction(
            "New", self.onNewFile, "Ctrl+N")
        self.open_action = file_menu.addAction(
            "Open", self.onOpenFile, "Ctrl+O")
        self.recent_menu = file_menu.addMenu("Open Recent")
        self.buildRecentFilesMenu()
        file_menu.addSeparator()
        export_menu = file_menu.addMenu("Export as...")
        self.setupExportMenu(export_menu)

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

        self.export_to_exodusii_action.setEnabled(self.mesh is not None)

    def connectSignals(self):
        pass

    def setupVtk(self):
        self.vtk_render_window = self.vtk_widget.GetRenderWindow()
        self.vtk_interactor = self.vtk_render_window.GetInteractor()

        self.vtk_interactor.SetInteractorStyle(MesherInteractorStyle2D(self))

        bkgnd = [0.9098039216, 0.92578125, 0.9450980392]
        self.vtk_renderer.SetGradientBackground(True)
        self.vtk_renderer.SetBackground(bkgnd)
        self.vtk_renderer.SetBackground2(bkgnd)
        # set anti-aliasing on
        self.vtk_renderer.SetUseFXAA(True)
        self.vtk_render_window.SetMultiSamples(1)

    def onNewFile(self):
        self.clear()
        self.file_name = None
        self.updateWindowTitle()
        self.updateWidgets()

    def openFile(self, file_name):
        """
        @param file_name[str] Name of the file to open
        """
        self.clear()
        base_name, ext = os.path.splitext(file_name)
        if ext == '.poly':
            success = self.openPolyFile(file_name)
            self.dim = 2
        elif ext == '.stl':
            success = self.openSTLFile(file_name)
            self.dim = 3
        else:
            print("Unknown file extension")
            success = False

        if success:
            if self.dim == 3:
                style = MesherInteractorStyle3D(self)
            else:
                style = MesherInteractorStyle2D(self)
            self.vtk_interactor.SetInteractorStyle(style)

            self.resetCamera()
            self.file_name = file_name
            self.updateWindowTitle()
            self.addToRecentFiles(self.file_name)
            self.updateMenuBar()
            self.updateWidgets()

    def openPolyFile(self, file_name):
        """
        @param file_name[str] Name of the poly file to open
        """
        self.poly = self.readPolyFile(file_name)
        if self.poly is not None:
            self.polyToVtk(self.poly)
            return True
        else:
            # TODO: improve this
            print("Error reading {}.".format(file_name))
            return False

    def openSTLFile(self, file_name):
        """
        @param file_name[str] Name of the STL file to open
        """
        reader = pyvista.get_reader(file_name)
        # TODO: error handling
        self.poly = reader.read()
        self.stlToVtk(self.poly)
        return True

    def onOpenFile(self):
        file_name, f = QFileDialog.getOpenFileName(
            self,
            'Open File',
            "",
            "All supported files (*poly *.stl);;"
            "poly files (*.poly);;"
            "STL files (*.stl)")
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

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.updateWidgets()

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

    def updateWindowTitle(self):
        if self.file_name is None:
            self.setWindowTitle(self.WINDOW_TITLE)
        else:
            self.setWindowTitle("{} \u2014 {}".format(
                self.WINDOW_TITLE, os.path.basename(self.file_name)))

    def onClicked(self, pt):
        pass

    def clear(self):
        self.dim = None
        self.poly = None
        self.mesh = None
        self.file_name = None
        self.vtk_renderer.RemoveAllViewProps()
        self.vtk_vertex_actor = None
        self.vtk_segment_actor = None
        self.vtk_mesh_actor = None

    def onUpdateWindow(self):
        self.vtk_render_window.Render()

    def resetCamera(self):
        camera = self.vtk_renderer.GetActiveCamera()
        focal_point = camera.GetFocalPoint()
        camera.SetPosition(focal_point[0], focal_point[1], 1)
        camera.SetRoll(0)
        self.vtk_renderer.ResetCamera()

    def readPolyFile(self, file_name):
        data = None
        with open(file_name) as f:
            str_poly = f.read()
            data = tr.loads(poly=str_poly)

        return data

    def polyToVtk(self, poly):
        self.vtk_renderer.RemoveAllViewProps()
        vertex_ugrid = self.verticesToUnstructuredGrid(poly)
        if vertex_ugrid is not None:
            self.vtk_vertex_actor = self.addToVtk(vertex_ugrid)
            self.setVertexProperties(self.vtk_vertex_actor)
        else:
            # TODO: reprt this to user in GUI
            print("No vertices found in poly file")
            self.vtk_vertex_actor = None

        segment_ugrid = self.segmentsToUnstructuredGrid(poly)
        if segment_ugrid is not None:
            self.vtk_segment_actor = self.addToVtk(segment_ugrid)
            self.setSegmentProperties(self.vtk_segment_actor)
        else:
            self.vtk_segment_actor = None

    def stlToVtk(self, poly):
        self.vtk_renderer.RemoveAllViewProps()
        grid = pyvista.UnstructuredGrid(poly)
        self.vtk_segment_actor = self.addToVtk(grid)
        self.setSurface3DProperties(self.vtk_segment_actor)

    def meshToVtk(self, grid):
        self.vtk_renderer.RemoveAllViewProps()
        if grid is not None:
            actor = self.addToVtk(grid)
            self.setMeshProperties(actor)
        else:
            actor = None
        return actor

    def verticesToUnstructuredGrid(self, poly):
        """
        Creates an unstructured grid with vertices
        """
        if 'vertices' in poly:
            points = poly['vertices']
            n_points = len(points)

            point_array = vtk.vtkPoints()
            point_array.Allocate(n_points)
            cell_array = vtk.vtkCellArray()
            cell_array.Allocate(n_points)

            for i, pt in enumerate(points):
                point_array.InsertPoint(i, [pt[0], pt[1], 0.])

                elem = vtk.vtkVertex()
                elem.GetPointIds().SetId(0, i)
                cell_array.InsertNextCell(elem)

            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(point_array)
            ugrid.SetCells(vtk.VTK_VERTEX, cell_array)

            return ugrid
        else:
            return None

    def segmentsToUnstructuredGrid(self, poly):
        if 'segments' in poly:
            points = poly['vertices']
            n_points = len(points)
            point_array = vtk.vtkPoints()
            point_array.Allocate(n_points)
            for i, pt in enumerate(points):
                point_array.InsertPoint(i, [pt[0], pt[1], 0.])

            segments = poly['segments']
            n_segments = len(segments)
            cell_array = vtk.vtkCellArray()
            cell_array.Allocate(n_segments)
            for seg in list(segments):
                elem = vtk.vtkLine()
                elem.GetPointIds().SetId(0, int(seg[0]))
                elem.GetPointIds().SetId(1, int(seg[1]))
                cell_array.InsertNextCell(elem)

            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(point_array)
            ugrid.SetCells(vtk.VTK_LINE, cell_array)

            return ugrid
        else:
            return None

    def trianglesToUnstructuredGrid(self, poly):
        if 'triangles' in poly:
            points = poly['vertices']
            n_points = len(points)
            point_array = vtk.vtkPoints()
            point_array.Allocate(n_points)
            for i, pt in enumerate(points):
                point_array.InsertPoint(i, [pt[0], pt[1], 0.])

            triangles = poly['triangles']
            n_triangles = len(triangles)
            cell_array = vtk.vtkCellArray()
            cell_array.Allocate(n_triangles)
            for tri in list(triangles):
                elem = vtk.vtkTriangle()
                elem.GetPointIds().SetId(0, int(tri[0]))
                elem.GetPointIds().SetId(1, int(tri[1]))
                elem.GetPointIds().SetId(2, int(tri[2]))
                cell_array.InsertNextCell(elem)

            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(point_array)
            ugrid.SetCells(vtk.VTK_TRIANGLE, cell_array)

            return ugrid
        else:
            return None

    def addToVtk(self, ugrid):
        mapper = vtk.vtkDataSetMapper()
        mapper.SetInputData(ugrid)
        mapper.ScalarVisibilityOff()

        actor = vtk.vtkActor()
        actor.SetMapper(mapper)
        actor.VisibilityOn()
        self.vtk_renderer.AddActor(actor)
        return actor

    def setVertexProperties(self, actor):
        property = actor.GetProperty()
        property.SetRepresentationToPoints()
        property.SetRenderPointsAsSpheres(True)
        property.SetVertexVisibility(True)
        property.SetPointSize(15)
        property.SetColor([0., 0., 0.])

    def setSegmentProperties(self, actor):
        property = actor.GetProperty()
        property.EdgeVisibilityOn()
        property.SetLineWidth(3.0)
        property.SetColor([0, 0, 0])
        property.SetOpacity(1)
        property.SetAmbient(1)
        property.SetDiffuse(0)

    def setSurface3DProperties(self, actor):
        property = actor.GetProperty()
        property.EdgeVisibilityOn()
        property.SetLineWidth(3.0)
        property.SetEdgeColor([0, 0, 0])

        property.SetColor([0.9, 0.9, 0.9])
        property.SetOpacity(1)
        property.SetAmbient(1)
        property.SetDiffuse(0)

        property.SetVertexVisibility(False)
        property.SetRenderPointsAsSpheres(False)
        property.SetPointSize(0)

    def setMeshProperties(self, actor):
        property = actor.GetProperty()
        property.SetRepresentationToSurface()
        property.SetColor([1, 1, 1])
        property.SetOpacity(1)
        property.SetAmbient(1)
        property.SetDiffuse(0)

        property.SetEdgeVisibility(True)
        property.SetLineWidth(3.0)
        property.SetEdgeColor([0, 0, 0])

        property.SetVertexVisibility(False)
        property.SetRenderPointsAsSpheres(False)
        property.SetPointSize(0)

    def onMesh(self):
        opts = ''
        if isinstance(self.poly, dict):
            tris = tr.triangulate(self.poly, opts)
            self.mesh = self.trianglesToUnstructuredGrid(tris)
        elif isinstance(self.poly, pyvista.core.pointset.PolyData):
            tet = tetgen.TetGen(self.poly)
            # TODO: user defined options
            # tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
            tet.tetrahedralize(order=1)
            self.mesh = tet.grid
        else:
            self.mesh = None
        self.vtk_mesh_actor = self.meshToVtk(self.mesh)
        self.updateMenuBar()

    def setupExportMenu(self, menu):
        self.export_to_exodusii_action = menu.addAction(
            "ExodusII...", self.onExportAsExodusII)

    def getFileName(self, window_title, name_filter, default_suffix):
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle(window_title)
        dialog.setNameFilter(name_filter)
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setDefaultSuffix(default_suffix)

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            return str(dialog.selectedFiles()[0])
        return None

    def onExportAsExodusII(self):
        file_name = self.getFileName('Export to ExodusII',
                                     'ExodusII files (*.e *.exo)',
                                     'e')
        if file_name:
            grid = pyvista.UnstructuredGrid(self.mesh)
            if self.dim == 2:
                vertices = grid.points[:, :2]
                m = meshio.Mesh(
                    vertices,
                    [
                        ('triangle', grid.cells_dict[vtk.VTK_TRIANGLE])
                    ]
                )
            elif self.dim == 3:
                vertices = grid.points
                m = meshio.Mesh(
                    vertices,
                    [
                        ('tetra', grid.cells_dict[vtk.VTK_TETRA])
                    ]
                )
            m.write(file_name)
