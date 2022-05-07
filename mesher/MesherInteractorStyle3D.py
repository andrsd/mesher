import vtk
from mesher.MesherInteractorInterface import MesherInteractorInterface


class MesherInteractorStyle3D(vtk.vtkInteractorStyleTrackballCamera,
                              MesherInteractorInterface):

    def __init__(self, widget):
        """Inits MesherInteractorStyle3D

        Args:
            widget: Widget that recieves VTK events converted into Qt events
        """
        vtk.vtkInteractorStyleImage.__init__(self)
        MesherInteractorInterface.__init__(self, widget)

    def onLeftButtonPress(self, interactor_style, event):
        super().onLeftButtonPress(interactor_style, event)
        super().OnLeftButtonDown()

    def onLeftButtonRelease(self, interactor_style, event):
        super().onLeftButtonRelease(interactor_style, event)
        super().OnLeftButtonUp()

    def onMouseMove(self, interactor_style, event):
        super().onMouseMove(interactor_style, event)
        super().OnMouseMove()
