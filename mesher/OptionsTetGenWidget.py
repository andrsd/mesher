from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QScrollArea, QVBoxLayout, QPushButton


class OptionsTetGenWidget(QScrollArea):

    data = []

    def __init__(self, settings, parent):
        """Inits OptionsTetGenWidget

        Args:
            settings: Application QSettings objects to store the options into
            parent: Parent widget
        """
        super().__init__(parent)

        self.settings = settings

        self.layout = QVBoxLayout()
        self.layout.addSpacing(8)

        self.setupWidgets()

        w = QtWidgets.QWidget()
        w.setLayout(self.layout)
        self.setWidget(w)
        self.setWindowTitle("TetGen Options")
        self.setFixedWidth(300)
        self.setWidgetResizable(True)
        self.setWindowFlag(QtCore.Qt.Tool)

        geom = self.settings.value("tet_opts/geometry")
        default_size = QtCore.QSize(300, 700)
        if geom is None:
            self.resize(default_size)
        else:
            if not self.restoreGeometry(geom):
                self.resize(default_size)

    def setupWidgets(self):
        self.layout.addStretch()

        self.mesh_button = QPushButton("Mesh")
        self.layout.addWidget(self.mesh_button)

    def closeEvent(self, event):
        self.settings.setValue("tet_opts/geometry", self.saveGeometry())
        event.accept()
