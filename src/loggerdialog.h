#pragma once

#include <QDialog>
#include "GmshMessage.h"

class MainWindow;
class QVBoxLayout;
class QListWidget;

class LoggerDialog : public QDialog, public GmshMessage {
public:
    LoggerDialog(MainWindow * parent);
    void operator()(std::string level, std::string message) override;

private:
    MainWindow * main_window;
    QVBoxLayout * layout;
    QListWidget * messages;
};
