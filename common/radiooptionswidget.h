#pragma once

#include <QWidget>
#include <QList>

class QVBoxLayout;
class QLabel;
class QRadioButton;

class RadioOptionsWidget : public QWidget {
    Q_OBJECT
public:
    explicit RadioOptionsWidget(const QStringList & str_options, QWidget * parent = nullptr);
    ~RadioOptionsWidget() override;

    int value() const;
    void setValue(int value);

private:
    QVBoxLayout * layout;
    QList<QRadioButton *> options;
};
