#include "pointtool.h"
#include "common/floatwidget.h"
#include <QFormLayout>

PointTool::PointTool(const QString & name, QWidget * parent) : BaseTool(name, parent)
{
    setWindowTitle("Point");

    auto l = layout();
    this->x = new FloatWidget("X", this);
    l->addRow("X", this->x);

    this->y = new FloatWidget("Y", this);
    l->addRow("Y", this->y);

    this->z = new FloatWidget("Z", this);
    l->addRow("Z", this->z);

    connectSignals();
}

void
PointTool::connectSignals()
{
    connect(this->x, &FloatWidget::changed, this, &PointTool::updateWidgets);
    connect(this->y, &FloatWidget::changed, this, &PointTool::updateWidgets);
    connect(this->z, &FloatWidget::changed, this, &PointTool::updateWidgets);
}

void
PointTool::updateWidgets()
{
    if (this->x->hasValue() && this->y->hasValue() && this->z->hasValue())
        enableOkButton();
    else
        enableOkButton(false);
}
