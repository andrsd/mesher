from PyQt5.QtWidgets import QDialog, QLineEdit, QPushButton, QVBoxLayout, \
    QLabel
from PyQt5.QtGui import QIntValidator
from PyQt5.QtCore import Qt


class AssignMarkerDlg(QDialog):

    def __init__(self, parent):
        """Inits AssignMarkerDlg."""
        super().__init__(parent)

        self.layout = QVBoxLayout()

        self.label = QLabel("Marker:")
        self.layout.addWidget(self.label)

        self.validator = QIntValidator()
        self.validator.setBottom(0)

        self.marker = QLineEdit()
        self.marker.setValidator(self.validator)
        self.layout.addWidget(self.marker)

        self.assign_button = QPushButton("Assign")
        self.assign_button.setDefault(True)
        self.assign_button.clicked.connect(self.accept)
        self.layout.addWidget(self.assign_button)

        self.setLayout(self.layout)
        self.setWindowTitle("Assign Marker")
        self.setWindowFlag(Qt.Tool, True)
