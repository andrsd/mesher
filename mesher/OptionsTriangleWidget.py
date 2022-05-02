from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QScrollArea, QCheckBox, QLabel, QLineEdit, \
    QVBoxLayout, QHBoxLayout, QPushButton
from PyQt5.QtCore import Qt


class OptionsTriangleWidget(QScrollArea):

    data = [
        {
            'label': 'Assign regional attributes',
            'param': 'attributes',
            'type': 'bool',
            'value': False,
            'help': 'Assigns a regional attribute to each triangle that '
                    'identifies what segment-bounded region it belongs to.'
        },
        {
            'label': 'Maximum area constraint',
            'param': 'max_volume',
            'type': 'float',
            'value': None,
            'help': 'Imposes a maximum triangle area constraint.'
        },
        {
            'label': 'Steiner points on the mesh boundary',
            'param': 'allow_boundary_steiner',
            'type': 'bool',
            'value': True,
            'help': 'Allow the insertion of Steiner points on the mesh '
                    'boundary'
        },
        {
            'label': 'Second order mesh',
            'param': 'mesh_order',
            'type': 'bool',
            'value': False,
            'help': 'Generates second-order subparametric elements with six '
                    'nodes each'
        },
        {
            'label': 'Quality meshing',
            'param': 'quality_meshing',
            'type': 'bool',
            'value': True,
            'help': 'Quality mesh generation with no angles smaller than 20 '
                    'degrees.'
        },
        {
            'label': 'Minimal angle',
            'param': 'min_angle',
            'type': 'float',
            'value': 20.,
            'help': ''
        }
    ]

    def __init__(self, settings, parent):
        """Inits OptionsTriangleWidget

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
        self.setWindowTitle("Triangle Options")
        self.setFixedWidth(300)
        self.setWidgetResizable(True)
        self.setWindowFlag(QtCore.Qt.Tool)

        geom = self.settings.value("tri_opts/geometry")
        default_size = QtCore.QSize(300, 700)
        if geom is None:
            self.resize(default_size)
        else:
            if not self.restoreGeometry(geom):
                self.resize(default_size)

    def setupWidgets(self):
        for d in self.data:
            if d['type'] == 'bool':
                self.addCheckbox(d)
            elif d['type'] == 'float':
                self.addEditFloat(d)

            self.layout.addSpacing(12)
        self.layout.addStretch()

        self.mesh_button = QPushButton("Mesh")
        self.layout.addWidget(self.mesh_button)

    def closeEvent(self, event):
        self.settings.setValue("tri_opts/geometry", self.saveGeometry())
        event.accept()

    def addCheckbox(self, item):
        checkbox = QCheckBox(item['label'])
        if item['value']:
            checkbox.setCheckState(Qt.Checked)
        else:
            checkbox.setCheckState(Qt.Unchecked)
        self.layout.addWidget(checkbox)

        if len(item['help']) > 0:
            self.addHelpWidget(item, 18)

        return checkbox

    def addEditFloat(self, item):
        layout = QHBoxLayout()
        label = QLabel(item['label'])
        layout.addWidget(label)
        lineedit = QLineEdit()
        layout.addWidget(lineedit)
        self.layout.addLayout(layout)

        if len(item['help']) > 0:
            self.addHelpWidget(item, 0)

        return lineedit

    def addHelpWidget(self, item, indent):
        help_label = QLabel(item['help'])
        font = help_label.font()
        font.setPointSizeF(font.pointSizeF() * 0.9)
        help_label.setFont(font)
        help_label.setWordWrap(True)
        layout = QHBoxLayout()
        layout.addWidget(help_label)
        layout.setContentsMargins(indent, 0, 0, 0)
        self.layout.addLayout(layout)
