#include "pointtool.h"
#include "widgets/floatwidget.h"
#include <QFormLayout>
#include <QLabel>

PointTool::PointTool(const QString & name, QWidget * parent) : BaseTool(name, parent)
{
    setWindowTitle("Point");

    auto l = layout();
    this->x = new FloatWidget("X", this);
    this->x->setMaximumWidth(QWIDGETSIZE_MAX);
    l->addRow("X", this->x);

    this->y = new FloatWidget("Y", this);
    this->y->setMaximumWidth(QWIDGETSIZE_MAX);
    l->addRow("Y", this->y);

    this->z = new FloatWidget("Z", this);
    this->z->setMaximumWidth(QWIDGETSIZE_MAX);
    l->addRow("Z", this->z);

    auto * lbl = l->labelForField(this->x);
    lbl->setFixedWidth(20);

    connectSignals();

    this->x->setFocus();
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
