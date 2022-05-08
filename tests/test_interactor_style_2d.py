import vtk
from unittest.mock import MagicMock
from mesher.MesherInteractorStyle2D import MesherInteractorStyle2D


def test_interactor_style_2d():
    widget = MagicMock()
    event = MagicMock()
    interactor_style = MesherInteractorStyle2D(widget)

    renderer = vtk.vtkRenderer()

    render_window = vtk.vtkRenderWindow()
    render_window.AddRenderer(renderer)

    render_window_interactor = vtk.vtkRenderWindowInteractor()
    render_window_interactor.SetRenderWindow(render_window)
    render_window_interactor.SetInteractorStyle(interactor_style)

    interactor_style.onLeftButtonPress(interactor_style, event)

    interactor_style.onLeftButtonRelease(interactor_style, event)

    interactor_style.onMouseMove(interactor_style, event)

    interactor_style.onKeyPress(interactor_style, event)

    interactor_style.onKeyRelease(interactor_style, event)

    interactor_style.onChar(interactor_style, event)
