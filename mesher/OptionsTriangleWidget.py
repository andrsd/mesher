from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QScrollArea, QCheckBox, QLabel, QLineEdit, \
    QVBoxLayout, QHBoxLayout, QPushButton, QWidget, QSizePolicy
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QDoubleValidator
from mesher.ClickableLabel import ClickableLabel


class OptionsTriangleWidget(QWidget):

    data = [
        {
            'name': 'assign_regional_attrs',
            'label': 'Assign regional attributes',
            'param': 'attributes',
            'type': 'bool',
            'value': False,
            'help': 'Assigns a regional attribute to each triangle that '
                    'identifies what segment-bounded region it belongs to.'
        },
        {
            'name': 'max_area_constraint',
            'label': 'Maximum area constraint',
            'param': 'max_volume',
            'type': 'float',
            'value': None,
            'help': 'Imposes a maximum triangle area constraint.'
        },
        {
            'name': 'steiner_pts_on_bnd',
            'label': 'Steiner points on the mesh boundary',
            'param': 'allow_boundary_steiner',
            'type': 'bool',
            'value': True,
            'help': 'Allow the insertion of Steiner points on the mesh '
                    'boundary'
        },
        {
            'name': 'second_order_mesh',
            'label': 'Second order mesh',
            'param': 'mesh_order',
            'type': 'bool',
            'value': False,
            'help': 'Generates second-order subparametric elements with six '
                    'nodes each'
        },
        {
            'name': 'quality_meshing',
            'label': 'Quality meshing',
            'param': 'quality_meshing',
            'type': 'bool',
            'value': True,
            'help': 'Quality mesh generation with no angles smaller than 20 '
                    'degrees.'
        },
        {
            'name': 'minimal_angle',
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

        self.setAttribute(Qt.WA_StyledBackground, True)
        self.setObjectName("triangleOptions")
        self.setStyleSheet("""
            #triangleOptions, #closeButton {
                border-radius: 6px;
                background-color: rgb(0, 0, 0);
                color: #fff;
            }
            QLabel, QCheckBox {
                background-color: rgb(0, 0, 0);
                color: #fff;
            }
            """)

        self.setupWidgets()

        effect = QtWidgets.QGraphicsOpacityEffect()
        effect.setOpacity(0.66)
        self.setGraphicsEffect(effect)

        self.setFixedWidth(300)
        self.updateWidgets()
        self.connectSignals()

    def setupWidgets(self):
        self.layout = QVBoxLayout()
        self.layout.addSpacing(8)
        self.layout.setContentsMargins(15, 4, 15, 17)

        title_layout = QHBoxLayout()
        self.title = QLabel("Triangle Options")
        self.title.setStyleSheet("""
            font-weight: bold;
            qproperty-alignment: AlignCenter;
            """)
        self.title.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        title_layout.addWidget(self.title)

        self.close_button = QPushButton("\u2716")
        self.close_button.setObjectName("closeButton")
        self.close_button.setStyleSheet("""
            font-size: 20px;
            """)
        title_layout.addWidget(self.close_button)

        self.layout.addLayout(title_layout)

        for d in self.data:
            if d['type'] == 'bool':
                self.addCheckbox(d)
            elif d['type'] == 'float':
                self.addEditFloat(d)

            self.layout.addSpacing(12)
        self.layout.addStretch()

        self.mesh_button = QPushButton("Mesh")
        self.mesh_button.setStyleSheet("""
            background-color: #eee;
            """)
        # self.mesh_button.setAttribute(Qt.WA_StyledBackground, False)
        self.layout.addWidget(self.mesh_button)
        self.setLayout(self.layout)

    def updateWidgets(self):
        quality_meshing = self.quality_meshing.checkState() == Qt.Checked
        self.minimal_angle.setEnabled(quality_meshing)
        self.minimal_angle_label.setEnabled(quality_meshing)

    def connectSignals(self):
        self.quality_meshing.stateChanged.connect(self.updateWidgets)
        self.close_button.clicked.connect(self.hide)

    def addCheckbox(self, item):
        checkbox = QCheckBox(item['label'], self)
        if item['value']:
            checkbox.setCheckState(Qt.Checked)
        else:
            checkbox.setCheckState(Qt.Unchecked)
        self.layout.addWidget(checkbox)
        setattr(self, item['name'], checkbox)

        if len(item['help']) > 0:
            self.addHelpWidget(item, 18)

        return checkbox

    def addEditFloat(self, item):
        layout = QHBoxLayout()
        label = QLabel(item['label'])
        layout.addWidget(label)
        setattr(self, item['name'] + "_label", label)
        lineedit = QLineEdit()
        lineedit.setValidator(QDoubleValidator())
        layout.addWidget(lineedit)
        setattr(self, item['name'], lineedit)
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

    def getParams(self):
        params = {}
        for d in self.data:
            widget = getattr(self, d['name'])
            param_name = d['param']
            if isinstance(widget, QtWidgets.QCheckBox):
                params[param_name] = widget.checkState() == Qt.Checked
            elif isinstance(widget, QtWidgets.QLineEdit):
                text = widget.text()
                if len(text) == 0:
                    params[param_name] = None
                else:
                    params[param_name] = float(text)
        return params
