from PyQt5 import QtWidgets, QtGui
from mesher.AssignMarkerDialog import AssignMarkerDialog


def test_about_dialog(main_window):
    dlg = AssignMarkerDialog(main_window)
    assert isinstance(dlg.label, QtWidgets.QLabel)
    assert isinstance(dlg.assign_button, QtWidgets.QPushButton)
    assert isinstance(dlg.marker, QtWidgets.QLineEdit)
    assert isinstance(dlg.validator, QtGui.QIntValidator)
    assert dlg.windowTitle() == "Assign Marker"
    assert isinstance(dlg.layout, QtWidgets.QVBoxLayout)
    assert dlg.assign_button.isDefault() is True
