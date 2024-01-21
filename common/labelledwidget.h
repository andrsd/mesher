#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;

class LabelledWidget : public QWidget {
    Q_OBJECT
public:
    explicit LabelledWidget(QWidget * parent = nullptr);
    ~LabelledWidget() override;

    void setLabelText(const QString & str);
    void setWidget(QWidget * widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

private:
    QVBoxLayout * layout;
    QLabel * label;
    QWidget * child;
};
