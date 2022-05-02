import vtk
from mesher.MesherInteractorInterface import MesherInteractorInterface


class MesherInteractorStyle2D(vtk.vtkInteractorStyleImage,
                              MesherInteractorInterface):

    def __init__(self, widget):
        """Inits MesherInteractorStyle2D

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
