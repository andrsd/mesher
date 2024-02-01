#include "selectioninfowidget.h"
#include <QGraphicsOpacityEffect>
#include <QVBoxLayout>
#include <QLabel>

SelectionInfoWidget::SelectionInfoWidget(QWidget * parent) :
    QWidget(parent),
    opacity(nullptr),
    layout(nullptr),
    text(nullptr)

{
    setAttribute(Qt::WA_StyledBackground, true);
    auto qss = QString("font-size: 14pt;");
    setStyleSheet(qss);
    this->opacity = new QGraphicsOpacityEffect(this);
    this->opacity->setOpacity(1.0);
    setGraphicsEffect(this->opacity);

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);

    this->text = new QLabel();
    this->layout->addWidget(this->text);

    setLayout(this->layout);
}

SelectionInfoWidget::~SelectionInfoWidget()
{
    delete this->opacity;
    delete this->layout;
    delete this->text;
}

void
SelectionInfoWidget::setText(const QString & text)
{
    this->text->setText(text);
}

void
SelectionInfoWidget::clear()
{
    this->text->setText("");
}
