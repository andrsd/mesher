#include "aboutdlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QPalette>
#include <QDesktopServices>
#include <QSvgWidget>
#include <QPushButton>
#include <QFile>
#include "common/clickablelabel.h"
#include "mesherconfig.h"
#include "textboxdlg.h"

namespace {

QString HOME_PAGE_URL("https://github.com/andrsd/mesher");

}

AboutDialog::AboutDialog(QWidget * parent) :
    QDialog(parent),
    layout(nullptr),
    icon(nullptr),
    title(nullptr),
    version(nullptr),
    homepage(nullptr),
    copyright(nullptr),
    license_dlg(nullptr),
    ack_dlg(nullptr)
{
    setWindowFlag(Qt::CustomizeWindowHint, true);
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    this->layout = new QHBoxLayout();
    this->layout->addSpacing(8);

    QByteArray svg;
    QString file_name(":/app-icon-svg");
    QFile file(file_name);
    if (file.open(QIODevice::ReadOnly))
        svg = file.readAll();

    this->icon = new QSvgWidget();
    this->icon->setFixedSize(128, 128);
    this->icon->setContentsMargins(16, 16, 16, 16);
    this->icon->load(svg);
    this->layout->addWidget(this->icon, 0, Qt::AlignTop);

    auto rside = new QVBoxLayout();
    rside->setSpacing(0);

    this->title = new QLabel(MESHER_APP_NAME);
    QFont font = this->title->font();
    font.setBold(true);
    font.setPointSize(int(2 * font.pointSize()));
    this->title->setFont(font);
    rside->addWidget(this->title);

    this->version = new QLabel(QString("Version %1").arg(MESHER_VERSION));
    font = this->version->font();
    this->version->setFont(font);
    rside->addWidget(this->version);

    rside->addSpacing(4);

    this->homepage = new ClickableLabel();
    this->homepage->setText(HOME_PAGE_URL);
    auto plt = QApplication::palette();
    auto link_clr = plt.color(QPalette::Link);
    this->homepage->setStyleSheet(QString("color: %1").arg(link_clr.name()));
    font = this->homepage->font();
    this->homepage->setFont(font);
    rside->addWidget(this->homepage);

    rside->addSpacing(40);

    this->copyright = new QLabel(MESHER_COPYRIGHT);
    font = this->copyright->font();
    this->copyright->setFont(font);
    rside->addWidget(this->copyright);

    auto btn_layout = new QHBoxLayout();
    btn_layout->setSpacing(16);

    auto ack_btn = new QPushButton("Acknowledgements...");
    btn_layout->addWidget(ack_btn);

    auto license_btn = new QPushButton("License...");
    btn_layout->addWidget(license_btn);

    rside->addSpacing(16);
    rside->addLayout(btn_layout);

    this->layout->addLayout(rside, 1);

    setLayout(this->layout);

    connect(this->homepage, &ClickableLabel::clicked, this, &AboutDialog::onHomepageClicked);
    connect(ack_btn, &QPushButton::clicked, this, &AboutDialog::onAcknowledgements);
    connect(license_btn, &QPushButton::clicked, this, &AboutDialog::onLicense);
}

AboutDialog::~AboutDialog()
{
    delete this->layout;
    delete this->icon;
    delete this->title;
    delete this->version;
    delete this->copyright;
    delete this->license_dlg;
    delete this->ack_dlg;
}

void
AboutDialog::onHomepageClicked()
{
    QDesktopServices::openUrl(QUrl(HOME_PAGE_URL));
}

void
AboutDialog::onAcknowledgements()
{
    if (this->ack_dlg == nullptr) {
        this->ack_dlg = new TextBoxDialog(this);
        this->ack_dlg->setWindowTitle("Acknowledgements");
        this->ack_dlg->loadTextFromFile(":/acknowledgements");
        this->ack_dlg->setMinimumSize(650, 550);
    }
    this->ack_dlg->show();
}

void
AboutDialog::onLicense()
{
    if (this->license_dlg == nullptr) {
        this->license_dlg = new TextBoxDialog(this);
        this->license_dlg->setWindowTitle("License");
        this->license_dlg->loadTextFromFile(":/license");
        this->license_dlg->setMinimumSize(500, 600);
    }
    this->license_dlg->show();
}
