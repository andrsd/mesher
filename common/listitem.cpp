#include "listitem.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPalette>
#include <QApplication>

ListItem::ListItem(QListWidgetItem * list_item) : list_item(list_item)
{
    auto l = new QHBoxLayout();
    l->setContentsMargins(4, 0, 4, 0);

    auto lbl = new QLabel();
    l->addWidget(lbl);

    this->btn = new QPushButton("\u2A09");
    this->btn->setContentsMargins(0, 0, 0, 0);
    this->btn->setFixedSize(12, 12);
    setSelected(false);
    l->addWidget(this->btn);

    setLayout(l);

    connect(this->btn, &QPushButton::clicked, this, &ListItem::onItemDeleted);
}

void
ListItem::setSelected(bool state)
{
    QColor clr;
    // FIXME: this should be populated from palette
    if (state)
        clr = Qt::white;
    else
        clr = Qt::darkBlue;

    QString qss = QString("border: none;"
                          "color: %1;"
                          "font-weight: bold;")
                      .arg(clr.name());
    this->btn->setStyleSheet(qss);
}

QListWidgetItem *
ListItem::listWidgetItem() const
{
    return this->list_item;
}

void
ListItem::onItemDeleted()
{
    emit itemDeleted(this->list_item);
}
