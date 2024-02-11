#include "physicalgrouptool.h"
#include "common/floatwidget.h"
#include "common/listwidget.h"
#include <QFormLayout>
#include <QLabel>
#include "GEntity.h"

int PhysicalGroupTool::N_POINTS = 0;
int PhysicalGroupTool::N_CURVES = 0;
int PhysicalGroupTool::N_SURFACES = 0;
int PhysicalGroupTool::N_VOLUMES = 0;

QString
PhysicalGroupTool::typeToName(Type type)
{
    if (type == POINT)
        return "Point";
    else if (type == CURVE)
        return "Curve";
    else if (type == SURFACE)
        return "Surface";
    else if (type == VOLUME)
        return "Volume";
    else
        return "Unknown";
}

PhysicalGroupTool::PhysicalGroupTool(Type type, int tag, QWidget * parent) :
    BaseTool(QString("Physical %1 %2").arg(typeToName(type), QString::number(tag)), parent),
    type(type)
{
    setWindowTitle(QString("Physical %1").arg(typeToName(type)));

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
PhysicalGroupTool::setEntities(const std::vector<GEntity *> & ents)
{
    for (auto & e : ents)
        addEntity(e);
}

void
PhysicalGroupTool::addEntity(GEntity * e)
{
    auto item = new QListWidgetItem(this->entities);
    auto name = QString("%1 %2").arg(e->getTypeString().c_str(), QString::number(e->tag()));
    item->setText(name);
    item->setData(Qt::UserRole, QVariant::fromValue(e));
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
    bool ok = !this->name->text().isEmpty() && !this->tag->text().isEmpty() &&
              this->entities->count() > 0;
    if (ok)
        enableOkButton();
    else
        enableOkButton(false);
}

void
PhysicalGroupTool::onSelectedGeomEntity()
{
}
