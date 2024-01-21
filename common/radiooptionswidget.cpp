#include "radiooptionswidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSignalMapper>

RadioOptionsWidget::RadioOptionsWidget(const QStringList & str_options, QWidget * parent) :
    QWidget(parent)
{
    this->signal_mapper = new QSignalMapper(this);

    this->layout = new QVBoxLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    setLayout(this->layout);

    for (auto & o : str_options) {
        auto * rb = new QRadioButton(o, this);
        connect(rb, &QRadioButton::toggled, this->signal_mapper, qOverload<>(&QSignalMapper::map));
        this->signal_mapper->setMapping(rb, this->options.size());
        this->options.append(rb);
        this->layout->addWidget(rb);
    }
    connect(this->signal_mapper, &QSignalMapper::mappedInt, this, &RadioOptionsWidget::onChanged);
}

RadioOptionsWidget::~RadioOptionsWidget()
{
    for (auto & it : this->options)
        delete it;
    delete this->layout;
}

int
RadioOptionsWidget::value() const
{
    for (int i = 0; i < this->options.size(); i++)
        if (this->options[i]->isChecked())
            return i;
    return -1;
}

void
RadioOptionsWidget::setValue(int value)
{
    if ((value >= 0) && (value < this->options.size()))
        this->options[value]->setChecked(Qt::CheckState::Checked);
}

void
RadioOptionsWidget::bindToSettings(QSettings * settings,
                                   const QString & key,
                                   const QVariant & default_value)
{
    bindSettings(settings, key);
    setValue(readSetting(default_value).toInt());
}

void
RadioOptionsWidget::onChanged(int id)
{
    if (this->options[id]->isChecked())
        storeSetting(id);
}
