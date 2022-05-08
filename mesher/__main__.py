import sys
import os
import signal
from PyQt5.QtCore import Qt, QStandardPaths, QCoreApplication, QTimer
from PyQt5.QtWidgets import QApplication
from mesher import consts
from mesher.MainWindow import MainWindow


def safe_timer(timeout, func, *args, **kwargs):
    """Create a timer that is safe against garbage collection and \
    overlapping calls.

    See: http://ralsina.me/weblog/posts/BB974.html
    """
    def timer_event():
        try:
            func(*args, **kwargs)
        finally:
            QTimer.singleShot(timeout, timer_event)
    QTimer.singleShot(timeout, timer_event)


def handle_sigint(signum, frame):
    QApplication.quit()


def handle_uncaught_exception(exc_type, exc, traceback):
    print('Unhandled exception', exc_type, exc, traceback)
    QApplication.quit()


sys.excepthook = handle_uncaught_exception


def main():
    home_dir = QStandardPaths.writableLocation(QStandardPaths.HomeLocation)
    os.chdir(home_dir)

    QCoreApplication.setOrganizationName("David Andrs")
    QCoreApplication.setOrganizationDomain("name.andrs")
    QCoreApplication.setApplicationName(consts.APP_NAME)

    QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
    QCoreApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)

    qapp = QApplication(sys.argv)
    qapp.setQuitOnLastWindowClosed(False)

    window = MainWindow()
    signal.signal(signal.SIGINT, handle_sigint)
    window.show()

    # Repeatedly run python-noop to give the interpreter time to
    # handle signals
    safe_timer(50, lambda: None)

    qapp.exec()


if __name__ == '__main__':
    main()
