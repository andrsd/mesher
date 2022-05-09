import pytest
from unittest.mock import MagicMock


@pytest.fixture
def meshing_options_widget(qtbot, main_window):
    from mesher.OptionsTriangleWidget import OptionsTriangleWidget
    settings = MagicMock()
    widget = OptionsTriangleWidget(settings, main_window)
    qtbot.addWidget(widget)
    yield widget


def test_get_params(meshing_options_widget):
    meshing_options_widget.max_area_constraint.setText('0.1')
    params = meshing_options_widget.getParams()

    assert params['attributes'] is False
    assert params['max_volume'] == 0.1
    assert params['allow_boundary_steiner'] is True
    assert params['mesh_order'] is False
    assert params['quality_meshing'] is True
    assert params['min_angle'] is None
