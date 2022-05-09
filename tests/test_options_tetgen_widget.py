import pytest
from unittest.mock import MagicMock


@pytest.fixture
def meshing_options_widget(qtbot, main_window):
    from mesher.OptionsTetGenWidget import OptionsTetGenWidget
    settings = MagicMock()
    widget = OptionsTetGenWidget(settings, main_window)
    qtbot.addWidget(widget)
    yield widget


def test_get_params(meshing_options_widget):
    params = meshing_options_widget.getParams()
    assert params['attributes'] is False
    assert params['max_volume'] is None
    assert params['insert_points'] is False
