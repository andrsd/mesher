import os
from unittest.mock import MagicMock, patch
import PyQt5
from PyQt5.QtCore import QEvent, QUrl
import vtk
import meshpy


@patch('mesher.MainWindow.MainWindow.clear')
def test_on_new_file(clr, main_window):
    main_window.onNewFile()
    clr.assert_called_once()
    assert main_window.file_name is None


def test_open_file_poly_2d(main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'quad.poly')
    main_window.openFile(file_name)


def test_open_file_poly_3d(main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'cube.poly')
    main_window.openFile(file_name)


@patch('mesher.MainWindow.MainWindow.showNotification')
def test_open_file_unsupported_file(sh_notf, main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'unsupp.yml')
    main_window.openFile(file_name)
    sh_notf.assert_called_with('Unsupported file format')


def test_on_about(main_window):
    main_window.about_dlg = MagicMock()
    main_window.onAbout()
    main_window.about_dlg.show.assert_called_once()


@patch('mesher.MainWindow.MainWindow.openFile')
@patch('PyQt5.QtWidgets.QFileDialog.getOpenFileName')
def test_file_open_with_no_filename(ofd, opn, main_window):
    ofd.return_value = ("asdf.poly", None)
    main_window.onOpenFile()
    ofd.assert_called_once()
    opn.assert_called_with("asdf.poly")


@patch('mesher.MainWindow.MainWindow.showMinimized')
def test_on_minimize(sh, main_window):
    main_window.onMinimize()
    sh.assert_called_once()


@patch('mesher.MainWindow.MainWindow.showNormal')
def test_bring_all_to_front(sh, main_window):
    main_window.onBringAllToFront()
    sh.assert_called_once()


@patch('mesher.MainWindow.MainWindow.activateWindow')
@patch('mesher.MainWindow.MainWindow.showNormal')
def test_on_show_main_window(shn, act, main_window):
    main_window.onShowMainWindow()
    shn.assert_called_once()
    act.assert_called_once()


@patch('mesher.MainWindow.MainWindow.updateMenuBar')
def test_event(upt, main_window):
    event = QEvent(QEvent.WindowActivate)
    main_window.event(event)
    upt.assert_called_once()


@patch('PyQt5.QtWidgets.QMainWindow.resizeEvent')
@patch('mesher.MainWindow.MainWindow.updateWidgets')
def test_resize_event(upt, rsz, main_window):
    event = MagicMock()
    main_window.resizeEvent(event)
    upt.assert_called_once()


@patch('mesher.MainWindow.MainWindow.openFile')
@patch('mesher.MainWindow.MainWindow.sender')
def test_on_open_recent_file(snd, opn, main_window):
    action = MagicMock()
    action.data.return_value = 'asdf.poly'
    snd.return_value = action
    main_window.onOpenRecentFile()
    opn.assert_called_with('asdf.poly')


@patch('mesher.MainWindow.MainWindow.buildRecentFilesMenu')
def test_on_clear_recent_files(brfm, main_window):
    main_window.onClearRecentFiles()
    assert len(main_window.recent_files) == 0
    brfm.assert_called_once()


@patch('mesher.MainWindow.MainWindow.isGeometryView')
@patch('mesher.MainWindow.MainWindow.selectSegment')
def test_on_clicked_2d(ss, igv, main_window):
    igv.return_value = True
    main_window.dim = 2
    pt = MagicMock()
    main_window.onClicked(pt)
    ss.assert_called_with(pt)


@patch('mesher.MainWindow.MainWindow.isGeometryView')
@patch('mesher.MainWindow.MainWindow.selectFacet')
def test_on_clicked_3d(sf, igv, main_window):
    igv.return_value = True
    main_window.dim = 3
    pt = MagicMock()
    main_window.onClicked(pt)
    sf.assert_called_with(pt)


@patch('mesher.MainWindow.MainWindow.setSurface3DProperties')
@patch('mesher.MainWindow.MainWindow.addToVtk')
def test_surface_to_vtk(a2vtk, set_prop, main_window):
    act = MagicMock()
    a2vtk.return_value = act
    surface = MagicMock()
    main_window.surfaceToVtk(surface)
    assert main_window.vtk_vertex_actor is None
    assert main_window.vtk_segment_actor == act
    set_prop.assert_called_with(act)


def test_mesh_to_vtk_none(main_window):
    mesh = None
    assert main_window.meshToVtk(mesh) is None


@patch('mesher.MainWindow.MainWindow.setMeshProperties')
@patch('mesher.MainWindow.MainWindow.addToVtk')
def test_mesh_to_vtk_obj(a2vtk, set_prop, main_window):
    act = MagicMock()
    a2vtk.return_value = act
    mesh = MagicMock()
    assert main_window.meshToVtk(mesh) == act
    set_prop.assert_called_with(act)


def tests_segments_2d_to_unstructured_grid(main_window):
    info = MagicMock()
    grid = main_window.segments2DToUnstructuredGrid(info)
    assert isinstance(grid, vtk.vtkUnstructuredGrid)


def tests_triangle_2d_to_unstructured_grid(main_window):
    info = MagicMock()
    grid = main_window.triangles2DToUnstructuredGrid(info)
    assert isinstance(grid, vtk.vtkUnstructuredGrid)


def test_triangles_3d_to_unstructured_grid(main_window):
    mesh = MagicMock()
    setattr(mesh, 'cells_dict', {'triangle': [[0, 1, 2], [1, 2, 3]]})
    grid = main_window.triangles3DToUnstructuredGrid(mesh)
    assert isinstance(grid, vtk.vtkUnstructuredGrid)


def test_tetras_to_unstructured_grid(main_window):
    mesh = MagicMock()
    setattr(mesh, 'cells_dict', {'triangle': [[0, 1, 2], [1, 2, 3]]})
    grid = main_window.tetrasToUnstructuredGrid(mesh)
    assert isinstance(grid, vtk.vtkUnstructuredGrid)


def test_set_vertex_properties(main_window):
    act = MagicMock()
    main_window.setVertexProperties(act)


def test_set_mesh_properties(main_window):
    act = MagicMock()
    main_window.setMeshProperties(act)


def test_set_hilight_segment_properties(main_window):
    act = MagicMock()
    main_window.setHighlightSegmentProperties(act, True)
    main_window.setHighlightSegmentProperties(act, False)


def test_set_selected_segment_properties(main_window):
    act = MagicMock()
    main_window.setSelectedSegmentProperties(act)


def test_set_hilight_facet_properties(main_window):
    act = MagicMock()
    main_window.setHighlightFacetProperties(act, True)
    main_window.setHighlightFacetProperties(act, False)


def test_set_selected_facet_properties(main_window):
    act = MagicMock()
    main_window.setSelectedFacetProperties(act)


@patch('mesher.MainWindow.MainWindow.showMeshingOptions')
def test_on_mesh(smo, main_window):
    main_window.showMeshingOptions()
    smo.assert_called_once()


@patch('mesher.MainWindow.MainWindow.updateMenuBar')
def test_on_mesh_clicked_none(umb, main_window):
    main_window.onMeshClicked()
    umb.assert_called_once()


@patch('meshpy.triangle.build')
@patch('mesher.MainWindow.MainWindow.setFacetMarkers')
@patch('mesher.MainWindow.MainWindow.updateMenuBar')
def test_on_mesh_clicked_triangle(umb, sfm, bld, main_window):
    main_window.info = meshpy.triangle.MeshInfo()
    main_window.onMeshClicked()
    umb.assert_called_once()
    sfm.assert_called_once()
    bld.assert_called_once()


@patch('meshpy.tet.build')
@patch('mesher.MainWindow.MainWindow.setFacetMarkers')
@patch('mesher.MainWindow.MainWindow.updateMenuBar')
def test_on_mesh_clicked_tet(umb, sfm, bld, main_window):
    main_window.info = meshpy.tet.MeshInfo()
    main_window.onMeshClicked()
    umb.assert_called_once()
    sfm.assert_called_once()
    bld.assert_called_once()


@patch('PyQt5.QtWidgets.QFileDialog.exec_')
@patch('PyQt5.QtWidgets.QFileDialog.selectedFiles')
def test_getFileName(sf, exc, main_window):
    sf.return_value = ['file.ext']
    exc.return_value = PyQt5.QtWidgets.QDialog.Accepted
    fn = main_window.getFileName("title", "filter", "suff")
    assert fn == 'file.ext'


@patch('mesher.MainWindow.MainWindow.showNotification')
@patch('mesher.exodusII.write')
@patch('mesher.MainWindow.MainWindow.createExodusIISideSets')
@patch('mesher.MainWindow.MainWindow.getFileName')
def test_on_export_as_exodusII_2d(gfn, ce2, wrt, sn, main_window):
    gfn.return_value = 'file.exo'
    main_window.dim = 2
    main_window.mesh = MagicMock()
    main_window.onExportAsExodusII()
    ce2.assert_called_once()
    wrt.assert_called_once()
    sn.assert_called_with("File 'file.exo' exported sucessfully")


@patch('mesher.MainWindow.MainWindow.showNotification')
@patch('mesher.exodusII.write')
@patch('mesher.MainWindow.MainWindow.createExodusIISideSets')
@patch('mesher.MainWindow.MainWindow.getFileName')
def test_on_export_as_exodusII_3d(gfn, ce2, wrt, sn, main_window):
    gfn.return_value = 'file.exo'
    main_window.dim = 3
    main_window.mesh = MagicMock()
    main_window.onExportAsExodusII()
    ce2.assert_called_once()
    wrt.assert_called_once()
    sn.assert_called_with("File 'file.exo' exported sucessfully")


def test_drag_enter_event(main_window):
    event = MagicMock()
    event.mimeData.return_value.hasUrls.return_value = False
    main_window.dragEnterEvent(event)
    event.ignore.assert_called_once()

    event = MagicMock()
    event.mimeData.return_value.hasUrls.return_value = True
    main_window.dragEnterEvent(event)
    event.accept.assert_called_once()


def test_drop_event_ignore(main_window):
    event = MagicMock()
    event.mimeData.return_value.hasUrls.return_value = False
    main_window.dropEvent(event)
    event.ignore.assert_called_once()


@patch('mesher.MainWindow.MainWindow.openFile')
def test_drop_event_accept(opf, main_window):
    event = MagicMock()
    event.mimeData.return_value.hasUrls.return_value = True
    event.mimeData.return_value.urls.return_value = [QUrl('file:///file.ext')]
    main_window.dropEvent(event)
    event.accept.assert_called_once()
    opf.assert_called_with('/file.ext')


@patch('mesher.NotificationWidget.NotificationWidget.setGeometry')
@patch('mesher.NotificationWidget.NotificationWidget.show')
def test_show_notification(sh, geom, main_window):
    main_window.showNotification("notif")
    assert main_window.notification.text.text() == "notif"
    sh.assert_called_with(2000)
    geom.assert_called_once()


def test_get_meshing_options_dialog(main_window):
    main_window.dim = 2
    assert main_window.getMeshingOptionsDialog() == main_window.opts_tri_dlg

    main_window.dim = 3
    assert main_window.getMeshingOptionsDialog() == main_window.opts_tet_dlg

    main_window.dim = 4
    assert main_window.getMeshingOptionsDialog() is None


def test_showMeshingOptions(main_window):
    main_window.showMeshingOptions()


@patch('mesher.MainWindow.MainWindow.updateMeshingOptionsGeometry')
@patch('mesher.MainWindow.MainWindow.getMeshingOptionsDialog')
def test_show_meshing_options(mopts, updt, main_window):
    dlg = MagicMock()
    mopts.return_value = dlg
    main_window.showMeshingOptions()
    updt.assert_called_with(dlg)
    dlg.show.assert_called_once()


def test_update_meshing_options_geometry(main_window):
    dlg = MagicMock()
    main_window.updateMeshingOptionsGeometry(dlg)
    dlg.adjustSize.assert_called_once()
    dlg.setGeometry.assert_called_once()


@patch('mesher.MainWindow.MainWindow.getMeshingOptionsDialog')
def test_on_hide_meshing_options(mopts, main_window):
    dlg = MagicMock()
    mopts.return_value = dlg
    main_window.onHideMeshingOptions()
    dlg.hide.assert_called_once()


@patch('mesher.MainWindow.MainWindow.highlightSegment')
@patch('mesher.MainWindow.MainWindow.isGeometryView')
def test_on_mouse_mode_2d(igv, hs, main_window):
    pt = MagicMock()
    igv.return_value = True
    main_window.dim = 2
    main_window.onMouseMove(pt)
    hs.assert_called_once()


@patch('mesher.MainWindow.MainWindow.highlightFacet')
@patch('mesher.MainWindow.MainWindow.isGeometryView')
def test_on_mouse_mode_3d(igv, hf, main_window):
    pt = MagicMock()
    igv.return_value = True
    main_window.dim = 3
    main_window.onMouseMove(pt)
    hf.assert_called_once()


def test_pick_actor(main_window):
    # better test is possible if we build the whole VTK pipeline
    pt = MagicMock()
    main_window.pickActor(pt)


@patch('mesher.MainWindow.MainWindow.setHighlightSegmentProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_highlight_segment(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.highlightSegment(pt)
    pa.assert_called_with(pt)
    assert main_window.higlight_actor == act
    hsp.assert_called_with(act, False)


@patch('mesher.MainWindow.MainWindow.setHighlightSegmentProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_select_segment_nosel(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.selectSegment(pt)
    assert act in main_window.selected_actors
    pa.assert_called_with(pt)
    hsp.assert_called_with(act, True)


@patch('mesher.MainWindow.MainWindow.setHighlightSegmentProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_select_segment_sel(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.selected_actors[act] = True
    main_window.selectSegment(pt)
    assert not(act in main_window.selected_actors)
    pa.assert_called_with(pt)
    hsp.assert_called_with(act, False)


@patch('mesher.MainWindow.MainWindow.setHighlightSegmentProperties')
@patch('mesher.MainWindow.MainWindow.setSegmentProperties')
def test_on_deselect_all_2d(ssp, shsp, main_window):
    main_window.dim = 2
    act = MagicMock()
    main_window.selected_actors = [act]
    main_window.higlight_actor = MagicMock()
    main_window.onDeselectAll()
    assert len(main_window.selected_actors.keys()) == 0
    ssp.assert_called_with(act)
    shsp.assert_called_with(act, False)


@patch('mesher.MainWindow.MainWindow.setHighlightFacetProperties')
@patch('mesher.MainWindow.MainWindow.setSurface3DProperties')
def test_on_deselect_all_3d(ssp, shsp, main_window):
    main_window.dim = 3
    act = MagicMock()
    main_window.selected_actors = [act]
    main_window.higlight_actor = MagicMock()
    main_window.onDeselectAll()
    assert len(main_window.selected_actors.keys()) == 0
    ssp.assert_called_with(act)
    shsp.assert_called_with(act, False)


@patch('mesher.MainWindow.MainWindow.setHighlightFacetProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_highlight_facet(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.highlightFacet(pt)
    pa.assert_called_with(pt)
    assert main_window.higlight_actor == act
    hsp.assert_called_with(act, False)


@patch('mesher.MainWindow.MainWindow.setHighlightFacetProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_select_facet_nosel(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.selectFacet(pt)
    assert act in main_window.selected_actors
    pa.assert_called_with(pt)
    hsp.assert_called_with(act, True)


@patch('mesher.MainWindow.MainWindow.setHighlightFacetProperties')
@patch('mesher.MainWindow.MainWindow.pickActor')
def test_select_facet_sel(pa, hsp, main_window):
    main_window.higlight_actor = MagicMock()
    act = MagicMock()
    pa.return_value = act
    pt = MagicMock()
    main_window.selected_actors[act] = True
    main_window.selectFacet(pt)
    assert not(act in main_window.selected_actors)
    pa.assert_called_with(pt)
    hsp.assert_called_with(act, False)


@patch('PyQt5.QtWidgets.QLineEdit.text')
@patch('mesher.AssignMarkerDialog.AssignMarkerDialog.exec')
def test_on_assign_marker(exc, txt, main_window):
    # mocking `QLineEdit.text` works since there is only single line edit
    # in AssignMarkerDialog - but I don't like it
    act = MagicMock()
    main_window.selected_actors[act] = True
    exc.return_value = PyQt5.QtWidgets.QDialog.Accepted
    txt.return_value = '2'
    main_window.onAssignMarker()
    assert main_window.facet_marker[act] == 2


def test_set_facet_markers(main_window):
    main_window.info = MagicMock()
    act = MagicMock()
    main_window.vtk_segment_actor = [act]
    main_window.facet_marker[act] = 2
    main_window.setFacetMarkers()
    main_window.info.facet_markers.__setitem__.assert_called_with(0, 2)


def test_create_exodusII_side_sets(main_window):
    assert main_window.createExodusIISideSets() == []


def test_is_geometry_view(main_window):
    main_window.mesh = MagicMock()
    assert main_window.isGeometryView() is False

    main_window.mesh = None
    assert main_window.isGeometryView() is True


def test_is_mesh_view(main_window):
    main_window.mesh = MagicMock()
    assert main_window.isMeshView() is True

    main_window.mesh = None
    assert main_window.isMeshView() is False
