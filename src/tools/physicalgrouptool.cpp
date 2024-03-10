#include "physicalgrouptool.h"
#include "widgets/listwidget.h"
#include "widgets/lineedit.h"
#include "utils.h"
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
    l->setSpacing(6);

    this->name = new LineEdit(this);
    l->addRow("Name", this->name);

    this->tag = new LineEdit(this);
    auto int_val = new QIntValidator();
    int_val->setBottom(0);
    this->tag->setValidator(int_val);
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
    item->setText(getEntityName(e));
    item->setData(Qt::UserRole, QVariant::fromValue(e));
    this->entities->addItem(item);
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
