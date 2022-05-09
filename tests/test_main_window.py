import os
from unittest.mock import MagicMock, patch


@patch('mesher.MainWindow.MainWindow.clear')
def test_on_new_file(clr, main_window):
    main_window.onNewFile()
    clr.assert_called_once()
    assert main_window.file_name is None


def test_open_file_poly_2d(main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'quad.poly')
    main_window.openFile(file_name)


def test_open_file_poly_3d(main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'cube.poly')
    main_window.openFile(file_name)


@patch('mesher.MainWindow.MainWindow.showNotification')
def test_open_file_unsupported_file(sh_notf, main_window):
    root = os.path.dirname(__file__)
    file_name = os.path.join(root, 'assets', 'unsupp.yml')
    main_window.openFile(file_name)
    sh_notf.assert_called_with('Unsupported file format')


# @patch('PyQt5.QtWidgets.QFileDialog.getOpenFileName')
# def test_on_open_file(ofd, main_window):
#     main_window.onOpenFile()
#     ofd.assert_called_once()


@patch('mesher.MainWindow.MainWindow.showMinimized')
def test_on_minimize(sh, main_window):
    main_window.onMinimize()
    sh.assert_called_once()


@patch('mesher.MainWindow.MainWindow.showNormal')
def test_bring_all_to_front(sh, main_window):
    main_window.onBringAllToFront()
    sh.assert_called_once()


@patch('mesher.MainWindow.MainWindow.activateWindow')
@patch('mesher.MainWindow.MainWindow.showNormal')
def test_on_show_main_window(shn, act, main_window):
    main_window.onShowMainWindow()
    shn.assert_called_once()
    act.assert_called_once()


@patch('PyQt5.QtWidgets.QMainWindow.resizeEvent')
@patch('mesher.MainWindow.MainWindow.updateWidgets')
def test_resize_event(upt, rsz, main_window):
    event = MagicMock()
    main_window.resizeEvent(event)
    upt.assert_called_once()


# @patch('mesher.MainWindow.MainWindow.openFile')
# def test_on_open_recent_file(opn, main_window):
#     main_window.onOpenRecentFile()
#     opn.assert_called_once()


@patch('mesher.MainWindow.MainWindow.buildRecentFilesMenu')
def test_on_clear_recent_files(brfm, main_window):
    main_window.onClearRecentFiles()
    assert len(main_window.recent_files) == 0
    brfm.assert_called_once()
