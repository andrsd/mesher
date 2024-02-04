#include "view.h"
#include "GModel.h"
#include "GVertex.h"
#include "GEdge.h"
#include "VertexArray.h"
#include "mainwindow.h"
#include <QSettings>

// TODO: move this into settings
QColor SELECTION_CLR = QColor(255, 173, 79);
QColor SELECTION_EDGE_CLR = QColor(179, 95, 0);
QColor HIGHLIGHT_CLR = QColor(255, 211, 79);

void
glColorQColor(const QColor & clr)
{
    glColor4f(clr.redF(), clr.greenF(), clr.blueF(), clr.alphaF());
}

View::DrawGVertex::DrawGVertex(View * view) : view(view) {}

void
View::DrawGVertex::operator()(GVertex * v)
{
    if (!v->getVisibility())
        return;
    if (v->geomType() == GEntity::BoundaryLayerPoint)
        return;

    auto settings = MainWindow::getSettings();
    auto point_display = settings->value("appearance/geo/point_display").toInt();
    auto point_size = settings->value("appearance/geo/point_size").toDouble();
    auto sel_point_size = settings->value("appearance/geo/selected_point_size").toDouble();
    auto show_points = settings->value("visibility/geo/points").toBool();
    auto show_point_labels = settings->value("visibility/geo/point_labels").toBool();
    auto hilight_orphans = settings->value("general/hilight_orphans").toBool();
    auto clr_selection = settings->value("appearance/geo/clr_selection").value<QColor>();
    auto clr_hilight0 = settings->value("appearance/geo/clr_hilight0").value<QColor>();
    auto clr_hilight1 = settings->value("appearance/geo/clr_hilight1").value<QColor>();
    auto clr_foregnd = settings->value("appearance/clr_foregnd").value<QColor>();
    auto clr_point = settings->value("appearance/geo/clr_points").value<QColor>();
    auto enable_lighting = settings->value("appearance/geo/enable_lighting").toBool();
    // FIXME: pull this from settings when font settings is implemented
    int font_size = 15;

    bool select = (this->view->render_mode == View::GMSH_SELECT && v->model() == GModel::current());
    if (select) {
        glPushName(0);
        glPushName(v->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    double ps = this->view->HIDPI(point_size);
    double sps = this->view->HIDPI(sel_point_size);

    if (v->getSelection() == HIGHLIGHT) {
        glPointSize((float) sps);
        glColorQColor(HIGHLIGHT_CLR);
    }
    else if (v->getSelection() == SELECTED) {
        glPointSize((float) sps);
        glColorQColor(clr_selection);
    }
    else {
        glPointSize((float) ps);
        if (v->useColor()) {
            auto col = v->getColor();
            glColor4ubv((GLubyte *) &col);
        }
        else
            glColorQColor(clr_point);
    }

    if (hilight_orphans) {
        if (v->isOrphan())
            glColorQColor(clr_hilight0);
        else if (v->numEdges() == 1)
            glColorQColor(clr_hilight1);
    }

    double x = v->x(), y = v->y(), z = v->z();
    this->view->transform(x, y, z);

    if (show_points || v->getSelection() > 1) {
        if (point_display > 0) {
            if (v->getSelection())
                this->view->drawSphere(sps, x, y, z, enable_lighting);
            else
                this->view->drawSphere(ps, x, y, z, enable_lighting);
        }
        else {
            glBegin(GL_POINTS);
            glVertex3d(x, y, z);
            glEnd();
        }
    }

    if (show_point_labels || v->getSelection() > 1) {
        double offset = (0.5 * ps + 0.1 * font_size) * this->view->pixel_equiv_x;
        glColorQColor(clr_foregnd);
        this->view->drawEntityLabel(v, x, y, z, offset);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}

//

View::DrawGEdge::DrawGEdge(View * view) : view(view) {}

void
View::DrawGEdge::operator()(GEdge * e)
{
    if (!e->getVisibility())
        return;
    if (e->geomType() == GEntity::DiscreteCurve)
        return;
    if (e->geomType() == GEntity::PartitionCurve)
        return;
    if (e->geomType() == GEntity::BoundaryLayerCurve)
        return;

    auto settings = MainWindow::getSettings();
    auto show_curves = settings->value("visibility/geo/curves").toBool();
    auto show_curve_labels = settings->value("visibility/geo/curve_labels").toBool();
    auto curve_type = settings->value("appearance/geo/curve_display").toInt();
    auto curve_width = settings->value("appearance/geo/curve_width").toInt();
    auto sel_curve_width = settings->value("appearance/geo/selected_curve_width").toInt();
    auto clr_curve = settings->value("appearance/geo/clr_curves").value<QColor>();
    auto clr_foregnd = settings->value("appearance/clr_foregnd").value<QColor>();
    auto clr_selection = settings->value("appearance/geo/clr_selection").value<QColor>();
    auto clr_hilight0 = settings->value("appearance/geo/clr_hilight0").value<QColor>();
    auto clr_hilight1 = settings->value("appearance/geo/clr_hilight1").value<QColor>();
    auto clr_tangents = settings->value("appearance/geo/clr_tangents").value<QColor>();
    auto enable_lighting = settings->value("appearance/geo/enable_lighting").toBool();
    auto hilight_orphans = settings->value("general/hilight_orphans").toBool();
    // FIXME: pull this from settings when font settings is implemented
    int font_size = 15;
    /// FIXME: this should be in user settings
    double geom_tangents = 0.;
    /// FIXME: this should be in user settings
    int vector_type = 0;

    bool select = (this->view->render_mode == View::GMSH_SELECT && e->model() == GModel::current());
    if (select) {
        glPushName(1);
        glPushName(e->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (e->getSelection() == HIGHLIGHT) {
        glLineWidth((float) sel_curve_width);
        glColorQColor(HIGHLIGHT_CLR);
    }
    else if (e->getSelection() == SELECTED) {
        glLineWidth((float) sel_curve_width);
        glColorQColor(clr_selection);
    }
    else {
        glLineWidth((float) curve_width);
        if (e->useColor()) {
            unsigned int col = e->getColor();
            glColor4ubv((GLubyte *) &col);
        }
        else
            glColorQColor(clr_curve);
    }

    if (hilight_orphans) {
        if (e->isOrphan())
            glColorQColor(clr_hilight0);
        else if (e->numFaces() == 1)
            glColorQColor(clr_hilight1);
    }

    Range<double> t_bounds = e->parBounds(0);
    double t_min = t_bounds.low();
    double t_max = t_bounds.high();

    if (show_curves || e->getSelection() > 1) {
        int N = e->minimumDrawSegments() + 1;
        if (curve_type > 0) {
            for (int i = 0; i < N - 1; i++) {
                double t1 = t_min + (double) i / (double) (N - 1) * (t_max - t_min);
                GPoint p1 = e->point(t1);
                double t2 = t_min + (double) (i + 1) / (double) (N - 1) * (t_max - t_min);
                GPoint p2 = e->point(t2);
                double x[2] = { p1.x(), p2.x() };
                double y[2] = { p1.y(), p2.y() };
                double z[2] = { p1.z(), p2.z() };
                this->view->transform(x[0], y[0], z[0]);
                this->view->transform(x[1], y[1], z[1]);
                this->view->drawCylinder(e->getSelection() ? sel_curve_width : curve_width,
                                         x,
                                         y,
                                         z,
                                         enable_lighting);
            }
        }
        else {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < N; i++) {
                double t = t_min + (double) i / (double) (N - 1) * (t_max - t_min);
                GPoint p = e->point(t);
                double x = p.x(), y = p.y(), z = p.z();
                this->view->transform(x, y, z);
                glVertex3d(x, y, z);
            }
            glEnd();
        }
    }

    if (show_curve_labels || e->getSelection() > 1) {
        GPoint p = e->point(t_min + 0.5 * (t_max - t_min));
        double offset = (0.5 * curve_width + 0.1 * font_size) * this->view->pixel_equiv_x;
        double x = p.x(), y = p.y(), z = p.z();
        this->view->transform(x, y, z);
        glColorQColor(clr_foregnd);
        this->view->drawEntityLabel(e, x, y, z, offset);
    }

    if (geom_tangents) {
        double t = t_min + 0.5 * (t_max - t_min);
        GPoint p = e->point(t);
        SVector3 der = e->firstDer(t);
        der.normalize();
        for (int i = 0; i < 3; i++)
            der[i] *= geom_tangents * this->view->pixel_equiv_x / this->view->s[i];
        glColorQColor(clr_tangents);
        double x = p.x(), y = p.y(), z = p.z();
        this->view->transform(x, y, z);
        this->view->transformOneForm(der[0], der[1], der[2]);
        this->view->drawVector(vector_type, 0, x, y, z, der[0], der[1], der[2], enable_lighting);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}

//

void
View::DrawGFace::_drawVertexArray(VertexArray * va,
                                  bool useNormalArray,
                                  int forceColor,
                                  const QColor & color)
{
    if (!va || !va->getNumVertices())
        return;
    glVertexPointer(3, GL_FLOAT, 0, va->getVertexArray());
    glEnableClientState(GL_VERTEX_ARRAY);
    if (useNormalArray) {
        glEnable(GL_LIGHTING);
        glNormalPointer(NORMAL_GLTYPE, 0, va->getNormalArray());
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    else {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if (forceColor) {
        glDisableClientState(GL_COLOR_ARRAY);
        glColorQColor(color);
    }
    else {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, va->getColorArray());
        glEnableClientState(GL_COLOR_ARRAY);
    }
    auto settings = MainWindow::getSettings();
    // FIXME: is this somewhere in user seetings in gmsh?
    int polygon_offset = 0;
    auto surface_type = settings->value("appearance/geo/surface_display").toInt();
    auto two_side_lighting = settings->value("appearance/geo/two_side_lighting").toBool();
    if (polygon_offset)
        glEnable(GL_POLYGON_OFFSET_FILL);
    if (surface_type > 1) {
        if (two_side_lighting)
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        else
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glDrawArrays(GL_TRIANGLES, 0, va->getNumVertices());
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

View::DrawGFace::DrawGFace(View * view) : view(view) {}

void
View::DrawGFace::operator()(GFace * f)
{
    if (!f->getVisibility())
        return;
    if (f->geomType() == GEntity::PartitionSurface)
        return;
    if (f->geomType() == GEntity::BoundaryLayerSurface)
        return;

    auto settings = MainWindow::getSettings();
    auto show_surfaces = settings->value("visibility/geo/surfaces").toBool();
    auto show_surface_labels = settings->value("visibility/geo/surface_labels").toBool();
    auto curve_width = settings->value("appearance/geo/curve_width").toInt();
    auto sel_curve_width = settings->value("appearance/geo/selected_curve_width").toInt();
    auto enable_lighting = settings->value("appearance/geo/enable_lighting").toBool();
    auto surface_type = settings->value("appearance/geo/surface_display").toInt();
    auto clr_foregnd = settings->value("appearance/clr_foregnd").value<QColor>();
    auto clr_selection = settings->value("appearance/geo/clr_selection").value<QColor>();
    auto clr_hilight0 = settings->value("appearance/geo/clr_hilight0").value<QColor>();
    auto clr_hilight1 = settings->value("appearance/geo/clr_hilight1").value<QColor>();
    auto clr_normals = settings->value("appearance/geo/clr_normals").value<QColor>();
    auto clr_surfaces = settings->value("appearance/geo/clr_surfaces").value<QColor>();
    auto hilight_orphans = settings->value("general/hilight_orphans").toBool();
    auto two_side_lighting = settings->value("appearance/geo/two_side_lighting").toBool();
    // FIXME: pull this from settings when font settings is implemented
    int font_size = 15;
    bool geom_normals = false;
    // FIXME: this should be a user setting
    int vector_type = 6;

    bool select = (this->view->render_mode == View::GMSH_SELECT && f->model() == GModel::current());
    if (select) {
        glPushName(2);
        glPushName(f->tag());
    }

    if (f->getSelection() == HIGHLIGHT) {
        glLineWidth((float) (sel_curve_width / 2.));
        glColorQColor(HIGHLIGHT_CLR);
    }
    else if (f->getSelection() == SELECTED) {
        glLineWidth((float) (sel_curve_width / 2.));
        glColorQColor(clr_selection);
    }
    else {
        glLineWidth((float) (curve_width / 2.));
        if (f->useColor()) {
            unsigned int clr = f->getColor();
            glColor4ubv((GLubyte *) &clr);
        }
        else
            glColorQColor(clr_surfaces);
    }

    if (hilight_orphans) {
        if (f->isOrphan())
            glColorQColor(clr_hilight0);
        else if (f->numRegions() == 1)
            glColorQColor(clr_hilight1);
    }

    if (two_side_lighting)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    else
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if ((show_surfaces || f->getSelection() > 1) && surface_type > 0)
        f->fillVertexArray();

    if (((show_surfaces || f->getSelection() > 1) && surface_type == 0) || show_surface_labels ||
        geom_normals)
        f->buildRepresentationCross();

    if (show_surfaces || f->getSelection() > 1) {
        if (surface_type > 0 && f->va_geom_triangles) {
            bool selected = false;
            if (f->getSelection())
                selected = true;
            _drawVertexArray(f->va_geom_triangles, enable_lighting, selected, clr_selection);
        }
        else {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0F0F);
            for (int dim = 0; dim < 2; dim++) {
                for (std::size_t i = 0; i < f->cross[dim].size(); i++) {
                    if (f->cross[dim][i].size() >= 2) {
                        glBegin(GL_LINE_STRIP);
                        for (std::size_t j = 0; j < f->cross[dim][i].size(); j++) {
                            double x = f->cross[dim][i][j].x();
                            double y = f->cross[dim][i][j].y();
                            double z = f->cross[dim][i][j].z();
                            this->view->transform(x, y, z);
                            glVertex3d(x, y, z);
                        }
                        glEnd();
                    }
                }
            }
            glDisable(GL_LINE_STIPPLE);
        }
    }

    if (f->cross[0].size() && f->cross[0][0].size()) {
        int idx = f->cross[0][0].size() / 2;
        if (show_surface_labels || f->getSelection() > 1) {
            double offset = 0.1 * font_size * this->view->pixel_equiv_x;
            double x = f->cross[0][0][idx].x();
            double y = f->cross[0][0][idx].y();
            double z = f->cross[0][0][idx].z();
            this->view->transform(x, y, z);
            glColorQColor(clr_foregnd);
            this->view->drawEntityLabel(f, x, y, z, offset);
        }

        if (geom_normals) {
            SPoint3 p(f->cross[0][0][idx].x(), f->cross[0][0][idx].y(), f->cross[0][0][idx].z());
            SPoint2 uv = f->parFromPoint(p);
            SVector3 n = f->normal(uv);
            for (int i = 0; i < 3; i++)
                n[i] *= geom_normals * this->view->pixel_equiv_x / this->view->s[i];
            glColorQColor(clr_normals);
            double x = p.x(), y = p.y(), z = p.z();
            this->view->transform(x, y, z);
            this->view->transformTwoForm(n[0], n[1], n[2]);
            this->view->drawVector(vector_type, 0, x, y, z, n[0], n[1], n[2], enable_lighting);
        }
    }

    if (select) {
        glPopName();
        glPopName();
    }
}

View::DrawGRegion::DrawGRegion(View * view) : view(view) {}

void
View::DrawGRegion::operator()(GRegion * rgn)
{
    if (!rgn->getVisibility())
        return;

    auto ctx = CTX::instance();
    auto settings = MainWindow::getSettings();
    auto curve_width = settings->value("appearance/geo/curve_width").toInt();
    auto sel_curve_width = settings->value("appearance/geo/selected_curve_width").toInt();
    auto show_volumes = settings->value("visibility/geo/volumes").toBool();
    auto show_volume_labels = settings->value("visibility/geo/volume_labels").toBool();
    auto enable_lighting = settings->value("appearance/geo/enable_lighting").toBool();
    auto two_side_lighting = settings->value("appearance/geo/two_side_lighting").toBool();
    auto clr_foregnd = settings->value("appearance/clr_foregnd").value<QColor>();
    auto clr_selection = settings->value("appearance/geo/clr_selection").value<QColor>();
    auto volume_repr = ctx->geom.volumeType;
    // FIXME: pull this from settings when font settings is implemented
    int font_size = 15;

    bool select =
        (this->view->render_mode == View::GMSH_SELECT && rgn->model() == GModel::current());
    if (select) {
        glPushName(3);
        glPushName(rgn->tag());
    }

    if (two_side_lighting)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    else
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (rgn->getSelection() == HIGHLIGHT) {
        glLineWidth((float) sel_curve_width);
        glColorQColor(HIGHLIGHT_CLR);
    }
    else if (rgn->getSelection() == SELECTED) {
        glLineWidth((float) sel_curve_width);
        glColorQColor(clr_selection);
    }
    else {
        glLineWidth((float) curve_width);
        unsigned int col = rgn->useColor() ? rgn->getColor() : ctx->color.geom.volume;
        glColor4ubv((GLubyte *) &col);
    }

    const double size = 8.;
    double x = 0., y = 0., z = 0., d = 0.;

    if (show_volumes || show_volume_labels || rgn->getSelection() > 1) {
        SBoundingBox3d bb = rgn->bounds(true); // fast approx if mesh-based
        SPoint3 p = bb.center();
        x = p.x();
        y = p.y();
        z = p.z();
        d = bb.diag() / 50.;
        this->view->transform(x, y, z);
    }

    if (show_volumes || rgn->getSelection() > 1) {
        if (volume_repr == 0) {
            this->view->drawSphere(size, x, y, z, enable_lighting);
        }
        else {
            glBegin(GL_LINE_LOOP);
            glVertex3d(x + d, y, z);
            glVertex3d(x, y + d, z);
            glVertex3d(x - d, y, z);
            glVertex3d(x, y - d, z);
            glEnd();
            glBegin(GL_LINE_LOOP);
            glVertex3d(x + d, y, z);
            glVertex3d(x, y, z + d);
            glVertex3d(x - d, y, z);
            glVertex3d(x, y, z - d);
            glEnd();
            glBegin(GL_LINE_LOOP);
            glVertex3d(x, y + d, z);
            glVertex3d(x, y, z + d);
            glVertex3d(x, y - d, z);
            glVertex3d(x, y, z - d);
            glEnd();
        }
    }

    if (show_volume_labels || rgn->getSelection() > 1) {
        double offset = (1. * size + 0.1 * font_size) * this->view->pixel_equiv_x;
        glColorQColor(clr_foregnd);
        this->view->drawEntityLabel(rgn, x, y, z, offset);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}
