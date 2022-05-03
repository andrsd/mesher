from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QCheckBox, QLabel, QLineEdit, QVBoxLayout, \
    QHBoxLayout, QPushButton, QWidget, QSizePolicy
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QDoubleValidator


class MeshingOptionsBaseWidget(QWidget):

    def __init__(self, settings, parent):
        """
        Inits MeshingOptionsBaseWidget

        Args:
            settings: Application QSettings objects to store the options into
            parent: Parent widget
        """
        super().__init__(parent)

        self.settings = settings

        self.setAttribute(Qt.WA_StyledBackground, True)
        self.setObjectName("options")
        self.setStyleSheet("""
            #options, #closeButton {
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
        self.title = QLabel()
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
        self.layout.addWidget(self.mesh_button)
        self.setLayout(self.layout)

    def updateWidgets(self):
        pass

    def connectSignals(self):
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
