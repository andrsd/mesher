from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QLabel


class ClickableLabel(QLabel):

    clicked = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)

    def enterEvent(self, event):
        f = self.font()
        f.setUnderline(True)
        self.setFont(f)

    def leaveEvent(self, event):
        f = self.font()
        f.setUnderline(False)
        self.setFont(f)

    def mouseReleaseEvent(self, event):
        self.clicked.emit()
