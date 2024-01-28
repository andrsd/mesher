#include "view.h"
#include "GModel.h"
#include "GVertex.h"
#include "GEdge.h"
#include "VertexArray.h"

View::DrawGVertex::DrawGVertex(View * view) : view(view) {}

void
View::DrawGVertex::operator()(GVertex * v)
{
    if (!v->getVisibility())
        return;
    if (v->geomType() == GEntity::BoundaryLayerPoint)
        return;

    auto ctx = CTX::instance();

    bool select = (this->view->render_mode == View::GMSH_SELECT && v->model() == GModel::current());
    if (select) {
        glPushName(0);
        glPushName(v->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    double ps = this->view->HIDPI(ctx->geom.pointSize);
    double sps = this->view->HIDPI(ctx->geom.selectedPointSize);

    if (v->getSelection()) {
        glPointSize((float) sps);
        // gl2psPointSize((float)(ctx->geom.selectedPointSize *
        //                        ctx->print.epsPointSizeFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glPointSize((float) ps);
        // gl2psPointSize((float) (ctx->geom.pointSize * ctx->print.epsPointSizeFactor));
        unsigned int col = v->useColor() ? v->getColor() : ctx->color.geom.point;
        glColor4ubv((GLubyte *) &col);
    }

    if (ctx->geom.highlightOrphans) {
        if (v->isOrphan())
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[0]);
        else if (v->numEdges() == 1)
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[1]);
    }

    double x = v->x(), y = v->y(), z = v->z();
    this->view->transform(x, y, z);

    if (ctx->geom.points || v->getSelection() > 1) {
        if (ctx->geom.pointType > 0) {
            if (v->getSelection())
                this->view->drawSphere(sps, x, y, z, ctx->geom.light);
            else
                this->view->drawSphere(ps, x, y, z, ctx->geom.light);
        }
        else {
            glBegin(GL_POINTS);
            glVertex3d(x, y, z);
            glEnd();
        }
    }

    if (CTX::instance()->geom.pointLabels || v->getSelection() > 1) {
        double offset = (0.5 * ps + 0.1 * CTX::instance()->glFontSize) * this->view->pixel_equiv_x;
        if (v->getSelection() > 1)
            glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
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

    auto ctx = CTX::instance();

    bool select = (this->view->render_mode == View::GMSH_SELECT && e->model() == GModel::current());
    if (select) {
        glPushName(1);
        glPushName(e->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (e->getSelection()) {
        glLineWidth((float) ctx->geom.selectedCurveWidth);
        // gl2psLineWidth((float) (ctx->geom.selectedCurveWidth *
        //                         ctx->print.epsLineWidthFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glLineWidth((float) ctx->geom.curveWidth);
        // gl2psLineWidth(
        //     (float) (ctx->geom.curveWidth * ctx->print.epsLineWidthFactor));
        unsigned int col = e->useColor() ? e->getColor() : ctx->color.geom.curve;
        glColor4ubv((GLubyte *) &col);
    }

    if (ctx->geom.highlightOrphans) {
        if (e->isOrphan())
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[0]);
        else if (e->numFaces() == 1)
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[1]);
    }

    Range<double> t_bounds = e->parBounds(0);
    double t_min = t_bounds.low();
    double t_max = t_bounds.high();

    if (ctx->geom.curves || e->getSelection() > 1) {
        int N = e->minimumDrawSegments() + 1;
        if (ctx->geom.curveType > 0) {
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
                this->view->drawCylinder(e->getSelection() ? ctx->geom.selectedCurveWidth
                                                           : ctx->geom.curveWidth,
                                         x,
                                         y,
                                         z,
                                         ctx->geom.light);
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

    if (ctx->geom.curveLabels || e->getSelection() > 1) {
        GPoint p = e->point(t_min + 0.5 * (t_max - t_min));
        double offset =
            (0.5 * ctx->geom.curveWidth + 0.1 * ctx->glFontSize) * this->view->pixel_equiv_x;
        double x = p.x(), y = p.y(), z = p.z();
        this->view->transform(x, y, z);
        if (e->getSelection() > 1)
            glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
        this->view->drawEntityLabel(e, x, y, z, offset);
    }

    if (ctx->geom.tangents) {
        double t = t_min + 0.5 * (t_max - t_min);
        GPoint p = e->point(t);
        SVector3 der = e->firstDer(t);
        der.normalize();
        for (int i = 0; i < 3; i++)
            der[i] *= ctx->geom.tangents * this->view->pixel_equiv_x / this->view->s[i];
        glColor4ubv((GLubyte *) &CTX::instance()->color.geom.tangents);
        double x = p.x(), y = p.y(), z = p.z();
        this->view->transform(x, y, z);
        this->view->transformOneForm(der[0], der[1], der[2]);
        this->view
            ->drawVector(ctx->vectorType, 0, x, y, z, der[0], der[1], der[2], ctx->geom.light);
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
                                  unsigned int color)
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
        glColor4ubv((GLubyte *) &color);
    }
    else {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, va->getColorArray());
        glEnableClientState(GL_COLOR_ARRAY);
    }
    if (CTX::instance()->polygonOffset)
        glEnable(GL_POLYGON_OFFSET_FILL);
    if (CTX::instance()->geom.surfaceType > 1) {
        if (CTX::instance()->geom.lightTwoSide)
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

    auto ctx = CTX::instance();

    bool select = (this->view->render_mode == View::GMSH_SELECT && f->model() == GModel::current());
    if (select) {
        glPushName(2);
        glPushName(f->tag());
    }

    if (f->getSelection()) {
        glLineWidth((float) (ctx->geom.selectedCurveWidth / 2.));
        // gl2psLineWidth((float) (ctx->geom.selectedCurveWidth / 2. *
        // ctx->print.epsLineWidthFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glLineWidth((float) (ctx->geom.curveWidth / 2.));
        // gl2psLineWidth((float) (ctx->geom.curveWidth / 2. * ctx->print.epsLineWidthFactor));
        unsigned int col = f->useColor() ? f->getColor() : ctx->color.geom.surface;
        glColor4ubv((GLubyte *) &col);
    }

    if (ctx->geom.highlightOrphans) {
        if (f->isOrphan())
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[0]);
        else if (f->numRegions() == 1)
            glColor4ubv((GLubyte *) &ctx->color.geom.highlight[1]);
    }

    if (ctx->geom.lightTwoSide)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    else
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if ((ctx->geom.surfaces || f->getSelection() > 1) && ctx->geom.surfaceType > 0)
        f->fillVertexArray();

    if (((ctx->geom.surfaces || f->getSelection() > 1) && ctx->geom.surfaceType == 0) ||
        ctx->geom.surfaceLabels || ctx->geom.normals)
        f->buildRepresentationCross();

    if (ctx->geom.surfaces || f->getSelection() > 1) {
        if (ctx->geom.surfaceType > 0 && f->va_geom_triangles) {
            bool selected = false;
            if (f->getSelection())
                selected = true;
            _drawVertexArray(f->va_geom_triangles,
                             ctx->geom.light,
                             selected,
                             ctx->color.geom.selection);
        }
        else {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0F0F);
            // gl2psEnable(GL2PS_LINE_STIPPLE);
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
            // gl2psDisable(GL2PS_LINE_STIPPLE);
        }
    }

    if (f->cross[0].size() && f->cross[0][0].size()) {
        int idx = f->cross[0][0].size() / 2;
        if (ctx->geom.surfaceLabels || f->getSelection() > 1) {
            double offset = 0.1 * ctx->glFontSize * this->view->pixel_equiv_x;
            double x = f->cross[0][0][idx].x();
            double y = f->cross[0][0][idx].y();
            double z = f->cross[0][0][idx].z();
            this->view->transform(x, y, z);
            if (f->getSelection() > 1)
                glColor4ubv((GLubyte *) &ctx->color.fg);
            this->view->drawEntityLabel(f, x, y, z, offset);
        }

        if (ctx->geom.normals) {
            SPoint3 p(f->cross[0][0][idx].x(), f->cross[0][0][idx].y(), f->cross[0][0][idx].z());
            SPoint2 uv = f->parFromPoint(p);
            SVector3 n = f->normal(uv);
            for (int i = 0; i < 3; i++)
                n[i] *= ctx->geom.normals * this->view->pixel_equiv_x / this->view->s[i];
            glColor4ubv((GLubyte *) &ctx->color.geom.normals);
            double x = p.x(), y = p.y(), z = p.z();
            this->view->transform(x, y, z);
            this->view->transformTwoForm(n[0], n[1], n[2]);
            this->view->drawVector(ctx->vectorType, 0, x, y, z, n[0], n[1], n[2], ctx->geom.light);
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

    bool select =
        (this->view->render_mode == View::GMSH_SELECT && rgn->model() == GModel::current());
    if (select) {
        glPushName(3);
        glPushName(rgn->tag());
    }

    if (ctx->geom.lightTwoSide)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    else
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (rgn->getSelection()) {
        glLineWidth((float) ctx->geom.selectedCurveWidth);
        // gl2psLineWidth((float) (ctx->geom.selectedCurveWidth * ctx->print.epsLineWidthFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glLineWidth((float) ctx->geom.curveWidth);
        // gl2psLineWidth((float) (ctx->geom.curveWidth * ctx->print.epsLineWidthFactor));
        unsigned int col = rgn->useColor() ? rgn->getColor() : ctx->color.geom.volume;
        glColor4ubv((GLubyte *) &col);
    }

    const double size = 8.;
    double x = 0., y = 0., z = 0., d = 0.;

    if (ctx->geom.volumes || ctx->geom.volumeLabels || rgn->getSelection() > 1) {
        SBoundingBox3d bb = rgn->bounds(true); // fast approx if mesh-based
        SPoint3 p = bb.center();
        x = p.x();
        y = p.y();
        z = p.z();
        d = bb.diag() / 50.;
        this->view->transform(x, y, z);
    }

    if (ctx->geom.volumes || rgn->getSelection() > 1) {
        if (ctx->geom.volumeType == 0) {
            this->view->drawSphere(size, x, y, z, ctx->geom.light);
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

    if (ctx->geom.volumeLabels || rgn->getSelection() > 1) {
        double offset = (1. * size + 0.1 * CTX::instance()->glFontSize) * this->view->pixel_equiv_x;
        if (rgn->getSelection() > 1)
            glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
        this->view->drawEntityLabel(rgn, x, y, z, offset);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}