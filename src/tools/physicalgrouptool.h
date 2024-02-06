#pragma once

#include "basetool.h"

class FloatWidget;
class ListWidget;

class PhysicalGroupTool : public BaseTool {
    Q_OBJECT
public:
    enum Type { POINT, CURVE, SURFACE, VOLUME };

    PhysicalGroupTool(Type type, const QString & name, QWidget * parent);

public slots:
    void onSelectedGeomEntity();

protected:
    void connectSignals();
    void updateWidgets() override;

    Type type;
    QLineEdit * name;
    QLineEdit * tag;
    ListWidget * entities;
};
