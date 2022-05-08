from mesher import vtk_helpers
import vtk
from PyQt5 import QtGui
from unittest.mock import MagicMock


def test_build_point_array_2d():
    points = [
        [0, 0],
        [1, 0],
        [1, 1]
    ]
    pt_arr = vtk_helpers.buildPointArray2D(points)
    assert isinstance(pt_arr, vtk.vtkPoints)
    assert pt_arr.GetNumberOfPoints() == 3
    # TODO: check coordinates


def test_build_point_array_3d():
    points = [
        [0, 0, 0],
        [1, 0, 0],
        [1, 1, 0.5]
    ]
    pt_arr = vtk_helpers.buildPointArray3D(points)
    assert isinstance(pt_arr, vtk.vtkPoints)
    assert pt_arr.GetNumberOfPoints() == 3
    # TODO: check coordinates


def test_build_cell_array_vertex():
    points = [
        [0, 0, 0],
        [1, 0, 0],
        [1, 1, 0.5]
    ]
    cell_arr = vtk_helpers.buildCellArrayVertex(points)
    assert isinstance(cell_arr, vtk.vtkCellArray)
    assert cell_arr.GetNumberOfCells() == 3


def test_build_cell_array_line():
    points = [
        [0, 1],
        [1, 2]
    ]
    cell_arr = vtk_helpers.buildCellArrayLine(points)
    assert isinstance(cell_arr, vtk.vtkCellArray)
    assert cell_arr.GetNumberOfCells() == 2


def test_build_cell_array_triangle():
    points = [
        [0, 1, 2],
        [1, 2, 3]
    ]
    cell_arr = vtk_helpers.buildCellArrayTriangle(points)
    assert isinstance(cell_arr, vtk.vtkCellArray)
    assert cell_arr.GetNumberOfCells() == 2


def test_build_cell_array_tetra():
    points = [
        [0, 1, 2, 3],
        [1, 2, 3, 4]
    ]
    cell_arr = vtk_helpers.buildCellArrayTetra(points)
    assert isinstance(cell_arr, vtk.vtkCellArray)
    assert cell_arr.GetNumberOfCells() == 2


def test_build_cell_array_polygon():
    facets = [MagicMock()]
    facets[0].polygons = [MagicMock()]
    facets[0].polygons[0].vertices = [1, 2, 3]
    cell_arr = vtk_helpers.buildCellArrayPolygon(facets)
    assert isinstance(cell_arr, vtk.vtkCellArray)
    assert cell_arr.GetNumberOfCells() == 1


def test_vertices_2d_to_unstructured_grid():
    points = [
        [0, 0],
        [1, 0]
    ]
    ugrid = vtk_helpers.vertices2DToUnstructuredGrid(points)
    assert isinstance(ugrid, vtk.vtkUnstructuredGrid)
    assert ugrid.GetNumberOfPoints() == 2
    assert ugrid.GetNumberOfCells() == 2


def test_vertices_3d_to_unstructured_grid():
    points = [
        [0, 0, 0],
        [1, 0, 0]
    ]
    ugrid = vtk_helpers.vertices3DToUnstructuredGrid(points)
    assert isinstance(ugrid, vtk.vtkUnstructuredGrid)
    assert ugrid.GetNumberOfPoints() == 2
    assert ugrid.GetNumberOfCells() == 2


def test_qcolor2vtk():
    qclr = QtGui.QColor(51, 102, 153)
    c = vtk_helpers.qcolor2vtk(qclr)
    assert c[0] == 0.2
    assert c[1] == 0.4
    assert c[2] == 0.6


def test_rgb2vtk():
    c = vtk_helpers.rgb2vtk([51, 102, 153])
    assert c[0] == 0.2
    assert c[1] == 0.4
    assert c[2] == 0.6
