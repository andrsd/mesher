import pytest


@pytest.fixture
def main_window(qtbot):
    """
    Returns an instance of MainWindow
    """
    from mesher.MainWindow import MainWindow
    main = MainWindow()
    qtbot.addWidget(main)
    yield main
