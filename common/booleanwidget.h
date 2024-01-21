#pragma once

#include <QWidget>

class QCheckBox;
class QHBoxLayout;
class QLabel;

class BooleanWidget : public QWidget {
    Q_OBJECT
public:
    explicit BooleanWidget(const QString & label, QWidget * parent = nullptr);
    ~BooleanWidget() override;

    bool value();
    void setValue(bool state);

private:
    QHBoxLayout * layout;
    QLabel * label;
    QCheckBox * check;
};
