#include "selectothersdialog.h"
#include "view.h"
#include "utils.h"
#include "widgets/tooltitlewidget.h"
#include "widgets/cancelbutton.h"
#include "GEntity.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QVariant>

SelectOthersDialog::SelectOthersDialog(View * view) : ToolWindow(view), view(view)
{
    setWindowTitle("Select other");
    setFixedWidth(150);

    this->setStyleSheet("QWidget { font-size: 12pt; }");

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(4, 4, 4, 4);
    setLayout(this->layout);

    this->entities = new QListWidget();
    this->entities->setStyleSheet("QListWidget {"
                                  "  border: 1px solid rgb(221, 221, 221);"
                                  "}"
                                  "QListWidget:hover {"
                                  "  border: 1px solid rgb(40, 80, 170);"
                                  "}");

    this->layout->addWidget(this->entities);

    std::vector<GEntity *> ents;
    setEntities(ents);

    connect(this->entities, &QListWidget::itemEntered, this, &SelectOthersDialog::onItemEntered);
    connect(this->entities, &QListWidget::itemClicked, this, &SelectOthersDialog::onItemClicked);
}

SelectOthersDialog::~SelectOthersDialog() {}

void
SelectOthersDialog::setEntities(const std::vector<GEntity *> & ents)
{
    this->entities->clear();

    auto bold_fnt = this->font();
    bold_fnt.setWeight(QFont::Bold);

    for (auto & e : ents) {
        QListWidgetItem * item = new QListWidgetItem(this->entities);
        item->setText(getEntityName(e));
        item->setData(Qt::UserRole, QVariant::fromValue(e));
        if (e == this->view->getHighlightedEntity())
            item->setFont(bold_fnt);
        this->entities->addItem(item);
    }
}

void
SelectOthersDialog::onItemEntered(QListWidgetItem * item)
{
    auto e = item->data(Qt::UserRole).value<GEntity *>();
}

void
SelectOthersDialog::onItemClicked(QListWidgetItem * item)
{
    auto e = item->data(Qt::UserRole).value<GEntity *>();
    this->view->selectEntity(e);
    this->hide();
}
