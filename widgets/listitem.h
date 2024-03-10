#pragma once

#include <QListWidget>
#include <QListWidgetItem>

class QPushButton;

class ListItem : public QWidget {
    Q_OBJECT
public:
    ListItem(QListWidgetItem * list_item);

    QListWidgetItem * listWidgetItem() const;
    void setSelected(bool state);

signals:
    void itemDeleted(QListWidgetItem * item);

protected slots:
    void onItemDeleted();

protected:
    QPushButton * btn;
    QListWidgetItem * list_item;
};
