#include "physicalgrouptool.h"
#include "common/floatwidget.h"
#include "common/listwidget.h"
#include <QFormLayout>
#include <QLabel>

PhysicalGroupTool::PhysicalGroupTool(Type type, const QString & name, QWidget * parent) :
    BaseTool(name, parent),
    type(type)
{
    setWindowTitle("Physical Group");

    auto l = layout();

    this->name = new QLineEdit(this);
    l->addRow("Name", this->name);

    this->tag = new QLineEdit(this);
    l->addRow("Tag", this->tag);

    this->entities = new ListWidget(this);
    this->entities->setMinimumHeight(100);
    this->entities->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    l->addRow(this->entities);
    
    connectSignals();

    this->name->setFocus();
}

void
PhysicalGroupTool::connectSignals()
{
    connect(this->name, &QLineEdit::textChanged, this, &PhysicalGroupTool::updateWidgets);
    connect(this->tag, &QLineEdit::textChanged, this, &PhysicalGroupTool::updateWidgets);
}

void
PhysicalGroupTool::updateWidgets()
{
    if (!this->name->text().isEmpty() && !this->tag->text().isEmpty())
        enableOkButton();
    else
        enableOkButton(false);
}

void
PhysicalGroupTool::onSelectedGeomEntity()
{
}
