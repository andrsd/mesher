from PyQt5.QtCore import Qt
from mesher.MeshingOptionsBaseWidget import MeshingOptionsBaseWidget


class OptionsTriangleWidget(MeshingOptionsBaseWidget):

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
        super().__init__(settings, parent)
        self.title.setText("Triangle Options")

    def updateWidgets(self):
        quality_meshing = self.quality_meshing.checkState() == Qt.Checked
        self.minimal_angle.setEnabled(quality_meshing)
        self.minimal_angle_label.setEnabled(quality_meshing)

    def connectSignals(self):
        super().connectSignals()
        self.quality_meshing.stateChanged.connect(self.updateWidgets)
