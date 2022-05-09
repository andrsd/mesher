import pytest
from PyQt5 import QtWidgets
from mesher import consts


@pytest.fixture
def about_dlg(qtbot, main_window):
    from mesher.AboutDialog import AboutDialog
    dlg = AboutDialog(main_window)
    qtbot.addWidget(dlg)
    yield dlg


def test_about_dialog(about_dlg):
    assert isinstance(about_dlg.icon, QtWidgets.QLabel)
    assert isinstance(about_dlg.title, QtWidgets.QLabel)
    assert about_dlg.title.text() == consts.APP_NAME
    assert isinstance(about_dlg.layout, QtWidgets.QVBoxLayout)
