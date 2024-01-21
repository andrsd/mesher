#pragma once

#include <QLabel>

class SectionTitleWidget : public QLabel {
    Q_OBJECT
public:
    SectionTitleWidget(const QString & name, QWidget * parent = nullptr);
    ~SectionTitleWidget() = default;
};
