#pragma once

#include <QDialog>

class QHBoxLayout;
class QLabel;
class QSvgWidget;
class ClickableLabel;
class TextBoxDialog;

class AboutDialog : public QDialog {
public:
    explicit AboutDialog(QWidget * parent = nullptr);
    ~AboutDialog() override;

protected slots:
    void onHomepageClicked();
    void onAcknowledgements();
    void onLicense();

protected:
    QHBoxLayout * layout;
    QSvgWidget * icon;
    QLabel * title;
    QLabel * version;
    ClickableLabel * homepage;
    QLabel * copyright;
    TextBoxDialog * license_dlg;
    TextBoxDialog * ack_dlg;
};
