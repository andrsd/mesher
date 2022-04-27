import vtk
from mesher.MesherInteractorInterface import MesherInteractorInterface


class MesherInteractorStyle3D(vtk.vtkInteractorStyleTrackballCamera,
                              MesherInteractorInterface):

    def __init__(self, widget):
        vtk.vtkInteractorStyleImage.__init__(self)
        MesherInteractorInterface.__init__(self, widget)

    def onLeftButtonPress(self, interactor_style, event):
        super().onLeftButtonPress(interactor_style, event)
        super().OnLeftButtonDown()

    def onLeftButtonRelease(self, interactor_style, event):
        super().onLeftButtonRelease(interactor_style, event)
        super().OnLeftButtonUp()