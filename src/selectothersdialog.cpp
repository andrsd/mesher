#include "selectothersdialog.h"
#include "view.h"
#include "GEntity.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QVariant>

SelectOthersDialog::SelectOthersDialog(View * view) : QDialog(view), view(view)
{
    setWindowTitle("Select");
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::CustomizeWindowHint);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    setFixedWidth(150);

    auto fnt = this->font();
    int fnt_pt = fnt.pointSize() * 0.9;
    this->setStyleSheet(QString("QWidget { font-size: %1pt; }").arg(fnt_pt));

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);

    this->entities = new QListWidget();
    this->entities->setStyleSheet("QListWidget { border: none; }");

    this->layout->addWidget(this->entities);

    std::vector<GEntity *> ents;
    setEntities(ents);

    setLayout(this->layout);

    connect(this->entities, &QListWidget::itemEntered, this, &SelectOthersDialog::onItemEntered);
    connect(this->entities, &QListWidget::itemClicked, this, &SelectOthersDialog::onItemClicked);
}

SelectOthersDialog::~SelectOthersDialog()
{
    delete this->layout;
}

void
SelectOthersDialog::setEntities(const std::vector<GEntity *> & ents)
{
    this->entities->clear();

    for (auto & e : ents) {
        QListWidgetItem * item = new QListWidgetItem(this->entities);
        auto text = QString("%1 %2").arg(e->getTypeString().c_str(), QString::number(e->tag()));
        item->setText(text);
        item->setData(Qt::UserRole, QVariant::fromValue(e));
        this->entities->addItem(item);
    }
}

void
SelectOthersDialog::onItemEntered(QListWidgetItem * item)
{
    auto e = item->data(Qt::UserRole).value<GEntity *>();
    qDebug() << "entered = " << e->getTypeString().c_str() << e->tag();
}

void
SelectOthersDialog::onItemClicked(QListWidgetItem * item)
{
    auto e = item->data(Qt::UserRole).value<GEntity *>();
    qDebug() << "clicked = " << e->getTypeString().c_str() << e->tag();
    this->view->selectEntity(e);
    this->hide();
}
