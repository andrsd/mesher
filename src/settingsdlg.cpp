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
#include <QFormLayout>
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
    setMinimumSize(600, 700);

    this->layout = new QHBoxLayout();

    this->categories = new QTreeWidget();
    this->categories->setHeaderHidden(true);
    this->categories->setFixedWidth(150);

    this->pane = new QStackedWidget();
    this->pane->setContentsMargins(10, 0, 10, 0);

    this->layout->addWidget(this->categories);

    auto rlayout = new QVBoxLayout();

    this->pane_title = new QLabel();
    this->pane_title->setFixedHeight(30);
    this->pane_title->setStyleSheet("QLabel { "
                                    "  font-weight: bold; "
                                    "  background-color: #ddd;"
                                    "  padding-left: 10;"
                                    "}");

    rlayout->addWidget(this->pane_title);

    rlayout->addWidget(this->pane);

    this->layout->addLayout(rlayout);

    setLayout(this->layout);

    createPanes();

    this->categories->expandAll();

    connect(this->categories,
            &QTreeWidget::itemSelectionChanged,
            this,
            &SettingsDialog::onCategorySelected);

    this->categories->topLevelItem(0)->setSelected(true);
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

std::tuple<QWidget *, QFormLayout *>
SettingsDialog::createPane()
{
    auto layout = new QFormLayout();
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
    layout->addRow("Rotation style", rotation);

    auto rot_about_com = new BooleanWidget("Rotate about pseudo center of mass");
    rot_about_com->bindToSettings(this->settings, "general/rotate_about_pseudo_com", true);
    layout->addWidget(rot_about_com);

    auto invert_zoom_direction = new BooleanWidget("Invert zoom direction");
    invert_zoom_direction->bindToSettings(this->settings, "general/invert_zoom_direction", false);
    layout->addWidget(invert_zoom_direction);

    auto geom_lbl = new SectionTitleWidget("Geometry");
    layout->addRow(geom_lbl);

    auto geometry_tolerance = new FloatWidget("");
    geometry_tolerance->bindToSettings(this->settings, "general/geometry_tolerance", 1e-8);
    layout->addRow("Geometry tolerance", geometry_tolerance);

    auto remove_duplicates = new BooleanWidget("Remove duplicate entities in GEO model transforms");
    remove_duplicates->bindToSettings(this->settings, "general/remove_geo_dups", true);
    layout->addWidget(remove_duplicates);

    auto hilight_orphans = new BooleanWidget("Highlight orphan entities");
    hilight_orphans->bindToSettings(this->settings, "general/hilight_orphans", true);
    layout->addWidget(hilight_orphans);

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

    auto elem_shrink_fact = new FloatWidget("");
    elem_shrink_fact->bindToSettings(this->settings, "appearance/mesh/element_shrink_fact", 1.0);
    QLabel * w1 = new QLabel("Element shrinking factor");
    w1->setWordWrap(true);
    layout->addRow(w1, elem_shrink_fact);

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(this->settings, "appearance/mesh/point_display", 0);
    layout->addRow("Points as", point_display);

    auto point_size = new FloatWidget("");
    point_size->bindToSettings(this->settings, "appearance/mesh/point_size", 4.0);
    layout->addRow("Point size", point_size);

    auto line_width = new FloatWidget("");
    line_width->bindToSettings(this->settings, "appearance/mesh/line_width", 1.0);
    layout->addRow("Line width", line_width);

    auto high_order_elem_subdivs = new IntegerWidget("");
    high_order_elem_subdivs->bindToSettings(this->settings,
                                            "appearance/mesh/high_order_elem_subdivs",
                                            2);

    QLabel * w = new QLabel("High-order elements subdivisions");
    w->setWordWrap(true);
    layout->addRow(w, high_order_elem_subdivs);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addRow(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(this->settings, "appearance/mesh/enable_lighting", true);
    layout->addWidget(enable_lighting);

    auto edge_lighting = new RadioOptionsWidget({ "No", "Surface", "Volume and surface" });
    edge_lighting->bindToSettings(this->settings, "appearance/mesh/edge_lighting", 2);
    layout->addRow("Edge lighting", edge_lighting);

    auto two_side_lighting = new BooleanWidget("Two-side lighting");
    two_side_lighting->bindToSettings(this->settings, "appearance/mesh/two_side_lighting", true);
    layout->addWidget(two_side_lighting);

    auto smooth_normals = new BooleanWidget("Smooth normals");
    smooth_normals->bindToSettings(this->settings, "appearance/mesh/smooth_normals", false);
    layout->addWidget(smooth_normals);

    auto smooth_theshold_angle = new FloatWidget("");
    smooth_theshold_angle->bindToSettings(this->settings,
                                          "appearance/mesh/smooth_theshold_angle",
                                          30.0);
    QLabel * w2 = new QLabel("Smoothing threshold angle");
    w2->setWordWrap(true);
    layout->addRow(w2, smooth_theshold_angle);

    auto coloring_mode = new RadioOptionsWidget(
        { "By element type", "By elementary entity", "By physical group", "By mesh partition" });
    coloring_mode->bindToSettings(this->settings, "appearance/mesh/coloring_mode", 1);
    layout->addRow("Coloring mode", coloring_mode);

    auto colors = new SectionTitleWidget("Colors");
    layout->addRow(colors);

    auto color_layout = new QVBoxLayout();

    auto clr_nodes = new LabelledColorWidget("Nodes");
    clr_nodes->bindToSettings(this->settings, "appearance/mesh/clr_nodes", QColor(0, 0, 255));
    color_layout->addWidget(clr_nodes);

    auto clr_nodes_sup = new LabelledColorWidget("NodesSup");
    clr_nodes_sup->bindToSettings(this->settings,
                                  "appearance/mesh/clr_nodes_sup",
                                  QColor(255, 0, 255));
    color_layout->addWidget(clr_nodes_sup);

    auto clr_lines = new LabelledColorWidget("Lines");
    clr_lines->bindToSettings(this->settings, "appearance/mesh/clr_lines", QColor(0, 0, 0));
    color_layout->addWidget(clr_lines);

    auto clr_tris = new LabelledColorWidget("Triangles");
    clr_tris->bindToSettings(this->settings, "appearance/mesh/clr_tris", QColor(160, 150, 255));
    color_layout->addWidget(clr_tris);

    auto clr_quads = new LabelledColorWidget("Quadrangles");
    clr_quads->bindToSettings(this->settings, "appearance/mesh/clr_quads", QColor(130, 120, 225));
    color_layout->addWidget(clr_quads);

    auto clr_tets = new LabelledColorWidget("Tetrahedra");
    clr_tets->bindToSettings(this->settings, "appearance/mesh/clr_tets", QColor(160, 150, 255));
    color_layout->addWidget(clr_tets);

    auto clr_hexs = new LabelledColorWidget("Hexahedra");
    clr_hexs->bindToSettings(this->settings, "appearance/mesh/clr_hexs", QColor(130, 120, 225));
    color_layout->addWidget(clr_hexs);

    auto clr_prisms = new LabelledColorWidget("Prisms");
    clr_prisms->bindToSettings(this->settings, "appearance/mesh/clr_prisms", QColor(232, 210, 23));
    color_layout->addWidget(clr_prisms);

    auto clr_pyramids = new LabelledColorWidget("Pyramids");
    clr_pyramids->bindToSettings(this->settings,
                                 "appearance/mesh/clr_pyramids",
                                 QColor(217, 113, 38));
    color_layout->addWidget(clr_pyramids);

    auto clr_trihedra = new LabelledColorWidget("Trihedra");
    clr_trihedra->bindToSettings(this->settings,
                                 "appearance/mesh/clr_trihedra",
                                 QColor(20, 255, 0));
    color_layout->addWidget(clr_trihedra);

    auto clr_tangents = new LabelledColorWidget("Tangents");
    clr_tangents->bindToSettings(this->settings,
                                 "appearance/mesh/clr_tangents",
                                 QColor(255, 255, 0));
    color_layout->addWidget(clr_tangents);

    auto clr_normal = new LabelledColorWidget("Normals");
    clr_normal->bindToSettings(this->settings, "appearance/mesh/clr_normal", QColor(255, 0, 0));
    color_layout->addWidget(clr_normal);

    auto clr_zero = new LabelledColorWidget("Zero");
    clr_zero->bindToSettings(this->settings, "appearance/mesh/clr_zero", QColor(255, 120, 0));
    color_layout->addWidget(clr_zero);

    auto clr_one = new LabelledColorWidget("One");
    clr_one->bindToSettings(this->settings, "appearance/mesh/clr_one", QColor(0, 255, 132));
    color_layout->addWidget(clr_one);

    auto clr_two = new LabelledColorWidget("Two");
    clr_two->bindToSettings(this->settings, "appearance/mesh/clr_two", QColor(255, 160, 0));
    color_layout->addWidget(clr_two);

    auto clr_three = new LabelledColorWidget("Three");
    clr_three->bindToSettings(this->settings, "appearance/mesh/clr_three", QColor(0, 255, 192));
    color_layout->addWidget(clr_three);

    auto clr_four = new LabelledColorWidget("Four");
    clr_four->bindToSettings(this->settings, "appearance/mesh/clr_four", QColor(255, 200, 0));
    color_layout->addWidget(clr_four);

    auto clr_five = new LabelledColorWidget("Five");
    clr_five->bindToSettings(this->settings, "appearance/mesh/clr_five", QColor(0, 216, 255));
    color_layout->addWidget(clr_five);

    auto clr_six = new LabelledColorWidget("Six");
    clr_six->bindToSettings(this->settings, "appearance/mesh/clr_six", QColor(255, 240, 0));
    color_layout->addWidget(clr_six);

    auto clr_seven = new LabelledColorWidget("Seven");
    clr_seven->bindToSettings(this->settings, "appearance/mesh/clr_seven", QColor(0, 176, 255));
    color_layout->addWidget(clr_seven);

    auto clr_eight = new LabelledColorWidget("Eight");
    clr_eight->bindToSettings(this->settings, "appearance/mesh/clr_eight", QColor(228, 255, 0));
    color_layout->addWidget(clr_eight);

    auto clr_nine = new LabelledColorWidget("Nine");
    clr_nine->bindToSettings(this->settings, "appearance/mesh/clr_nine", QColor(0, 116, 255));
    color_layout->addWidget(clr_nine);

    auto clr_ten = new LabelledColorWidget("Ten");
    clr_ten->bindToSettings(this->settings, "appearance/mesh/clr_ten", QColor(188, 255, 0));
    color_layout->addWidget(clr_ten);

    auto clr_eleven = new LabelledColorWidget("Eleven");
    clr_eleven->bindToSettings(this->settings, "appearance/mesh/clr_eleven", QColor(0, 76, 255));
    color_layout->addWidget(clr_eleven);

    auto clr_twelve = new LabelledColorWidget("Twelve");
    clr_twelve->bindToSettings(this->settings, "appearance/mesh/clr_twelve", QColor(148, 255, 0));
    color_layout->addWidget(clr_twelve);

    auto clr_thirteen = new LabelledColorWidget("Thirteen");
    clr_thirteen->bindToSettings(this->settings,
                                 "appearance/mesh/clr_thirteen",
                                 QColor(24, 0, 255));
    color_layout->addWidget(clr_thirteen);

    auto clr_fourteen = new LabelledColorWidget("Fourteen");
    clr_fourteen->bindToSettings(this->settings,
                                 "appearance/mesh/clr_fourteen",
                                 QColor(108, 255, 0));
    color_layout->addWidget(clr_fourteen);

    auto clr_fifteen = new LabelledColorWidget("Fifteen");
    clr_fifteen->bindToSettings(this->settings, "appearance/mesh/clr_fifteen", QColor(84, 0, 255));
    color_layout->addWidget(clr_fifteen);

    auto clr_sixteen = new LabelledColorWidget("Sixteen");
    clr_sixteen->bindToSettings(this->settings, "appearance/mesh/clr_sixteen", QColor(68, 255, 0));
    color_layout->addWidget(clr_sixteen);

    auto clr_seventeen = new LabelledColorWidget("Seventeen");
    clr_seventeen->bindToSettings(this->settings,
                                  "appearance/mesh/clr_seventeen",
                                  QColor(104, 0, 255));
    color_layout->addWidget(clr_seventeen);

    auto clr_eighteen = new LabelledColorWidget("Eighteen");
    clr_eighteen->bindToSettings(this->settings,
                                 "appearance/mesh/clr_eighteen",
                                 QColor(0, 255, 52));
    color_layout->addWidget(clr_eighteen);

    auto clr_nineteen = new LabelledColorWidget("Nineteen");
    clr_nineteen->bindToSettings(this->settings,
                                 "appearance/mesh/clr_nineteen",
                                 QColor(184, 0, 255));
    color_layout->addWidget(clr_nineteen);

    layout->addRow(color_layout);

    return pane;
}

QWidget *
SettingsDialog::createAppearanceGeometryPane()
{
    auto [pane, layout] = createPane();

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(this->settings, "appearance/geo/point_display", 0);
    layout->addRow("Display as", point_display);

    auto point_size = new FloatWidget("");
    point_size->bindToSettings(this->settings, "appearance/geo/point_size", 4.0);
    layout->addRow("Point size", point_size);

    auto selected_point_size = new FloatWidget("");
    selected_point_size->bindToSettings(this->settings, "appearance/geo/selected_point_size", 6.0);
    layout->addRow("Selected point size", selected_point_size);

    auto curve_appear = new SectionTitleWidget("Curves");
    layout->addRow(curve_appear);

    auto curve_display = new RadioOptionsWidget({ "Color segment", "3D cylinder" });
    curve_display->bindToSettings(this->settings, "appearance/geo/curve_display", 0);
    layout->addRow("Display as", curve_display);

    auto curve_width = new FloatWidget("");
    curve_width->bindToSettings(this->settings, "appearance/geo/curve_width", 2.0);
    layout->addRow("Curve width", curve_width);

    auto selected_curve_width = new FloatWidget("");
    selected_curve_width->bindToSettings(this->settings,
                                         "appearance/geo/selected_curve_width",
                                         3.0);
    layout->addRow("Selected curve width", selected_curve_width);

    auto curve_subdivs = new IntegerWidget("");
    curve_subdivs->bindToSettings(this->settings, "appearance/geo/curve_subdivs", 40);
    layout->addRow("Curve subdivisions", curve_subdivs);

    auto surface_appear = new SectionTitleWidget("Surfaces");
    layout->addRow(surface_appear);

    auto surface_display = new RadioOptionsWidget({ "Cross", "Wireframe", "Solid" });
    surface_display->bindToSettings(this->settings, "appearance/geo/surface_display", 0);
    layout->addRow("Display as", surface_display);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addRow(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(this->settings, "appearance/geo/enable_lighting", true);
    layout->addRow("", enable_lighting);

    auto two_side_lighting = new BooleanWidget("Two side lighting");
    two_side_lighting->bindToSettings(this->settings, "appearance/geo/two_side_lighting", true);
    layout->addRow("", two_side_lighting);

    auto colors = new SectionTitleWidget("Colors");
    layout->addRow(colors);

    auto color_layout = new QVBoxLayout();

    auto clr_point = new LabelledColorWidget("Points");
    clr_point->bindToSettings(this->settings, "appearance/geo/clr_points", QColor(90, 90, 90));
    color_layout->addWidget(clr_point);

    auto clr_surfaces = new LabelledColorWidget("Surfaces");
    clr_surfaces->bindToSettings(this->settings, "appearance/geo/clr_surfaces", QColor(0, 0, 255));
    color_layout->addWidget(clr_surfaces);

    auto clr_curves = new LabelledColorWidget("Curves");
    clr_curves->bindToSettings(this->settings, "appearance/geo/clr_curves", QColor(128, 128, 128));
    color_layout->addWidget(clr_curves);

    auto clr_volumes = new LabelledColorWidget("Volumes");
    clr_volumes->bindToSettings(this->settings, "appearance/geo/clr_volumes", QColor(255, 255, 0));
    color_layout->addWidget(clr_volumes);

    auto clr_selection = new LabelledColorWidget("Selection");
    clr_selection->bindToSettings(this->settings,
                                  "appearance/geo/clr_selection",
                                  QColor(255, 0, 0));
    color_layout->addWidget(clr_selection);

    auto clr_hilight0 = new LabelledColorWidget("Highlight 0");
    clr_hilight0->bindToSettings(this->settings, "appearance/geo/clr_hilight0", QColor(255, 0, 0));
    color_layout->addWidget(clr_hilight0);

    auto clr_hilight1 = new LabelledColorWidget("Highlight 1");
    clr_hilight1->bindToSettings(this->settings,
                                 "appearance/geo/clr_hilight1",
                                 QColor(255, 150, 0));
    color_layout->addWidget(clr_hilight1);

    auto clr_hilight2 = new LabelledColorWidget("Highlight 2");
    clr_hilight2->bindToSettings(this->settings,
                                 "appearance/geo/clr_hilight2",
                                 QColor(255, 255, 0));
    color_layout->addWidget(clr_hilight2);

    auto clr_tangents = new LabelledColorWidget("Tangents");
    clr_tangents->bindToSettings(this->settings,
                                 "appearance/geo/clr_tangents",
                                 QColor(255, 255, 0));
    color_layout->addWidget(clr_tangents);

    auto clr_normals = new LabelledColorWidget("Normals");
    clr_normals->bindToSettings(this->settings, "appearance/geo/clr_normals", QColor(255, 0, 0));
    color_layout->addWidget(clr_normals);

    auto clr_projects = new LabelledColorWidget("Projections");
    clr_projects->bindToSettings(this->settings, "appearance/geo/clr_projects", QColor(0, 255, 0));
    color_layout->addWidget(clr_projects);

    layout->addRow(color_layout);

    return pane;
}

QWidget *
SettingsDialog::createOpenCASCADEPane()
{
    auto [pane, layout] = createPane();

    auto vert_layout = new QVBoxLayout();

    auto lbl = new QLabel("Model healing options");
    vert_layout->addWidget(lbl);

    auto remove_degenerates = new BooleanWidget("Remove degenerated edge and face");
    remove_degenerates->bindToSettings(this->settings, "open_cascasde/remove_degenerates", false);
    vert_layout->addWidget(remove_degenerates);

    auto remove_small_edges = new BooleanWidget("Remove small edges");
    remove_small_edges->bindToSettings(this->settings, "open_cascasde/remove_small_edges", false);
    vert_layout->addWidget(remove_small_edges);

    auto remove_small_faces = new BooleanWidget("Remove small faces");
    remove_small_faces->bindToSettings(this->settings, "open_cascasde/remove_small_faces", false);
    vert_layout->addWidget(remove_small_faces);

    auto sew_faces = new BooleanWidget("Sew faces");
    sew_faces->bindToSettings(this->settings, "open_cascasde/sew_faces", false);
    vert_layout->addWidget(sew_faces);

    auto fix_shells = new BooleanWidget("Fix shells and make solid");
    fix_shells->bindToSettings(this->settings, "open_cascasde/fix_shells", false);
    vert_layout->addWidget(fix_shells);

    auto global_model_scaling = new FloatWidget("");
    global_model_scaling->bindToSettings(this->settings, "open_cascasde/global_model_scaling", 1.);

    auto horz_layout = new QHBoxLayout();
    auto lbl2 = new QLabel("Global model scaling");
    horz_layout->addWidget(lbl2);
    horz_layout->addWidget(global_model_scaling);
    horz_layout->addStretch();
    vert_layout->addLayout(horz_layout);

    layout->addRow(vert_layout);

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
        this->pane_title->setText(first->text(column));
    }
}
