#pragma once

#include <QWidget>

class QGraphicsOpacityEffect;
class QVBoxLayout;
class QLabel;

class SelectionInfoWidget : public QWidget {
public:
    explicit SelectionInfoWidget(QWidget * parent = nullptr);
    ~SelectionInfoWidget() override;

    void setText(const QString & text);
    void clear();

protected:
    QGraphicsOpacityEffect * opacity;
    QVBoxLayout * layout;
    QLabel * text;
};
