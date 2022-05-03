from mesher.MeshingOptionsBaseWidget import MeshingOptionsBaseWidget


class OptionsTetGenWidget(MeshingOptionsBaseWidget):

    data = []

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
