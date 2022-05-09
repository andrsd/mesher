import pytest
from unittest.mock import MagicMock


@pytest.fixture
def clickable_label(qtbot, main_window):
    from mesher.ClickableLabel import ClickableLabel
    widget = ClickableLabel(main_window)
    qtbot.addWidget(widget)
    yield widget


def test_enter_event(clickable_label):
    e = MagicMock()
    clickable_label.enterEvent(e)
    assert clickable_label.font().underline() is True


def test_leave_event(clickable_label):
    e = MagicMock()
    clickable_label.leaveEvent(e)
    assert clickable_label.font().underline() is False


def test_mouse_release(clickable_label):
    emit = MagicMock()
    emit.clicked.return_value = None
    e = MagicMock()
    clickable_label.clicked.connect(emit.clicked)
    clickable_label.mouseReleaseEvent(e)
    emit.clicked.assert_called_once()
