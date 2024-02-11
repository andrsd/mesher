#pragma once

#include "basetool.h"

class FloatWidget;
class ListWidget;
class GEntity;

class PhysicalGroupTool : public BaseTool {
    Q_OBJECT
public:
    enum Type { POINT, CURVE, SURFACE, VOLUME };

    PhysicalGroupTool(Type type, int tag, QWidget * parent);

    void setEntities(const std::vector<GEntity *> & ents);

public slots:
    void onSelectedGeomEntity();

protected:
    void connectSignals();
    void updateWidgets() override;
    void addEntity(GEntity * e);

    static QString typeToName(Type type);

    Type type;
    QLineEdit * name;
    QLineEdit * tag;
    ListWidget * entities;

public:
    static int N_POINTS;
    static int N_CURVES;
    static int N_SURFACES;
    static int N_VOLUMES;
};
