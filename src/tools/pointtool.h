#pragma once

#include "basetool.h"

class FloatWidget;

class PointTool : public BaseTool {
public:
    PointTool(const QString & name, QWidget * parent);

protected:
    void connectSignals();
    void updateWidgets() override;

    FloatWidget * x;
    FloatWidget * y;
    FloatWidget * z;
};
