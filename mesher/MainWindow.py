import os
import vtk
import meshio
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QMenuBar, QActionGroup, QApplication, \
    QFileDialog, QShortcut, QDialog
from PyQt5.QtCore import QEvent, QSettings, Qt, QTimer
from PyQt5.QtGui import QKeySequence
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from mesher.AboutDialog import AboutDialog
from mesher.MesherInteractorStyle2D import MesherInteractorStyle2D
from mesher.MesherInteractorStyle3D import MesherInteractorStyle3D
from mesher.NotificationWidget import NotificationWidget
from mesher.OptionsTetGenWidget import OptionsTetGenWidget
from mesher.OptionsTriangleWidget import OptionsTriangleWidget
from mesher.AssignMarkerDlg import AssignMarkerDlg
from mesher import exodusII
from mesher import vtk_helpers
import triangle
import meshpy.triangle
import meshpy.tet


def round_trip_connect(start, end):
    result = []
    for i in range(start, end):
        result.append((i, i + 1))
    result.append((end, start))
    return result


class MainWindow(QtWidgets.QMainWindow):
    """Main window"""

    WINDOW_TITLE = "Mesher"
    MAX_RECENT_FILES = 10

    SELECTION_CLR = [255, 173, 79]
    SELECTION_EDGE_CLR = [179, 95, 0]

    def __init__(self):
        """Inits MainWindow"""
        super().__init__()
        self.point_size = 15
        self.line_width = 3.

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

        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.onUpdateWindow)
        self.update_timer.start(250)

        QtCore.QTimer.singleShot(1, self.updateWidgets)

    def setupWidgets(self):
        self.vtk_widget = QVTKRenderWindowInteractor(self)
        self.vtk_renderer = vtk.vtkRenderer()
        self.vtk_widget.GetRenderWindow().AddRenderer(self.vtk_renderer)
        self.setCentralWidget(self.vtk_widget)

        self.setupNotificationWidget()

        self.opts_tri_dlg = OptionsTriangleWidget(self.settings, self)
        self.opts_tri_dlg.mesh_button.clicked.connect(self.onMeshClicked)
        self.opts_tri_dlg.setVisible(False)
        self.opts_tet_dlg = OptionsTetGenWidget(self.settings, self)
        self.opts_tet_dlg.mesh_button.clicked.connect(self.onMeshClicked)
        self.opts_tet_dlg.setVisible(False)

    def setupNotificationWidget(self):
        self.notification = NotificationWidget(self)
        self.notification.setVisible(False)

    def updateWidgets(self):
        meshing_possible = self.info is not None
        self.mesh_action.setEnabled(meshing_possible)

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
        self.mesh_action = file_menu.addAction("Mesh...", self.onMesh)
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

        self.export_to_exodusii_action.setEnabled(self.isMeshView())

    def connectSignals(self):
        self.mesh_shortcut = QShortcut(QKeySequence("Ctrl+Return"), self)
        self.mesh_shortcut.activated.connect(self.onMeshClicked)

        self.esc_shortcut = QShortcut(QKeySequence("Escape"), self)
        self.esc_shortcut.activated.connect(self.onHideMeshingOptions)

        self.deselect_all_shortcut = QShortcut(QKeySequence("Space"), self)
        self.deselect_all_shortcut.activated.connect(self.onDeselectAll)

        self.assign_marker_shortcut = QShortcut(QKeySequence("M"), self)
        self.assign_marker_shortcut.activated.connect(self.onAssignMarker)

    def setupVtk(self):
        self.vtk_render_window = self.vtk_widget.GetRenderWindow()
        self.vtk_interactor = self.vtk_render_window.GetInteractor()

        self.vtk_interactor.SetInteractorStyle(MesherInteractorStyle2D(self))

        bkgnd = vtk_helpers.rgb2vtk([232, 236, 241])
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
        self.opts_tri_dlg.hide()
        self.opts_tet_dlg.hide()

    def openFile(self, file_name):
        """
        @param file_name[str] Name of the file to open
        """
        self.clear()
        _, ext = os.path.splitext(file_name)
        if ext == '.poly':
            success = self.openPolyFile(file_name)
        elif ext == '.stl':
            success = self.openSTLFile(file_name)
        else:
            self.showNotification("Unsupported file format")
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
        self.info = self.readPolyFile(file_name)
        if self.info is not None:
            return self.meshInfoToVtk()
        else:
            return False

    def openSTLFile(self, file_name):
        """
        @param file_name[str] Name of the STL file to open
        """
        try:
            m = meshio.read(file_name)
            self.dim = 3
            self.info = meshpy.tet.MeshInfo()
            self.info.set_points(m.points)
            self.info.set_facets(m.cells_dict['triangle'])
            self.surfaceToVtk(m)
            return True
        except meshio.ReadError as e:
            self.showNotification("Error reading '{}': {}".format(
                os.path.basename(file_name), str(e)))
            return False

    def onOpenFile(self):
        file_name, _ = QFileDialog.getOpenFileName(
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
        dlg = self.getMeshingOptionsDialog()
        if dlg is not None:
            self.updateMeshingOptionsGeometry(dlg)

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
        if self.isGeometryView():
            # highlighting is done only on geometries, not meshes
            if self.dim == 2:
                self.selectSegment(pt)
            else:
                self.selectFacet(pt)

    def clear(self):
        self.dim = None
        self.info = None
        self.mesh = None
        self.file_name = None
        self.vtk_renderer.RemoveAllViewProps()
        self.vtk_vertex_actor = None
        self.vtk_segment_actor = None
        self.vtk_hole_actor = None
        self.vtk_mesh_actor = None
        self.higlight_actor = None
        self.selected_actors = {}
        self.facet_marker = {}

    def onUpdateWindow(self):
        self.vtk_render_window.Render()

    def resetCamera(self):
        camera = self.vtk_renderer.GetActiveCamera()
        focal_point = camera.GetFocalPoint()
        camera.SetPosition(focal_point[0], focal_point[1], 1)
        camera.SetRoll(0)
        self.vtk_renderer.ResetCamera()

    def readPolyFile(self, file_name):
        try:
            with open(file_name) as f:
                str_poly = f.read()
                data = triangle.loads(poly=str_poly)

                if 'segments' in data:
                    info = meshpy.triangle.MeshInfo()
                    vertices = data['vertices']
                    info.set_points(vertices)
                    segs = data['segments']
                    info.set_facets(segs)
                    if 'holes' in data:
                        info.set_holes(data['holes'])
                    self.dim = 2
                    return info
                else:
                    self.showNotification("No segments found in the file.")
                    self.dim = None
                    return None

        except Exception:
            info = meshpy.tet.MeshInfo()
            file_base_name = os.path.splitext(file_name)[0]
            try:
                info.load_poly(file_base_name)
                self.dim = 3
                return info
            except RuntimeError:
                self.showNotification("Error reading '{}'".format(
                    os.path.basename(file_name)))
                return None

    def meshInfoToVtk(self):
        if self.dim == 2:
            self.infoToVtk2D(self.info)
            return True
        elif self.dim == 3:
            self.infoToVtk3D(self.info)
            return True
        else:
            self.showNotification(
                "Unsupported dimension in poly file '{}'".format(self.dim))
            return False

    def infoToVtk2D(self, info):
        self.segments2DToVtk(info)

        if info.holes is not None:
            holes_ugrid = vtk_helpers.vertices2DToUnstructuredGrid(info.holes)
            if holes_ugrid is not None:
                self.vtk_hole_actor = self.addToVtk(holes_ugrid)
                self.setHolesProperties(self.vtk_hole_actor)
            else:
                self.vtk_hole_actor = None

    def infoToVtk3D(self, info):
        self.facets3DToVtk(info)

        if info.holes is not None:
            holes_ugrid = vtk_helpers.vertices3DToUnstructuredGrid(info.holes)
            if holes_ugrid is not None:
                self.vtk_hole_actor = self.addToVtk(holes_ugrid)
                self.setHolesProperties(self.vtk_hole_actor)
            else:
                self.vtk_hole_actor = None

    def surfaceToVtk(self, surface):
        self.vtk_vertex_actor = None
        grid = self.triangles3DToUnstructuredGrid(surface)
        self.vtk_segment_actor = self.addToVtk(grid)
        self.setSurface3DProperties(self.vtk_segment_actor)

    def meshToVtk(self, grid):
        if grid is not None:
            actor = self.addToVtk(grid)
            self.setMeshProperties(actor)
        else:
            actor = None
        return actor

    def segments2DToUnstructuredGrid(self, info):
        points = vtk_helpers.buildPointArray2D(info.points)
        lines = vtk_helpers.buildCellArrayLine(info.facets)
        ugrid = vtk.vtkUnstructuredGrid()
        ugrid.SetPoints(points)
        ugrid.SetCells(vtk.VTK_LINE, lines)
        return ugrid

    def triangles2DToUnstructuredGrid(self, mesh):
        points = vtk_helpers.buildPointArray2D(mesh.points)
        triangles = vtk_helpers.buildCellArrayTriangle(mesh.elements)
        ugrid = vtk.vtkUnstructuredGrid()
        ugrid.SetPoints(points)
        ugrid.SetCells(vtk.VTK_TRIANGLE, triangles)
        return ugrid

    def triangles3DToUnstructuredGrid(self, mesh):
        if 'triangle' in mesh.cells_dict:
            points = vtk_helpers.buildPointArray3D(mesh.points)
            triangles = vtk_helpers.buildCellArrayTriangle(
                mesh.cells_dict['triangle'])
            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(points)
            ugrid.SetCells(vtk.VTK_TRIANGLE, triangles)
            return ugrid
        else:
            return None

    def segments2DToVtk(self, info):
        """
        Create unstructured grids one for each segment
        """
        self.vtk_segment_actor = []
        self.facet_marker = {}
        for facet in list(info.facets):
            pt_map = {}
            pts = vtk.vtkPoints()
            cells = vtk.vtkCellArray()
            elem = vtk.vtkLine()
            for i, vertex_id in enumerate(facet):
                vertex_id = int(vertex_id)
                if vertex_id in pt_map:
                    vtk_pt_id = pt_map[vertex_id]
                else:
                    pt = info.points[vertex_id]
                    vtk_pt_id = pts.InsertNextPoint(pt[0], pt[1], 0)
                    pt_map[vertex_id] = vtk_pt_id
                elem.GetPointIds().SetId(i, vtk_pt_id)
            cells.InsertNextCell(elem)

            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(pts)
            ugrid.SetCells(vtk.VTK_LINE, cells)
            actor = self.addToVtk(ugrid)
            self.vtk_segment_actor.append(actor)
            self.setSegmentProperties(actor)
            # TODO: get the marker from `info`
            self.facet_marker[actor] = 1

    def facets3DToVtk(self, info):
        """
        Create unstructured grids one for each facet
        """
        self.vtk_segment_actor = []
        for facet in list(info.facets):
            pt_map = {}
            pts = vtk.vtkPoints()
            cells = vtk.vtkCellArray()
            for poly in list(facet.polygons):
                elem = vtk.vtkPolygon()
                elem.GetPointIds().SetNumberOfIds(len(poly.vertices))
                for i, vertex_id in enumerate(poly.vertices):
                    vertex_id = int(vertex_id)
                    if vertex_id in pt_map:
                        vtk_pt_id = pt_map[vertex_id]
                    else:
                        pt = info.points[vertex_id]
                        vtk_pt_id = pts.InsertNextPoint(pt[0], pt[1], pt[2])
                        pt_map[vertex_id] = vtk_pt_id
                    elem.GetPointIds().SetId(i, vtk_pt_id)
                cells.InsertNextCell(elem)

            ugrid = vtk.vtkUnstructuredGrid()
            ugrid.SetPoints(pts)
            ugrid.SetCells(vtk.VTK_POLYGON, cells)
            actor = self.addToVtk(ugrid)
            self.vtk_segment_actor.append(actor)
            self.setSurface3DProperties(actor)

    def tetrasToUnstructuredGrid(self, mesh):
        points = vtk_helpers.buildPointArray3D(mesh.points)
        tetras = vtk_helpers.buildCellArrayTetra(mesh.elements)
        ugrid = vtk.vtkUnstructuredGrid()
        ugrid.SetPoints(points)
        ugrid.SetCells(vtk.VTK_TETRA, tetras)
        return ugrid

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
        prop = actor.GetProperty()
        prop.SetRepresentationToPoints()
        prop.SetRenderPointsAsSpheres(True)
        prop.SetVertexVisibility(True)
        prop.SetPointSize(self.point_size)
        prop.SetColor(vtk_helpers.rgb2vtk([0, 0, 0]))

    def setSegmentProperties(self, actor):
        prop = actor.GetProperty()
        prop.EdgeVisibilityOn()
        prop.SetLineWidth(self.line_width)
        prop.SetColor(vtk_helpers.rgb2vtk([0, 0, 0]))
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

    def setHolesProperties(self, actor):
        prop = actor.GetProperty()
        prop.SetRepresentationToPoints()
        prop.SetRenderPointsAsSpheres(True)
        prop.SetVertexVisibility(True)
        prop.SetPointSize(self.point_size)
        prop.SetColor(vtk_helpers.rgb2vtk([255, 0, 0]))

    def setSurface3DProperties(self, actor):
        prop = actor.GetProperty()
        prop.EdgeVisibilityOn()
        prop.SetLineWidth(self.line_width)
        prop.SetEdgeColor(vtk_helpers.rgb2vtk([0, 0, 0]))

        prop.SetColor(vtk_helpers.rgb2vtk([230, 230, 230]))
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

        prop.SetVertexVisibility(False)
        prop.SetRenderPointsAsSpheres(False)
        prop.SetPointSize(0)

    def setMeshProperties(self, actor):
        prop = actor.GetProperty()
        prop.SetRepresentationToSurface()
        prop.SetColor(vtk_helpers.rgb2vtk([255, 255, 255]))
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

        prop.SetEdgeVisibility(True)
        prop.SetLineWidth(self.line_width)
        prop.SetEdgeColor(vtk_helpers.rgb2vtk([0, 0, 0]))

        prop.SetVertexVisibility(False)
        prop.SetRenderPointsAsSpheres(False)
        prop.SetPointSize(0)

    def setHighlightSegmentProperties(self, actor, selected):
        prop = actor.GetProperty()
        prop.SetRepresentationToSurface()
        prop.SetRenderPointsAsSpheres(False)
        prop.SetVertexVisibility(False)
        prop.SetPointSize(0)
        prop.EdgeVisibilityOn()
        # TODO: move colors into class variables
        if selected:
            prop.SetColor(vtk_helpers.rgb2vtk([237, 181, 69]))
        else:
            prop.SetColor(vtk_helpers.rgb2vtk([255, 207, 110]))
        prop.SetLineWidth(self.line_width + 4)
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

    def setSelectedSegmentProperties(self, actor):
        prop = actor.GetProperty()
        prop.SetRepresentationToSurface()
        prop.SetRenderPointsAsSpheres(False)
        prop.SetVertexVisibility(False)
        prop.SetPointSize(0)
        prop.EdgeVisibilityOn()
        prop.SetColor(vtk_helpers.rgb2vtk(self.SELECTION_EDGE_CLR))
        prop.SetLineWidth(self.line_width + 4)
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

    def setHighlightFacetProperties(self, actor, selected):
        prop = actor.GetProperty()
        prop.SetRepresentationToSurface()
        prop.SetRenderPointsAsSpheres(False)
        prop.SetVertexVisibility(False)
        prop.SetPointSize(0)
        prop.EdgeVisibilityOn()
        prop.SetLineWidth(self.line_width + 2)
        prop.SetEdgeColor(vtk_helpers.rgb2vtk(self.SELECTION_EDGE_CLR))
        # TODO: move colors into class variables
        if selected:
            prop.SetColor(vtk_helpers.rgb2vtk([237, 181, 69]))
        else:
            prop.SetColor(vtk_helpers.rgb2vtk([255, 207, 110]))
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

    def setSelectedFacetProperties(self, actor):
        prop = actor.GetProperty()
        prop.SetRepresentationToSurface()
        prop.SetRenderPointsAsSpheres(False)
        prop.SetVertexVisibility(False)
        prop.SetPointSize(0)
        prop.EdgeVisibilityOn()
        prop.SetLineWidth(self.line_width + 4)
        prop.SetEdgeColor(vtk_helpers.rgb2vtk(self.SELECTION_EDGE_CLR))
        prop.SetColor(vtk_helpers.rgb2vtk(self.SELECTION_CLR))
        prop.SetOpacity(1)
        prop.SetAmbient(1)
        prop.SetDiffuse(0)

    def onMesh(self):
        self.showMeshingOptions()

    def onMeshClicked(self):
        if isinstance(self.info, meshpy.triangle.MeshInfo):
            self.setFacetMarkers()
            params = self.opts_tri_dlg.getParams()
            self.mesh = meshpy.triangle.build(self.info, **params)
            grid = self.triangles2DToUnstructuredGrid(self.mesh)
        elif isinstance(self.info, meshpy.tet.MeshInfo):
            self.setFacetMarkers()
            params = self.opts_tet_dlg.getParams()
            self.mesh = meshpy.tet.build(self.info, **params)
            grid = self.tetrasToUnstructuredGrid(self.mesh)
        else:
            grid = None

        self.vtk_renderer.RemoveAllViewProps()
        self.vtk_mesh_actor = self.meshToVtk(grid)
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
            if self.dim == 2:
                m = meshio.Mesh(
                    self.mesh.points,
                    [
                        ('triangle', self.mesh.elements)
                    ]
                )
                setattr(m, 'side_sets', self.createExodusIISideSets())
            elif self.dim == 3:
                m = meshio.Mesh(
                    self.mesh.points,
                    [
                        ('tetra', self.mesh.elements)
                    ]
                )
                setattr(m, 'side_sets', self.createExodusIISideSets())
            exodusII.write(file_name, m)
            self.showNotification("File '{}' exported sucessfully".format(
                os.path.basename(file_name)))

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.accept()
        else:
            event.ignore()

    def dropEvent(self, event):
        if event.mimeData().hasUrls():
            event.setDropAction(Qt.CopyAction)
            event.accept()

            file_names = []
            for url in event.mimeData().urls():
                file_names.append(url.toLocalFile())
            if len(file_names) > 0:
                self.openFile(file_names[0])
        else:
            event.ignore()

    def showNotification(self, text, ms=2000):
        """
        @param text Notification text
        @param ms Timeout for fade out in milliseconds
        """
        self.notification.setText(text)
        self.notification.adjustSize()
        width = self.geometry().width()
        left = (width - self.notification.width()) / 2
        # top = 10
        top = self.height() - self.notification.height() - 10
        self.notification.setGeometry(
            left, top,
            self.notification.width(),
            self.notification.height())
        self.notification.setGraphicsEffect(None)
        self.notification.show(ms)

    def getMeshingOptionsDialog(self):
        if self.dim == 2:
            return self.opts_tri_dlg
        elif self.dim == 3:
            return self.opts_tet_dlg
        else:
            return None

    def showMeshingOptions(self):
        dlg = self.getMeshingOptionsDialog()
        if dlg is not None:
            self.updateMeshingOptionsGeometry(dlg)
            dlg.show()

    def updateMeshingOptionsGeometry(self, dlg):
        dlg.adjustSize()
        margin = 10
        width = self.geometry().width()
        height = self.geometry().height() - (2 * margin)
        left = (width - dlg.width()) - margin
        top = margin
        dlg.setGeometry(left, top, dlg.width(), height)

    def onHideMeshingOptions(self):
        dlg = self.getMeshingOptionsDialog()
        if dlg is not None:
            dlg.hide()

    def onMouseMove(self, pt):
        if self.isGeometryView():
            # highlighting is done only on geometries, not meshes
            if self.dim == 2:
                self.highlightSegment(pt)
            else:
                self.highlightFacet(pt)

    def pickActor(self, pt):
        picker = vtk.vtkPropPicker()
        if picker.PickProp(pt.x(), pt.y(), self.vtk_renderer):
            return picker.GetViewProp()
        else:
            return None

    def highlightSegment(self, pt):
        if self.higlight_actor is not None:
            if self.higlight_actor in self.selected_actors:
                self.setSelectedSegmentProperties(self.higlight_actor)
            else:
                self.setSegmentProperties(self.higlight_actor)
            self.higlight_actor = None

        actor = self.pickActor(pt)
        if actor is not None:
            selected = actor in self.selected_actors
            self.setHighlightSegmentProperties(actor, selected)
            self.higlight_actor = actor

    def selectSegment(self, pt):
        actor = self.pickActor(pt)
        if actor is not None:
            if actor in self.selected_actors:
                del self.selected_actors[actor]
                selected = actor in self.selected_actors
                self.setHighlightSegmentProperties(actor, selected)
            else:
                self.selected_actors[actor] = True
                self.setHighlightSegmentProperties(actor, True)

    def onDeselectAll(self):
        if self.dim == 2:
            for actor in self.selected_actors:
                self.setSegmentProperties(actor)
            if self.higlight_actor is not None:
                self.setHighlightSegmentProperties(actor, False)
        else:
            for actor in self.selected_actors:
                self.setSurface3DProperties(actor)
            if self.higlight_actor is not None:
                self.setHighlightFacetProperties(actor, False)
        self.selected_actors = {}

    def highlightFacet(self, pt):
        if self.higlight_actor is not None:
            if self.higlight_actor in self.selected_actors:
                self.setSelectedFacetProperties(self.higlight_actor)
            else:
                self.setSurface3DProperties(self.higlight_actor)
            self.higlight_actor = None

        actor = self.pickActor(pt)
        if actor is not None:
            selected = actor in self.selected_actors
            self.setHighlightFacetProperties(actor, selected)
            self.higlight_actor = actor

    def selectFacet(self, pt):
        actor = self.pickActor(pt)
        if actor is not None:
            if actor in self.selected_actors:
                del self.selected_actors[actor]
                selected = actor in self.selected_actors
                self.setHighlightFacetProperties(actor, selected)
            else:
                self.selected_actors[actor] = True
                self.setHighlightFacetProperties(actor, True)

    def onAssignMarker(self):
        if len(self.selected_actors) == 0:
            return

        dlg = AssignMarkerDlg(self)
        if dlg.exec() == QDialog.Accepted:
            marker = int(dlg.marker.text())
            for actor in self.selected_actors.keys():
                self.facet_marker[actor] = marker

    def setFacetMarkers(self):
        # drill into MeshInfo bypassing the API
        self.info.facet_markers.setup()
        for i, actor in enumerate(self.vtk_segment_actor):
            marker = self.facet_marker[actor]
            self.info.facet_markers[i] = marker

    def createExodusIISideSets(self):
        # TODO: create ExodusII sidesets from self.mesh
        # figure out how to map facet info into a element ID and local element
        # side
        return []

    def isGeometryView(self):
        """Returns True if showing geometry."""
        return self.mesh is None

    def isMeshView(self):
        """Returns True if showing mesh."""
        return self.mesh is not None
