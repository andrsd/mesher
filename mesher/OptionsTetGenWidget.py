from mesher.MeshingOptionsBaseWidget import MeshingOptionsBaseWidget


class OptionsTetGenWidget(MeshingOptionsBaseWidget):

    data = [
        {
            'name': 'assign_attrs',
            'label': 'Assign attributes',
            'param': 'attributes',
            'type': 'bool',
            'value': False,
            'help': 'Assigns attributes to identify tetrahedra in certain '
                    'regions.'
        },
        {
            'name': 'max_volume',
            'label': 'Maximum volume constraint',
            'param': 'max_volume',
            'type': 'float',
            'value': None,
            'help': 'Applies a maximum tetrahedron volume constraint.'
        },
        {
            'name': 'insert_points',
            'label': 'Inserts additional points into mesh',
            'param': 'insert_points',
            'type': 'bool',
            'value': False,
            'help': 'Inserts a list of additional points into mesh.'
        }
    ]

    def __init__(self, settings, parent):
        """Inits OptionsTetGenWidget

        Args:
            settings: Application QSettings objects to store the options into
            parent: Parent widget
        """
        super().__init__(settings, parent)
        self.title.setText("TetGen Options")

    def updateWidgets(self):
        pass

    def connectSignals(self):
        super().connectSignals()
