#include "sectiontitlewidget.h"
#include <QLabel>

SectionTitleWidget::SectionTitleWidget(const QString & name, QWidget * parent) : QHBoxLayout()
{
    auto line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setContentsMargins(0, 10, 0, 10);
    line->setFixedHeight(10);
    line->setStyleSheet("color: #D3D3D3;");
    addWidget(line);
}
