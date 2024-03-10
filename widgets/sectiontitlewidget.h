#pragma once

#include <QHBoxLayout>

class QLabel;

class SectionTitleWidget : public QHBoxLayout {
    Q_OBJECT
public:
    SectionTitleWidget(const QString & name, QWidget * parent = nullptr);
    ~SectionTitleWidget() = default;

    QLabel * text;
};
