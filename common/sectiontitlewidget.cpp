#include "sectiontitlewidget.h"

SectionTitleWidget::SectionTitleWidget(const QString & name, QWidget * parent) :
    QLabel(name, parent)
{
    this->setStyleSheet("QLabel { font-weight: bold }");
    this->setContentsMargins(0, 20, 0, 4);
}
