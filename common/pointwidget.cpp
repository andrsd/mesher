#include "pointwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

PointWidget::PointWidget(QWidget * parent) : QWidget(parent)
{
    const int wd = 50;

    this->validator = new QDoubleValidator();

    this->x_lbl = new QLabel("X");
    this->x_edit = new QLineEdit();
    this->x_edit->setValidator(this->validator);
    this->x_edit->setFixedWidth(wd);

    this->y_lbl = new QLabel("Y");
    this->y_edit = new QLineEdit();
    this->y_edit->setValidator(this->validator);
    this->y_edit->setFixedWidth(wd);

    this->z_lbl = new QLabel("Z");
    this->z_edit = new QLineEdit();
    this->z_edit->setValidator(this->validator);
    this->z_edit->setFixedWidth(wd);

    this->layout = new QHBoxLayout();
    this->layout->addWidget(this->x_lbl);
    this->layout->addWidget(this->x_edit);
    this->layout->addWidget(this->y_lbl);
    this->layout->addWidget(this->y_edit);
    this->layout->addWidget(this->z_lbl);
    this->layout->addWidget(this->z_edit);

    this->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(this->layout);

    connect(this->x_edit, &QLineEdit::textChanged, this, &PointWidget::onXTextChanged);
    connect(this->y_edit, &QLineEdit::textChanged, this, &PointWidget::onYTextChanged);
    connect(this->z_edit, &QLineEdit::textChanged, this, &PointWidget::onZTextChanged);
}

PointWidget::~PointWidget()
{
    delete this->validator;
    delete this->layout;
    delete this->x_lbl;
    delete this->x_edit;
    delete this->y_lbl;
    delete this->y_edit;
    delete this->z_lbl;
    delete this->z_edit;
}

QVector3D
PointWidget::value() const
{
    auto x = this->x_edit->text().toDouble();
    auto y = this->y_edit->text().toDouble();
    auto z = this->z_edit->text().toDouble();
    return QVector3D(x, y, z);
}

void
PointWidget::setValue(const QVector3D & value)
{
    auto x = QString("%1").arg(value.x());
    this->x_edit->setText(x);

    auto y = QString("%1").arg(value.y());
    this->y_edit->setText(y);

    auto z = QString("%1").arg(value.z());
    this->z_edit->setText(z);
}

void
PointWidget::bindToSettings(QSettings * settings,
                            const QString & key,
                            const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).value<QVector3D>());
}

void
PointWidget::onXTextChanged(const QString & text)
{
    auto x = text.toDouble();
    auto y = this->y_edit->text().toDouble();
    auto z = this->z_edit->text().toDouble();
    storeSetting(QVector3D(x, y, z));
}

void
PointWidget::onYTextChanged(const QString & text)
{
    auto x = this->x_edit->text().toDouble();
    auto y = text.toDouble();
    auto z = this->z_edit->text().toDouble();
    storeSetting(QVector3D(x, y, z));
}

void
PointWidget::onZTextChanged(const QString & text)
{
    auto x = this->x_edit->text().toDouble();
    auto y = this->y_edit->text().toDouble();
    auto z = text.toDouble();
    storeSetting(QVector3D(x, y, z));
}
