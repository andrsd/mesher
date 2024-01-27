#pragma once

#include <QDialog>
#include "GmshMessage.h"

class MainWindow;
class QVBoxLayout;
class QListWidget;

class LoggerDialog : public QDialog, public GmshMessage {
    Q_OBJECT
public:
    LoggerDialog(MainWindow * parent);
    void operator()(std::string level, std::string message) override;

signals:
    void newMessage(const QString & msg);

protected slots:
    void onNewMessage(const QString & msg);

private:
    MainWindow * main_window;
    QVBoxLayout * layout;
    QListWidget * messages;
};
