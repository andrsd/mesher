#include "selectothersdialog.h"
#include "view.h"
#include "utils.h"
#include "widgets/tooltitlewidget.h"
#include "widgets/cancelbutton.h"
#include "GEntity.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QVariant>

SelectOthersDialog::SelectOthersDialog(View * view) : QDialog(view), view(view)
{
    setWindowTitle("Select other");
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::FramelessWindowHint);
    setStyleSheet("QDialog {"
                  "  background-color: rgb(255, 255, 255);"
                  "}");

    setFixedWidth(150);

    this->setStyleSheet("QWidget { font-size: 12pt; }");

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setSpacing(0);
    setLayout(this->layout);

    auto btn_layout = new QHBoxLayout();
    btn_layout->setContentsMargins(0, 0, 0, 0);
    btn_layout->setSpacing(0);

    this->title = new ToolTitleWidget(this);
    this->title->setText("Select other");
    btn_layout->addWidget(this->title);

    this->cancel = new CancelButton();
    btn_layout->addWidget(this->cancel);

    this->layout->addLayout(btn_layout);

    this->body_layout = new QVBoxLayout();
    this->body_layout->setContentsMargins(4, 4, 4, 4);
    this->layout->addLayout(this->body_layout);

    this->entities = new QListWidget();
    this->entities->setStyleSheet("QListWidget {"
                                  "  border: 1px solid rgb(221, 221, 221);"
                                  "}"
                                  "QListWidget:hover {"
                                  "  border: 1px solid rgb(40, 80, 170);"
                                  "}");

    this->body_layout->addWidget(this->entities);

    std::vector<GEntity *> ents;
    setEntities(ents);

    connect(this->cancel, &QPushButton::clicked, this, &SelectOthersDialog::onCancel);

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

void
SelectOthersDialog::onCancel()
{
    reject();
}
