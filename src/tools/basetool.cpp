#include "basetool.h"
#include "widgets/namewidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

BaseTool::BaseTool(const QString & name, QWidget * parent) : ToolWindow(name, parent)
{
    setFixedWidth(200);

    this->form_layout = new QFormLayout();
    this->form_layout->setSizeConstraint(QLayout::SetMaximumSize);
    this->form_layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->form_layout->setContentsMargins(4, 4, 4, 4);
    this->form_layout->setSpacing(2);
    this->form_layout->setLabelAlignment(Qt::AlignLeft);
    setLayout(this->form_layout);
}

QFormLayout *
BaseTool::layout()
{
    return this->form_layout;
}

void
BaseTool::showEvent(QShowEvent * event)
{
    updateWidgets();
}
