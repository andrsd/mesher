import platform
import pytest
from unittest.mock import MagicMock, patch


if platform.system() == "Darwin":
    @pytest.fixture
    def notification_widget(qtbot, main_window):
        from mesher.NotificationWidget import NotificationWidget
        widget = NotificationWidget(main_window)
        qtbot.addWidget(widget)
        yield widget

    def test_set_text(notification_widget):
        notification_widget.setText("asdf")
        assert notification_widget.text.text(), "asdf"

    @patch('mesher.NotificationWidget.NotificationWidget.hide')
    def test_on_dismiss(hide, notification_widget):
        notification_widget.dismiss = MagicMock()
        notification_widget.onDismiss()
        hide.assert_called_once()
        notification_widget.dismiss.emit.assert_called_once()

    @patch('PyQt5.QtCore.QTimer.singleShot')
    def test_show(single_shot, notification_widget):
        notification_widget.show()
        single_shot.assert_called_once()

    @patch('PyQt5.QtCore.QPropertyAnimation.start')
    def test_on_notification_fade_out(start, notification_widget):
        notification_widget.onNotificationFadeOut()
        start.assert_called_once()
