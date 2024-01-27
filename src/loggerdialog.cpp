#include "loggerdialog.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QDebug>

LoggerDialog::LoggerDialog(MainWindow * parent) :
    QDialog(parent),
    GmshMessage(),
    main_window(parent)
{
    setWindowTitle("Messages");
    this->layout = new QVBoxLayout();

    this->messages = new QListWidget();
    this->layout->addWidget(this->messages);

    setLayout(this->layout);

    connect(this, &LoggerDialog::newMessage, this, &LoggerDialog::onNewMessage);
}

void
LoggerDialog::onNewMessage(const QString & msg)
{
    auto * item = new QListWidgetItem(msg);
    this->messages->addItem(item);
    this->messages->scrollToItem(item);
}

void
LoggerDialog::operator()(std::string level, std::string message)
{
    qDebug() << QString(level.c_str()) << ": " << QString(message.c_str());

    QString msg = QString("%1: %2").arg(level.c_str(), message.c_str());
    emit newMessage(msg);
}
