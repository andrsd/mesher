from PyQt5.QtCore import Qt, QPoint, QEvent, QCoreApplication
from PyQt5.QtGui import QKeySequence, QKeyEvent


class MesherInteractorInterface:

    def __init__(self, widget):
        """Inits MesherInteractorInterface

        Args:
            widget: Widget that recieves VTK events converted into Qt events
        """
        self.widget = widget
        self.last_mouse_pos = None
        self.left_button_down = False

        self.AddObserver("LeftButtonPressEvent", self.onLeftButtonPress)
        self.AddObserver("LeftButtonReleaseEvent", self.onLeftButtonRelease)
        # self.AddObserver("KeyReleaseEvent", self.onKeyReleased, 1000)
        self.AddObserver("KeyPressEvent", self.onKeyPress)
        self.AddObserver("KeyReleaseEvent", self.onKeyRelease)
        self.AddObserver("CharEvent", self.onChar)

    def onLeftButtonPress(self, interactor_style, event):
        self.left_button_down = True
        interactor = interactor_style.GetInteractor()
        click_pos = interactor.GetEventPosition()
        pt = QPoint(click_pos[0], click_pos[1])
        self.last_mouse_pos = pt

    def onLeftButtonRelease(self, interactor_style, event):
        self.left_button_down = False
        interactor = interactor_style.GetInteractor()
        click_pos = interactor.GetEventPosition()
        pt = QPoint(click_pos[0], click_pos[1])
        if self.last_mouse_pos == pt:
            self.widget.onClicked(pt)

    def onKeyPress(self, interactor_style, event):
        interactor = interactor_style.GetInteractor()
        key = interactor.GetKeyCode()
        seq = QKeySequence(key)
        # FIXME: get the modifiers from interactor
        mods = Qt.NoModifier
        if len(seq) > 0:
            e = QKeyEvent(QEvent.KeyPress, seq[0], mods)
            QCoreApplication.postEvent(self.widget, e)

    def onKeyRelease(self, interactor_style, event):
        # interactor = interactor_style.GetInteractor()
        pass

    def onChar(self, interactor_style, event):
        # interactor = interactor_style.GetInteractor()
        pass
