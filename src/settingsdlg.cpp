#include "settingsdlg.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QScrollArea>
#include "common/radiooptionswidget.h"
#include "common/floatwidget.h"
#include "common/integerwidget.h"
#include "common/booleanwidget.h"
#include "common/labelledcolorwidget.h"
#include "common/sectiontitlewidget.h"

SettingsDialog::SettingsDialog(QSettings * settings, QWidget * parent) :
    QDialog(parent),
    settings(settings)
{
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    setWindowTitle("Settings");
    setMinimumSize(800, 700);

    this->layout = new QHBoxLayout();

    this->categories = new QTreeWidget();
    this->categories->setHeaderHidden(true);
    this->categories->setFixedWidth(150);

    this->pane = new QStackedWidget();
    this->pane->setContentsMargins(0, 0, 0, 0);

    this->layout->addWidget(this->categories);
    this->layout->addWidget(this->pane);

    setLayout(this->layout);

    createPanes();

    this->categories->expandAll();

    connect(this->categories,
            &QTreeWidget::itemSelectionChanged,
            this,
            &SettingsDialog::onCategorySelected);
}

SettingsDialog::~SettingsDialog()
{
    delete this->layout;
    delete this->categories;
}

QTreeWidgetItem *
SettingsDialog::addPane(QWidget * pane, const QString & name, QTreeWidgetItem * parent)
{
    auto idx = this->pane->addWidget(pane);
    auto ti = new QTreeWidgetItem(parent, QStringList(name));
    ti->setData(0, Qt::UserRole, idx);
    return ti;
}

void
SettingsDialog::createPanes()
{
    auto general_pane = createGeneralPane();
    auto general = addPane(general_pane, "General", this->categories->invisibleRootItem());

    auto appearance_pane = createAppearancePane();
    auto appearance = addPane(appearance_pane, "Appearance", this->categories->invisibleRootItem());

    auto appear_geom_pane = createAppearanceGeometryPane();
    auto apperance_geom = addPane(appear_geom_pane, "Geometry", appearance);

    auto appear_mesh_pane = createAppearanceMeshPane();
    auto apperance_mesh = addPane(appear_mesh_pane, "Mesh", appearance);

    auto occ_pane = createOpenCASCADEPane();
    auto occ = addPane(occ_pane, "OpenCASCADE", this->categories->invisibleRootItem());
}

std::tuple<QWidget *, QVBoxLayout *>
SettingsDialog::createPane()
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    auto pane = new QWidget();
    pane->setLayout(layout);

    auto area = new QScrollArea();
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setWidget(pane);

    return { area, layout };
}

QWidget *
SettingsDialog::createGeneralPane()
{
    auto [pane, layout] = createPane();

    auto label = new QLabel("General");
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);

    auto show_bounding_boxes = new BooleanWidget("Show bounding boxes");
    show_bounding_boxes->bindToSettings(this->settings, "general/show_bounding_boxes", false);
    layout->addWidget(show_bounding_boxes);

    auto simple_model_user_inter =
        new BooleanWidget("Draw simplified model during user interaction");
    simple_model_user_inter->bindToSettings(this->settings,
                                            "general/draw_simple_model_on_interaction",
                                            false);
    layout->addWidget(simple_model_user_inter);

    auto double_buffering = new BooleanWidget("Enable double buffering");
    double_buffering->bindToSettings(this->settings, "general/double_buffering", true);
    layout->addWidget(double_buffering);

    auto anti_aliasing = new BooleanWidget("Enable anti-aliasing");
    anti_aliasing->bindToSettings(this->settings, "general/anti_aliasng", false);
    layout->addWidget(anti_aliasing);

    auto rotation = new RadioOptionsWidget({ "Trackball rotation", "Euler angles" });
    rotation->bindToSettings(this->settings, "general/rotation", 0);
    layout->addWidget(rotation);

    auto rot_about_com = new BooleanWidget("Rotate about pseudo center of mass");
    rot_about_com->bindToSettings(this->settings, "general/rotate_about_pseudo_com", true);
    layout->addWidget(rot_about_com);

    auto invert_zoom_direction = new BooleanWidget("Invert zoom direction");
    invert_zoom_direction->bindToSettings(this->settings, "general/invert_zoom_direction", false);
    layout->addWidget(invert_zoom_direction);

    auto geom_lbl = new QLabel("Geometry");
    layout->addWidget(geom_lbl);

    auto geometry_tolerance = new FloatWidget("Geometry tolerance");
    geometry_tolerance->bindToSettings(this->settings, "general/geometry_tolerance", 1e-8);
    layout->addWidget(geometry_tolerance);

    auto remove_duplicates = new BooleanWidget("Remove duplicate entities in GEO model transforms");
    remove_duplicates->bindToSettings(this->settings, "general/remove_geo_dups", true);
    layout->addWidget(remove_duplicates);

    auto hilight_orphans = new BooleanWidget("Highlight orphan entities");
    hilight_orphans->bindToSettings(this->settings, "general/hilight_orphans", true);
    layout->addWidget(hilight_orphans);

    layout->addStretch();

    return pane;
}

QWidget *
SettingsDialog::createAppearancePane()
{
    auto [pane, layout] = createPane();
    return pane;
}

QWidget *
SettingsDialog::createAppearanceMeshPane()
{
    auto [pane, layout] = createPane();

    auto label = new QLabel("Mesh");
    layout->addWidget(label);

    auto elem_shrink_fact = new FloatWidget("Element shrinking factor");
    elem_shrink_fact->bindToSettings(this->settings, "appearance/mesh/element_shrink_fact", 1.0);
    layout->addWidget(elem_shrink_fact);

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(this->settings, "appearance/mesh/point_display", 0);
    layout->addWidget(point_display);

    auto point_size = new FloatWidget("Point size");
    point_size->bindToSettings(this->settings, "appearance/mesh/point_size", 4.0);
    layout->addWidget(point_size);

    auto line_width = new FloatWidget("Line width");
    line_width->bindToSettings(this->settings, "appearance/mesh/line_width", 1.0);
    layout->addWidget(line_width);

    auto high_order_elem_subdivs = new IntegerWidget("High-order elements subdivisions");
    high_order_elem_subdivs->bindToSettings(this->settings,
                                            "appearance/mesh/high_order_elem_subdivs",
                                            2);
    layout->addWidget(high_order_elem_subdivs);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addWidget(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(this->settings, "appearance/mesh/enable_lighting", true);
    layout->addWidget(enable_lighting);

    auto edge_lighting_lbl = new QLabel("Edge lighting");
    layout->addWidget(edge_lighting_lbl);

    auto edge_lighting = new RadioOptionsWidget({ "No", "Surface", "Volume and surface" });
    edge_lighting->bindToSettings(this->settings, "appearance/mesh/edge_lighting", 2);
    layout->addWidget(edge_lighting);

    auto two_side_lighting = new BooleanWidget("Two-side lighting");
    two_side_lighting->bindToSettings(this->settings, "appearance/mesh/two_side_lighting", true);
    layout->addWidget(two_side_lighting);

    auto smooth_normals = new BooleanWidget("Smooth normals");
    smooth_normals->bindToSettings(this->settings, "appearance/mesh/smooth_normals", false);
    layout->addWidget(smooth_normals);

    auto smooth_theshold_angle = new FloatWidget("Smoothing threshold angle");
    smooth_theshold_angle->bindToSettings(this->settings,
                                          "appearance/mesh/smooth_theshold_angle",
                                          30.0);
    layout->addWidget(smooth_theshold_angle);

    auto coloring_mode_lbl = new QLabel("Coloring mode");
    layout->addWidget(coloring_mode_lbl);

    auto coloring_mode = new RadioOptionsWidget(
        { "By element type", "By elementary entity", "By physical group", "By mesh partition" });
    coloring_mode->bindToSettings(this->settings, "appearance/mesh/coloring_mode", 1);
    layout->addWidget(coloring_mode);

    auto colors = new SectionTitleWidget("Colors");
    layout->addWidget(colors);

    layout->addWidget(new LabelledColorWidget("Nodes"));
    layout->addWidget(new LabelledColorWidget("NodesSup"));
    layout->addWidget(new LabelledColorWidget("Lines"));
    layout->addWidget(new LabelledColorWidget("Triangles"));
    layout->addWidget(new LabelledColorWidget("Quadrangles"));
    layout->addWidget(new LabelledColorWidget("Tetrahedra"));
    layout->addWidget(new LabelledColorWidget("Hexahedra"));
    layout->addWidget(new LabelledColorWidget("Prisms"));
    layout->addWidget(new LabelledColorWidget("Pyramids"));
    layout->addWidget(new LabelledColorWidget("Trihedra"));
    layout->addWidget(new LabelledColorWidget("Normals"));
    layout->addWidget(new LabelledColorWidget("Zero"));
    layout->addWidget(new LabelledColorWidget("One"));
    layout->addWidget(new LabelledColorWidget("Two"));
    layout->addWidget(new LabelledColorWidget("Three"));
    layout->addWidget(new LabelledColorWidget("Four"));
    layout->addWidget(new LabelledColorWidget("Five"));
    layout->addWidget(new LabelledColorWidget("Six"));
    layout->addWidget(new LabelledColorWidget("Seven"));
    layout->addWidget(new LabelledColorWidget("Eight"));
    layout->addWidget(new LabelledColorWidget("Nine"));
    layout->addWidget(new LabelledColorWidget("Ten"));
    layout->addWidget(new LabelledColorWidget("Eleven"));
    layout->addWidget(new LabelledColorWidget("Twelve"));
    layout->addWidget(new LabelledColorWidget("Thirteen"));
    layout->addWidget(new LabelledColorWidget("Fourteen"));
    layout->addWidget(new LabelledColorWidget("Fifteen"));
    layout->addWidget(new LabelledColorWidget("Sixteen"));
    layout->addWidget(new LabelledColorWidget("Seventeen"));
    layout->addWidget(new LabelledColorWidget("Eighteen"));
    layout->addWidget(new LabelledColorWidget("Nineteen"));

    layout->addStretch();

    return pane;
}

QWidget *
SettingsDialog::createAppearanceGeometryPane()
{
    auto [pane, layout] = createPane();

    auto label = new QLabel("Geometry");
    layout->addWidget(label);

    auto pnt_appear = new SectionTitleWidget("Points");
    layout->addWidget(pnt_appear);

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(this->settings, "appearance/geo/point_display", 0);
    layout->addWidget(point_display);

    auto point_size = new FloatWidget("Point size");
    point_size->bindToSettings(this->settings, "appearance/geo/point_size", 4.0);
    layout->addWidget(point_size);

    auto selected_point_size = new FloatWidget("Selected point size");
    selected_point_size->bindToSettings(this->settings, "appearance/geo/selected_point_size", 6.0);
    layout->addWidget(selected_point_size);

    auto curve_appear = new SectionTitleWidget("Curves");
    layout->addWidget(curve_appear);

    auto curve_display = new RadioOptionsWidget({ "Color segment", "3D cylinder" });
    curve_display->bindToSettings(this->settings, "appearance/geo/curve_display", 0);
    layout->addWidget(curve_display);

    auto curve_width = new FloatWidget("Curve width");
    curve_width->bindToSettings(this->settings, "appearance/geo/curve_width", 2.0);
    layout->addWidget(curve_width);

    auto selected_curve_width = new FloatWidget("Selected curve width");
    selected_curve_width->bindToSettings(this->settings,
                                         "appearance/geo/selected_curve_width",
                                         3.0);
    layout->addWidget(selected_curve_width);

    auto curve_subdivs = new IntegerWidget("Curve subdivisions");
    curve_subdivs->bindToSettings(this->settings, "appearance/geo/curve_subdivs", 40);
    layout->addWidget(curve_subdivs);

    auto surface_appear = new SectionTitleWidget("Surfaces");
    layout->addWidget(surface_appear);

    auto surface_display = new RadioOptionsWidget({ "Cross", "Wireframe", "Solid" });
    surface_display->bindToSettings(this->settings, "appearance/geo/surface_display", 0);
    layout->addWidget(surface_display);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addWidget(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(this->settings, "appearance/geo/enable_lighting", true);
    layout->addWidget(enable_lighting);

    auto two_side_lighting = new BooleanWidget("Two side lighting");
    two_side_lighting->bindToSettings(this->settings, "appearance/geo/two_side_lighting", true);
    layout->addWidget(two_side_lighting);

    auto colors = new SectionTitleWidget("Colors");
    layout->addWidget(colors);

    auto clr_point = new LabelledColorWidget("Points");
    layout->addWidget(clr_point);

    layout->addWidget(new LabelledColorWidget("Curves"));
    layout->addWidget(new LabelledColorWidget("Surfaces"));
    layout->addWidget(new LabelledColorWidget("Volumes"));
    layout->addWidget(new LabelledColorWidget("Selection"));
    layout->addWidget(new LabelledColorWidget("Highlight 0"));
    layout->addWidget(new LabelledColorWidget("Highlight 1"));
    layout->addWidget(new LabelledColorWidget("Highlight 2"));
    layout->addWidget(new LabelledColorWidget("Tangents"));
    layout->addWidget(new LabelledColorWidget("Normals"));
    layout->addWidget(new LabelledColorWidget("Projections"));

    layout->addStretch();

    return pane;
}

QWidget *
SettingsDialog::createOpenCASCADEPane()
{
    auto [pane, layout] = createPane();

    auto label = new QLabel("OpenCASCADE");
    layout->addWidget(label);

    auto lbl = new QLabel("Model healing options");

    auto remove_degenerates = new BooleanWidget("Remove degenerated edge and face");
    remove_degenerates->bindToSettings(this->settings, "open_cascasde/remove_degenerates", false);
    layout->addWidget(remove_degenerates);

    auto remove_small_edges = new BooleanWidget("Remove small edges");
    remove_small_edges->bindToSettings(this->settings, "open_cascasde/remove_small_edges", false);
    layout->addWidget(remove_small_edges);

    auto remove_small_faces = new BooleanWidget("Remove small faces");
    remove_small_faces->bindToSettings(this->settings, "open_cascasde/remove_small_faces", false);
    layout->addWidget(remove_small_faces);

    auto sew_faces = new BooleanWidget("Sew faces");
    sew_faces->bindToSettings(this->settings, "open_cascasde/sew_faces", false);
    layout->addWidget(sew_faces);

    auto fix_shells = new BooleanWidget("Fix shells and make solid");
    fix_shells->bindToSettings(this->settings, "open_cascasde/fix_shells", false);
    layout->addWidget(fix_shells);

    auto global_model_scaling = new FloatWidget("Global model scaling");
    global_model_scaling->bindToSettings(this->settings, "open_cascasde/global_model_scaling", 1.);
    layout->addWidget(global_model_scaling);

    layout->addStretch();

    return pane;
}

void
SettingsDialog::onCategorySelected()
{
    auto selected = this->categories->selectedItems();
    if (selected.length() > 0) {
        const int column = 0;
        auto first = selected.first();
        auto pane_index = first->data(column, Qt::UserRole).value<int>();
        this->pane->setCurrentIndex(pane_index);
    }
}
