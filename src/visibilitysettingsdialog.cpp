#include "visibilitysettingsdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QVariant>
#include "common/booleanwidget.h"
#include "common/radiooptionswidget.h"
#include "common/sectiontitlewidget.h"
#include "mainwindow.h"

VisibilitySettingsDialog::VisibilitySettingsDialog(QWidget * parent) : QDialog(parent)
{
    setWindowTitle("Visibility");
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::CustomizeWindowHint);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    setFixedWidth(200);

    this->tabs = new QTabWidget();
    this->tabs->addTab(createGeoWidget(), "Geometry");
    this->tabs->addTab(createMeshWidget(), "Mesh");

    auto fnt = this->tabs->font();
    int fnt_pt = fnt.pointSize() * 0.9;
    this->tabs->setStyleSheet(QString("QWidget { font-size: %1pt; }").arg(fnt_pt));

    auto layout = new QVBoxLayout();
    layout->addWidget(this->tabs);
    layout->setContentsMargins(8, 16, 8, 8);
    setLayout(layout);
}

QWidget *
VisibilitySettingsDialog::createGeoWidget()
{
    auto [tab, layout] = createTab();

    auto settings = MainWindow::getSettings();

    auto points = new BooleanWidget("Points");
    points->bindToSettings(settings, "visibility/geo/points", true);
    connect(points, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(points);

    auto curves = new BooleanWidget("Curves");
    curves->bindToSettings(settings, "visibility/geo/curves", true);
    connect(curves, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(curves);

    auto surfaces = new BooleanWidget("Surfaces");
    surfaces->bindToSettings(settings, "visibility/geo/surfaces", false);
    connect(surfaces, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(surfaces);

    auto volumes = new BooleanWidget("Volumes");
    volumes->bindToSettings(settings, "visibility/geo/volumes", false);
    connect(volumes, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(volumes);

    auto label_section = new SectionTitleWidget("Labels");
    layout->addLayout(label_section);

    auto label_type = new RadioOptionsWidget({ "Description",
                                               "Elementary tags",
                                               "Physical tag(s)",
                                               "Elementary name",
                                               "Physical name(s)" });
    label_type->bindToSettings(settings, "visibility/geo/label_type", 0);
    connect(label_type, &RadioOptionsWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(label_type);

    auto point_labels = new BooleanWidget("Point labels");
    point_labels->bindToSettings(settings, "visibility/geo/point_labels", false);
    connect(point_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(point_labels);

    auto curve_labels = new BooleanWidget("Curve labels");
    curve_labels->bindToSettings(settings, "visibility/geo/curve_labels", false);
    connect(curve_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(curve_labels);

    auto surface_labels = new BooleanWidget("Surface labels");
    surface_labels->bindToSettings(settings, "visibility/geo/surface_labels", false);
    connect(surface_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(surface_labels);

    auto volume_labels = new BooleanWidget("Volume labels");
    volume_labels->bindToSettings(settings, "visibility/geo/volume_labels", false);
    connect(volume_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(volume_labels);

    layout->addStretch();

    return tab;
}

QWidget *
VisibilitySettingsDialog::createMeshWidget()
{
    auto [tab, layout] = createTab();

    auto settings = MainWindow::getSettings();

    auto nodes = new BooleanWidget("Nodes");
    nodes->bindToSettings(settings, "visibility/mesh/nodes", false);
    connect(nodes, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(nodes);

    auto one_d = new BooleanWidget("1D elements");
    one_d->bindToSettings(settings, "visibility/mesh/1d_elements", false);
    connect(one_d, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(one_d);

    auto two_d_edges = new BooleanWidget("2D element edges");
    two_d_edges->bindToSettings(settings, "visibility/mesh/2d_edges", true);
    connect(two_d_edges, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(two_d_edges);

    auto two_d_faces = new BooleanWidget("2D element faces");
    two_d_faces->bindToSettings(settings, "visibility/mesh/2d_faces", false);
    connect(two_d_faces, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(two_d_faces);

    auto three_d_edges = new BooleanWidget("3D element edges");
    three_d_edges->bindToSettings(settings, "visibility/mesh/3d_edges", true);
    connect(three_d_edges, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(three_d_edges);

    auto three_d_faces = new BooleanWidget("3D element faces");
    three_d_faces->bindToSettings(settings, "visibility/mesh/3d_faces", false);
    connect(three_d_faces, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(three_d_faces);

    auto label_section = new SectionTitleWidget("Labels");
    layout->addLayout(label_section);

    auto label_type = new RadioOptionsWidget({ "Node/elements tag",
                                               "Elementary entity tag",
                                               "Physical groups tag(s)",
                                               "Mesh partition",
                                               "Coordinates" });
    label_type->bindToSettings(settings, "visibility/mesh/label_type", 0);
    connect(label_type, &RadioOptionsWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(label_type);

    auto node_labels = new BooleanWidget("Node labels");
    node_labels->bindToSettings(settings, "visibility/mesh/node_labels", false);
    connect(node_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(node_labels);

    auto one_d_labels = new BooleanWidget("1D element labels");
    one_d_labels->bindToSettings(settings, "visibility/mesh/one_d_labels", false);
    connect(one_d_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(one_d_labels);

    auto two_d_labels = new BooleanWidget("2D element labels");
    two_d_labels->bindToSettings(settings, "visibility/mesh/two_d_labels", false);
    connect(two_d_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(two_d_labels);

    auto three_d_labels = new BooleanWidget("3D element labels");
    three_d_labels->bindToSettings(settings, "visibility/mesh/three_d_labels", false);
    connect(three_d_labels, &BooleanWidget::changed, this, &VisibilitySettingsDialog::onChanged);
    layout->addWidget(three_d_labels);

    layout->addStretch();

    return tab;
}

std::tuple<QWidget *, QVBoxLayout *>
VisibilitySettingsDialog::createTab()
{
    auto tab_layout = new QVBoxLayout();
    tab_layout->setContentsMargins(8, 10, 8, 10);
    tab_layout->setSpacing(8);

    auto pane_widget = new QWidget();
    pane_widget->setLayout(tab_layout);

    return { pane_widget, tab_layout };
}

void
VisibilitySettingsDialog::onChanged()
{
    emit changed();
}
