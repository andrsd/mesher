#pragma once

#include <QListWidget>

class QSignalMapper;
class QShortcut;

class ListWidget : public QListWidget {
    Q_OBJECT
public:
    ListWidget(QWidget * parent = nullptr);

    void addItem(QListWidgetItem * item);

protected slots:
    void onDeletePressed();
    void onDeleteItem(QListWidgetItem * item);

protected:
    QSignalMapper * signal_mapper;
    QShortcut * del_kbd;
};
