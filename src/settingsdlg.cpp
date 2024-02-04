#include "settingsdlg.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QSettings>
#include <QScrollArea>
#include <QFormLayout>
#include "common/radiooptionswidget.h"
#include "common/floatwidget.h"
#include "common/integerwidget.h"
#include "common/booleanwidget.h"
#include "common/labelledcolorwidget.h"
#include "common/sectiontitlewidget.h"
#include "common/pointwidget.h"
#include "mainwindow.h"

SettingsDialog::SettingsDialog(QWidget * parent) : QDialog(parent)
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
SettingsDialog::addPane(QWidget * pane_widget, const QString & name, QTreeWidgetItem * parent)
{
    auto idx = this->pane->addWidget(pane_widget);
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
    auto form_layout = new QFormLayout();
    form_layout->setContentsMargins(0, 10, 0, 0);

    auto pane_widget = new QWidget();
    pane_widget->setLayout(form_layout);

    auto area = new QScrollArea();
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setWidget(pane_widget);

    return { area, form_layout };
}

QWidget *
SettingsDialog::createGeneralPane()
{
    auto [pane, layout] = createPane();

    auto settings = MainWindow::getSettings();

    auto show_bounding_boxes = new BooleanWidget("Show bounding boxes");
    show_bounding_boxes->bindToSettings(settings, "general/show_bounding_boxes", false);
    connect(show_bounding_boxes, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(show_bounding_boxes);

    auto simple_model_user_inter =
        new BooleanWidget("Draw simplified model during user interaction");
    simple_model_user_inter->bindToSettings(settings,
                                            "general/draw_simple_model_on_interaction",
                                            false);
    connect(simple_model_user_inter, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(simple_model_user_inter);

    auto double_buffering = new BooleanWidget("Enable double buffering");
    double_buffering->bindToSettings(settings, "general/double_buffering", true);
    connect(double_buffering, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(double_buffering);

    auto anti_aliasing = new BooleanWidget("Enable anti-aliasing");
    anti_aliasing->bindToSettings(settings, "general/anti_aliasng", false);
    connect(anti_aliasing, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(anti_aliasing);

    auto rot_lbl = new SectionTitleWidget("Rotation");
    layout->addRow(rot_lbl);

    auto rotation = new RadioOptionsWidget({ "Trackball rotation", "Euler angles" });
    rotation->bindToSettings(settings, "general/rotation", 0);
    connect(rotation, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Rotation style", rotation);

    auto rot_about_com = new BooleanWidget("Rotate about pseudo center of mass");
    rot_about_com->bindToSettings(settings, "general/rotate_about_pseudo_com", true);
    connect(rot_about_com, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(rot_about_com);

    auto zoom_lbl = new SectionTitleWidget("Zoom");
    layout->addRow(zoom_lbl);

    auto invert_zoom_direction = new BooleanWidget("Invert zoom direction");
    invert_zoom_direction->bindToSettings(settings, "general/invert_zoom_direction", false);
    connect(invert_zoom_direction, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(invert_zoom_direction);

    auto geom_lbl = new SectionTitleWidget("Geometry");
    layout->addRow(geom_lbl);

    auto geometry_tolerance = new FloatWidget("");
    geometry_tolerance->bindToSettings(settings, "general/geometry_tolerance", 1e-8);
    connect(geometry_tolerance, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Geometry tolerance", geometry_tolerance);

    auto remove_duplicates = new BooleanWidget("Remove duplicate entities in GEO model transforms");
    remove_duplicates->bindToSettings(settings, "general/remove_geo_dups", true);
    connect(remove_duplicates, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(remove_duplicates);

    auto hilight_orphans = new BooleanWidget("Highlight orphan entities");
    hilight_orphans->bindToSettings(settings, "general/hilight_orphans", false);
    connect(hilight_orphans, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(hilight_orphans);

    return pane;
}

QWidget *
SettingsDialog::createAppearancePane()
{
    auto [pane, layout] = createPane();

    auto settings = MainWindow::getSettings();

    auto z_clip_factor = new FloatWidget("");
    z_clip_factor->bindToSettings(settings, "appearance/z_clip_factor", 5.0);
    connect(z_clip_factor, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Z-clipping distance factor", z_clip_factor);

    auto poly_offset_factor = new FloatWidget("");
    poly_offset_factor->bindToSettings(settings, "appearance/poly_offset_factor", 0.5);
    connect(poly_offset_factor, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Polygon offset factor", poly_offset_factor);

    auto apply_poly_offset = new BooleanWidget("Apply polygon offset");
    apply_poly_offset->bindToSettings(settings, "appearance/apply_poly_offset", false);
    connect(apply_poly_offset, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("", apply_poly_offset);

    auto lbl1 = new SectionTitleWidget("");
    layout->addRow(lbl1);

    auto quadric_subdivs = new IntegerWidget("");
    quadric_subdivs->bindToSettings(settings, "appearance/quadric_subdivs", 6);
    connect(quadric_subdivs, &IntegerWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Quadric subdivisions", quadric_subdivs);

    auto point_size = new FloatWidget("");
    point_size->bindToSettings(settings, "appearance/point_size", 3.0);
    connect(point_size, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Point size", point_size);

    auto line_width = new FloatWidget("");
    line_width->bindToSettings(settings, "appearance/line_width", 1.0);
    connect(line_width, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Line width", line_width);

    auto lbl2 = new SectionTitleWidget("");
    layout->addRow(lbl2);

    auto font_engine = new RadioOptionsWidget({ "Native", "Cairo", "String texture" });
    font_engine->enableOption(1, false);
    font_engine->bindToSettings(settings, "appearance/font_engine", 0);
    connect(font_engine, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Font rending engine", font_engine);

    auto lbl3 = new SectionTitleWidget("");
    layout->addRow(lbl3);

    auto light_position = new PointWidget();
    light_position->bindToSettings(settings,
                                   "appearance/light_position",
                                   QVector3D(0.65, 0.65, 1.00));
    connect(light_position, &PointWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Light position", light_position);

    auto light_pos_divisor = new FloatWidget("");
    light_pos_divisor->bindToSettings(settings, "appearance/light_pos_divisor", 0.);
    connect(light_pos_divisor, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Light position divisor", light_pos_divisor);

    auto lbl4 = new SectionTitleWidget("");
    layout->addRow(lbl4);

    auto material_shininess = new FloatWidget("");
    material_shininess->bindToSettings(settings, "appearance/material_shininess", 0.4);
    connect(material_shininess, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Material shininess", material_shininess);

    auto material_exponent = new FloatWidget("");
    material_exponent->bindToSettings(settings, "appearance/material_exponent", 40.);
    connect(material_exponent, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Material exponent", material_exponent);

    auto lbl5 = new SectionTitleWidget("");
    layout->addRow(lbl5);

    auto bkgnd_gradient = new RadioOptionsWidget({ "None", "Vertical", "Horizontal", "Radial" });
    bkgnd_gradient->bindToSettings(settings, "appearance/bkgnd_gradient", 1);
    connect(bkgnd_gradient, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Background gradient", bkgnd_gradient);

    auto colors = new SectionTitleWidget("Colors");
    layout->addRow(colors);

    auto color_layout = new QVBoxLayout();

    auto clr_bkgnd = new LabelledColorWidget("Background");
    clr_bkgnd->bindToSettings(settings, "appearance/clr_bkgnd", QColor(255, 255, 255));
    connect(clr_bkgnd, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_bkgnd);

    auto clr_bkgnd2 = new LabelledColorWidget("Background gradient");
    clr_bkgnd2->bindToSettings(settings, "appearance/clr_bkgnd2", QColor(208, 215, 255));
    connect(clr_bkgnd2, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_bkgnd2);

    auto clr_foregnd = new LabelledColorWidget("Foreground");
    clr_foregnd->bindToSettings(settings, "appearance/clr_foregnd", QColor(85, 85, 85));
    connect(clr_foregnd, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_foregnd);

    auto clr_text = new LabelledColorWidget("Text");
    clr_text->bindToSettings(settings, "appearance/clr_text", QColor(0, 0, 0));
    connect(clr_text, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_text);

    auto clr_axes = new LabelledColorWidget("Axes");
    clr_axes->bindToSettings(settings, "appearance/clr_axes", QColor(0, 0, 0));
    connect(clr_axes, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_axes);

    auto clr_ambient_light = new LabelledColorWidget("Ambient light");
    clr_ambient_light->bindToSettings(settings, "appearance/clr_ambient_light", QColor(25, 25, 25));
    connect(clr_ambient_light, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_ambient_light);

    auto clr_diffuse_light = new LabelledColorWidget("Diffuse light");
    clr_diffuse_light->bindToSettings(settings,
                                      "appearance/clr_diffuse_light",
                                      QColor(255, 255, 255));
    connect(clr_diffuse_light, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_diffuse_light);

    auto clr_specular_light = new LabelledColorWidget("Specular light");
    clr_specular_light->bindToSettings(settings,
                                       "appearance/clr_specular_light",
                                       QColor(255, 255, 255));
    connect(clr_specular_light, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_specular_light);

    layout->addRow(color_layout);

    return pane;
}

QWidget *
SettingsDialog::createAppearanceMeshPane()
{
    auto [pane, layout] = createPane();

    auto settings = MainWindow::getSettings();

    auto elem_shrink_fact = new FloatWidget("");
    elem_shrink_fact->bindToSettings(settings, "appearance/mesh/element_shrink_fact", 1.0);
    connect(elem_shrink_fact, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    auto * w1 = new QLabel("Element shrinking factor");
    w1->setWordWrap(true);
    layout->addRow(w1, elem_shrink_fact);

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(settings, "appearance/mesh/point_display", 0);
    connect(point_display, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Points as", point_display);

    auto point_size = new FloatWidget("");
    point_size->bindToSettings(settings, "appearance/mesh/point_size", 4.0);
    connect(point_size, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Point size", point_size);

    auto line_width = new FloatWidget("");
    line_width->bindToSettings(settings, "appearance/mesh/line_width", 1.0);
    connect(line_width, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Line width", line_width);

    auto high_order_elem_subdivs = new IntegerWidget("");
    high_order_elem_subdivs->bindToSettings(settings, "appearance/mesh/high_order_elem_subdivs", 2);
    connect(high_order_elem_subdivs, &IntegerWidget::changed, this, &SettingsDialog::onChanged);

    auto * w = new QLabel("High-order elements subdivisions");
    w->setWordWrap(true);
    layout->addRow(w, high_order_elem_subdivs);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addRow(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(settings, "appearance/mesh/enable_lighting", true);
    connect(enable_lighting, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(enable_lighting);

    auto edge_lighting = new RadioOptionsWidget({ "No", "Surface", "Volume and surface" });
    edge_lighting->bindToSettings(settings, "appearance/mesh/edge_lighting", 2);
    connect(edge_lighting, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Edge lighting", edge_lighting);

    auto two_side_lighting = new BooleanWidget("Two-side lighting");
    two_side_lighting->bindToSettings(settings, "appearance/mesh/two_side_lighting", true);
    connect(two_side_lighting, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(two_side_lighting);

    auto smooth_lbl = new SectionTitleWidget("Smoothing");
    layout->addRow(smooth_lbl);

    auto smooth_normals = new BooleanWidget("Smooth normals");
    smooth_normals->bindToSettings(settings, "appearance/mesh/smooth_normals", false);
    connect(smooth_normals, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addWidget(smooth_normals);

    auto smooth_theshold_angle = new FloatWidget("");
    smooth_theshold_angle->bindToSettings(settings, "appearance/mesh/smooth_theshold_angle", 30.0);
    connect(smooth_theshold_angle, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    auto * w2 = new QLabel("Smoothing threshold angle");
    w2->setWordWrap(true);
    layout->addRow(w2, smooth_theshold_angle);

    auto color_mode_lbl = new SectionTitleWidget("Coloring mode");
    layout->addRow(color_mode_lbl);

    auto coloring_mode = new RadioOptionsWidget(
        { "By element type", "By elementary entity", "By physical group", "By mesh partition" });
    coloring_mode->bindToSettings(settings, "appearance/mesh/coloring_mode", 1);
    connect(coloring_mode, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Coloring mode", coloring_mode);

    auto colors = new SectionTitleWidget("Colors");
    layout->addRow(colors);

    auto color_layout = new QVBoxLayout();

    auto clr_nodes = new LabelledColorWidget("Nodes");
    clr_nodes->bindToSettings(settings, "appearance/mesh/clr_nodes", QColor(0, 0, 255));
    connect(clr_nodes, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_nodes);

    auto clr_nodes_sup = new LabelledColorWidget("NodesSup");
    clr_nodes_sup->bindToSettings(settings, "appearance/mesh/clr_nodes_sup", QColor(255, 0, 255));
    connect(clr_nodes_sup, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_nodes_sup);

    auto clr_lines = new LabelledColorWidget("Lines");
    clr_lines->bindToSettings(settings, "appearance/mesh/clr_lines", QColor(0, 0, 0));
    connect(clr_lines, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_lines);

    auto clr_tris = new LabelledColorWidget("Triangles");
    clr_tris->bindToSettings(settings, "appearance/mesh/clr_tris", QColor(160, 150, 255));
    connect(clr_tris, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_tris);

    auto clr_quads = new LabelledColorWidget("Quadrangles");
    clr_quads->bindToSettings(settings, "appearance/mesh/clr_quads", QColor(130, 120, 225));
    connect(clr_quads, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_quads);

    auto clr_tets = new LabelledColorWidget("Tetrahedra");
    clr_tets->bindToSettings(settings, "appearance/mesh/clr_tets", QColor(160, 150, 255));
    connect(clr_tets, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_tets);

    auto clr_hexs = new LabelledColorWidget("Hexahedra");
    clr_hexs->bindToSettings(settings, "appearance/mesh/clr_hexs", QColor(130, 120, 225));
    connect(clr_hexs, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_hexs);

    auto clr_prisms = new LabelledColorWidget("Prisms");
    clr_prisms->bindToSettings(settings, "appearance/mesh/clr_prisms", QColor(232, 210, 23));
    connect(clr_prisms, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_prisms);

    auto clr_pyramids = new LabelledColorWidget("Pyramids");
    clr_pyramids->bindToSettings(settings, "appearance/mesh/clr_pyramids", QColor(217, 113, 38));
    connect(clr_pyramids, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_pyramids);

    auto clr_trihedra = new LabelledColorWidget("Trihedra");
    clr_trihedra->bindToSettings(settings, "appearance/mesh/clr_trihedra", QColor(20, 255, 0));
    connect(clr_trihedra, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_trihedra);

    auto clr_tangents = new LabelledColorWidget("Tangents");
    clr_tangents->bindToSettings(settings, "appearance/mesh/clr_tangents", QColor(255, 255, 0));
    connect(clr_tangents, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_tangents);

    auto clr_normal = new LabelledColorWidget("Normals");
    clr_normal->bindToSettings(settings, "appearance/mesh/clr_normal", QColor(255, 0, 0));
    connect(clr_normal, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_normal);

    auto clr_zero = new LabelledColorWidget("Zero");
    clr_zero->bindToSettings(settings, "appearance/mesh/clr_zero", QColor(255, 120, 0));
    connect(clr_zero, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_zero);

    auto clr_one = new LabelledColorWidget("One");
    clr_one->bindToSettings(settings, "appearance/mesh/clr_one", QColor(0, 255, 132));
    connect(clr_one, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_one);

    auto clr_two = new LabelledColorWidget("Two");
    clr_two->bindToSettings(settings, "appearance/mesh/clr_two", QColor(255, 160, 0));
    connect(clr_two, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_two);

    auto clr_three = new LabelledColorWidget("Three");
    clr_three->bindToSettings(settings, "appearance/mesh/clr_three", QColor(0, 255, 192));
    connect(clr_three, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_three);

    auto clr_four = new LabelledColorWidget("Four");
    clr_four->bindToSettings(settings, "appearance/mesh/clr_four", QColor(255, 200, 0));
    connect(clr_four, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_four);

    auto clr_five = new LabelledColorWidget("Five");
    clr_five->bindToSettings(settings, "appearance/mesh/clr_five", QColor(0, 216, 255));
    connect(clr_five, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_five);

    auto clr_six = new LabelledColorWidget("Six");
    clr_six->bindToSettings(settings, "appearance/mesh/clr_six", QColor(255, 240, 0));
    connect(clr_six, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_six);

    auto clr_seven = new LabelledColorWidget("Seven");
    clr_seven->bindToSettings(settings, "appearance/mesh/clr_seven", QColor(0, 176, 255));
    connect(clr_seven, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_seven);

    auto clr_eight = new LabelledColorWidget("Eight");
    clr_eight->bindToSettings(settings, "appearance/mesh/clr_eight", QColor(228, 255, 0));
    connect(clr_eight, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_eight);

    auto clr_nine = new LabelledColorWidget("Nine");
    clr_nine->bindToSettings(settings, "appearance/mesh/clr_nine", QColor(0, 116, 255));
    connect(clr_nine, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_nine);

    auto clr_ten = new LabelledColorWidget("Ten");
    clr_ten->bindToSettings(settings, "appearance/mesh/clr_ten", QColor(188, 255, 0));
    connect(clr_ten, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_ten);

    auto clr_eleven = new LabelledColorWidget("Eleven");
    clr_eleven->bindToSettings(settings, "appearance/mesh/clr_eleven", QColor(0, 76, 255));
    connect(clr_eleven, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_eleven);

    auto clr_twelve = new LabelledColorWidget("Twelve");
    clr_twelve->bindToSettings(settings, "appearance/mesh/clr_twelve", QColor(148, 255, 0));
    connect(clr_twelve, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_twelve);

    auto clr_thirteen = new LabelledColorWidget("Thirteen");
    clr_thirteen->bindToSettings(settings, "appearance/mesh/clr_thirteen", QColor(24, 0, 255));
    connect(clr_thirteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_thirteen);

    auto clr_fourteen = new LabelledColorWidget("Fourteen");
    clr_fourteen->bindToSettings(settings, "appearance/mesh/clr_fourteen", QColor(108, 255, 0));
    connect(clr_fourteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_fourteen);

    auto clr_fifteen = new LabelledColorWidget("Fifteen");
    clr_fifteen->bindToSettings(settings, "appearance/mesh/clr_fifteen", QColor(84, 0, 255));
    connect(clr_fifteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_fifteen);

    auto clr_sixteen = new LabelledColorWidget("Sixteen");
    clr_sixteen->bindToSettings(settings, "appearance/mesh/clr_sixteen", QColor(68, 255, 0));
    connect(clr_sixteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_sixteen);

    auto clr_seventeen = new LabelledColorWidget("Seventeen");
    clr_seventeen->bindToSettings(settings, "appearance/mesh/clr_seventeen", QColor(104, 0, 255));
    connect(clr_seventeen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_seventeen);

    auto clr_eighteen = new LabelledColorWidget("Eighteen");
    clr_eighteen->bindToSettings(settings, "appearance/mesh/clr_eighteen", QColor(0, 255, 52));
    connect(clr_eighteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_eighteen);

    auto clr_nineteen = new LabelledColorWidget("Nineteen");
    clr_nineteen->bindToSettings(settings, "appearance/mesh/clr_nineteen", QColor(184, 0, 255));
    connect(clr_nineteen, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_nineteen);

    layout->addRow(color_layout);

    return pane;
}

QWidget *
SettingsDialog::createAppearanceGeometryPane()
{
    auto [pane, layout] = createPane();

    auto settings = MainWindow::getSettings();

    auto point_display = new RadioOptionsWidget({ "Color dot", "3D sphere" });
    point_display->bindToSettings(settings, "appearance/geo/point_display", 0);
    connect(point_display, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Display as", point_display);

    auto point_size = new FloatWidget("");
    point_size->bindToSettings(settings, "appearance/geo/point_size", 4.0);
    connect(point_size, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Point size", point_size);

    auto selected_point_size = new FloatWidget("");
    selected_point_size->bindToSettings(settings, "appearance/geo/selected_point_size", 6.0);
    connect(selected_point_size, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Selected point size", selected_point_size);

    auto curve_appear = new SectionTitleWidget("Curves");
    layout->addRow(curve_appear);

    auto curve_display = new RadioOptionsWidget({ "Color segment", "3D cylinder" });
    curve_display->bindToSettings(settings, "appearance/geo/curve_display", 0);
    connect(curve_display, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Display as", curve_display);

    auto curve_width = new FloatWidget("");
    curve_width->bindToSettings(settings, "appearance/geo/curve_width", 2.0);
    connect(curve_width, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Curve width", curve_width);

    auto selected_curve_width = new FloatWidget("");
    selected_curve_width->bindToSettings(settings, "appearance/geo/selected_curve_width", 3.0);
    connect(selected_curve_width, &FloatWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Selected curve width", selected_curve_width);

    auto curve_subdivs = new IntegerWidget("");
    curve_subdivs->bindToSettings(settings, "appearance/geo/curve_subdivs", 40);
    connect(curve_subdivs, &IntegerWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Curve subdivisions", curve_subdivs);

    auto surface_appear = new SectionTitleWidget("Surfaces");
    layout->addRow(surface_appear);

    auto surface_display = new RadioOptionsWidget({ "Cross", "Wireframe", "Solid" });
    surface_display->bindToSettings(settings, "appearance/geo/surface_display", 0);
    connect(surface_display, &RadioOptionsWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("Display as", surface_display);

    auto lighting = new SectionTitleWidget("Lighting");
    layout->addRow(lighting);

    auto enable_lighting = new BooleanWidget("Enable lighting");
    enable_lighting->bindToSettings(settings, "appearance/geo/enable_lighting", true);
    connect(enable_lighting, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("", enable_lighting);

    auto two_side_lighting = new BooleanWidget("Two side lighting");
    two_side_lighting->bindToSettings(settings, "appearance/geo/two_side_lighting", true);
    connect(two_side_lighting, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    layout->addRow("", two_side_lighting);

    auto colors = new SectionTitleWidget("Colors");
    layout->addRow(colors);

    auto color_layout = new QVBoxLayout();

    auto clr_point = new LabelledColorWidget("Points");
    clr_point->bindToSettings(settings, "appearance/geo/clr_points", QColor(90, 90, 90));
    connect(clr_point, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_point);

    auto clr_curves = new LabelledColorWidget("Curves");
    clr_curves->bindToSettings(settings, "appearance/geo/clr_curves", QColor(0, 0, 255));
    connect(clr_curves, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_curves);

    auto clr_surfaces = new LabelledColorWidget("Surfaces");
    clr_surfaces->bindToSettings(settings, "appearance/geo/clr_surfaces", QColor(128, 128, 128));
    connect(clr_surfaces, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_surfaces);

    auto clr_volumes = new LabelledColorWidget("Volumes");
    clr_volumes->bindToSettings(settings, "appearance/geo/clr_volumes", QColor(200, 200, 0));
    connect(clr_volumes, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_volumes);

    auto clr_selection = new LabelledColorWidget("Selection");
    clr_selection->bindToSettings(settings, "appearance/geo/clr_selection", QColor(255, 0, 0));
    connect(clr_selection, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_selection);

    auto clr_hilight0 = new LabelledColorWidget("Highlight 0");
    clr_hilight0->bindToSettings(settings, "appearance/geo/clr_hilight0", QColor(255, 0, 0));
    connect(clr_hilight0, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_hilight0);

    auto clr_hilight1 = new LabelledColorWidget("Highlight 1");
    clr_hilight1->bindToSettings(settings, "appearance/geo/clr_hilight1", QColor(255, 150, 0));
    connect(clr_hilight1, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_hilight1);

    auto clr_hilight2 = new LabelledColorWidget("Highlight 2");
    clr_hilight2->bindToSettings(settings, "appearance/geo/clr_hilight2", QColor(255, 255, 0));
    connect(clr_hilight2, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_hilight2);

    auto clr_tangents = new LabelledColorWidget("Tangents");
    clr_tangents->bindToSettings(settings, "appearance/geo/clr_tangents", QColor(255, 255, 0));
    connect(clr_tangents, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_tangents);

    auto clr_normals = new LabelledColorWidget("Normals");
    clr_normals->bindToSettings(settings, "appearance/geo/clr_normals", QColor(255, 0, 0));
    connect(clr_normals, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_normals);

    auto clr_projects = new LabelledColorWidget("Projections");
    clr_projects->bindToSettings(settings, "appearance/geo/clr_projects", QColor(0, 255, 0));
    connect(clr_projects, &LabelledColorWidget::changed, this, &SettingsDialog::onChanged);
    color_layout->addWidget(clr_projects);

    layout->addRow(color_layout);

    return pane;
}

QWidget *
SettingsDialog::createOpenCASCADEPane()
{
    auto [cat_pane, pane_layout] = createPane();

    auto settings = MainWindow::getSettings();

    auto vert_layout = new QVBoxLayout();

    auto lbl = new QLabel("Model healing options");

    auto remove_degenerates = new BooleanWidget("Remove degenerated edge and face");
    remove_degenerates->bindToSettings(settings, "open_cascasde/remove_degenerates", false);
    connect(remove_degenerates, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    vert_layout->addWidget(remove_degenerates);

    auto remove_small_edges = new BooleanWidget("Remove small edges");
    remove_small_edges->bindToSettings(settings, "open_cascasde/remove_small_edges", false);
    connect(remove_small_edges, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    vert_layout->addWidget(remove_small_edges);

    auto remove_small_faces = new BooleanWidget("Remove small faces");
    remove_small_faces->bindToSettings(settings, "open_cascasde/remove_small_faces", false);
    connect(remove_small_faces, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    vert_layout->addWidget(remove_small_faces);

    auto sew_faces = new BooleanWidget("Sew faces");
    sew_faces->bindToSettings(settings, "open_cascasde/sew_faces", false);
    connect(sew_faces, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    vert_layout->addWidget(sew_faces);

    auto fix_shells = new BooleanWidget("Fix shells and make solid");
    fix_shells->bindToSettings(settings, "open_cascasde/fix_shells", false);
    connect(fix_shells, &BooleanWidget::changed, this, &SettingsDialog::onChanged);
    vert_layout->addWidget(fix_shells);

    auto global_model_scaling = new FloatWidget("");
    global_model_scaling->bindToSettings(settings, "open_cascasde/global_model_scaling", 1.);
    connect(global_model_scaling, &FloatWidget::changed, this, &SettingsDialog::onChanged);

    pane_layout->addRow(lbl, vert_layout);

    pane_layout->addRow("Global model scaling", global_model_scaling);

    return cat_pane;
}

void
SettingsDialog::onCategorySelected()
{
    auto selected = this->categories->selectedItems();
    if (!selected.empty()) {
        const int column = 0;
        auto first = selected.first();
        auto pane_index = first->data(column, Qt::UserRole).value<int>();
        this->pane->setCurrentIndex(pane_index);
        this->pane_title->setText(first->text(column));
    }
}

void
SettingsDialog::onChanged()
{
    qDebug() << "SettingsDialog::onChanged()";
    emit changed();
}
