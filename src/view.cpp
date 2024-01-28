#include "view.h"
#include "mainwindow.h"
#include "gl2ps.h"
#include "GModel.h"
#include "VertexArray.h"
#include "Trackball.h"
#include "StringUtils.h"
#include <QOpenGLFunctions>
#include <QPainter>

template <class T>
static void
drawBarycentricDual(std::vector<T *> & elements)
{
    glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0F0F);
    gl2psEnable(GL2PS_LINE_STIPPLE);
    glBegin(GL_LINES);
    for (std::size_t i = 0; i < elements.size(); i++) {
        MElement * ele = elements[i];
        if (!isElementVisible(ele))
            continue;
        SPoint3 pc = ele->barycenter();
        if (ele->getDim() == 2) {
            for (int j = 0; j < ele->getNumEdges(); j++) {
                MEdge e = ele->getEdge(j);
                SPoint3 p = e.barycenter();
                glVertex3d(pc.x(), pc.y(), pc.z());
                glVertex3d(p.x(), p.y(), p.z());
            }
        }
        else if (ele->getDim() == 3) {
            for (int j = 0; j < ele->getNumFaces(); j++) {
                MFace f = ele->getFace(j);
                SPoint3 p = f.barycenter();
                glVertex3d(pc.x(), pc.y(), pc.z());
                glVertex3d(p.x(), p.y(), p.z());
                for (std::size_t k = 0; k < f.getNumVertices(); k++) {
                    MEdge e(f.getVertex(k),
                            (k == f.getNumVertices() - 1) ? f.getVertex(0) : f.getVertex(k + 1));
                    SPoint3 pe = e.barycenter();
                    glVertex3d(p.x(), p.y(), p.z());
                    glVertex3d(pe.x(), pe.y(), pe.z());
                }
            }
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    gl2psDisable(GL2PS_LINE_STIPPLE);
}

template <class T>
static void
drawVoronoiDual(std::vector<T *> & elements)
{
    glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0F0F);
    gl2psEnable(GL2PS_LINE_STIPPLE);
    glBegin(GL_LINES);
    for (std::size_t i = 0; i < elements.size(); i++) {
        T * ele = elements[i];
        if (!isElementVisible(ele))
            continue;
        SPoint3 pc = ele->circumcenter();
        if (ele->getDim() == 2) {
            for (int j = 0; j < ele->getNumEdges(); j++) {
                MEdge e = ele->getEdge(j);
                SVector3 p2p1(e.getVertex(1)->x() - e.getVertex(0)->x(),
                              e.getVertex(1)->y() - e.getVertex(0)->y(),
                              e.getVertex(1)->z() - e.getVertex(0)->z());
                SVector3 pcp1(pc.x() - e.getVertex(0)->x(),
                              pc.y() - e.getVertex(0)->y(),
                              pc.z() - e.getVertex(0)->z());
                double alpha = dot(pcp1, p2p1) / dot(p2p1, p2p1);
                SPoint3 p((1 - alpha) * e.getVertex(0)->x() + alpha * e.getVertex(1)->x(),
                          (1 - alpha) * e.getVertex(0)->y() + alpha * e.getVertex(1)->y(),
                          (1 - alpha) * e.getVertex(0)->z() + alpha * e.getVertex(1)->z());
                glVertex3d(pc.x(), pc.y(), pc.z());
                glVertex3d(p.x(), p.y(), p.z());
            }
        }
        else if (ele->getDim() == 3) {
            for (int j = 0; j < ele->getNumFaces(); j++) {
                MFace f = ele->getFace(j);
                SPoint3 p = f.barycenter();
                glVertex3d(pc.x(), pc.y(), pc.z());
                glVertex3d(p.x(), p.y(), p.z());
                for (std::size_t k = 0; k < f.getNumVertices(); k++) {
                    MEdge e(f.getVertex(k),
                            (k == f.getNumVertices() - 1) ? f.getVertex(0) : f.getVertex(k + 1));
                    SPoint3 pe = e.barycenter();
                    glVertex3d(p.x(), p.y(), p.z());
                    glVertex3d(pe.x(), pe.y(), pe.z());
                }
            }
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    gl2psDisable(GL2PS_LINE_STIPPLE);
}

//

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
        gl2psPointSize((float) (ctx->geom.pointSize * ctx->print.epsPointSizeFactor));
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
        gl2psLineWidth((float) (ctx->geom.selectedCurveWidth / 2. * ctx->print.epsLineWidthFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glLineWidth((float) (ctx->geom.curveWidth / 2.));
        gl2psLineWidth((float) (ctx->geom.curveWidth / 2. * ctx->print.epsLineWidthFactor));
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
            gl2psEnable(GL2PS_LINE_STIPPLE);
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
            gl2psDisable(GL2PS_LINE_STIPPLE);
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
        gl2psLineWidth((float) (ctx->geom.selectedCurveWidth * ctx->print.epsLineWidthFactor));
        glColor4ubv((GLubyte *) &ctx->color.geom.selection);
    }
    else {
        glLineWidth((float) ctx->geom.curveWidth);
        gl2psLineWidth((float) (ctx->geom.curveWidth * ctx->print.epsLineWidthFactor));
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

//

View::DrawMeshGVertex::DrawMeshGVertex(View * view) : view(view) {}

void
View::DrawMeshGVertex::operator()(GVertex * v)
{
    if (!v->getVisibility())
        return;

    auto ctx = CTX::instance();

    bool select = (view->renderMode() == View::GMSH_SELECT && v->model() == GModel::current());
    if (select) {
        glPushName(0);
        glPushName(v->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (ctx->mesh.nodes || ctx->mesh.nodeLabels)
        this->view->drawVerticesPerEntity(v);

    if (select) {
        glPopName();
        glPopName();
    }
}

//

View::DrawMeshGEdge::DrawMeshGEdge(View * view) : view(view) {}

void
View::DrawMeshGEdge::operator()(GEdge * e)
{
    if (!e->getVisibility())
        return;

    auto ctx = CTX::instance();

    bool select =
        (this->view->renderMode() == View::GMSH_SELECT && e->model() == GModel::current());
    if (select) {
        glPushName(1);
        glPushName(e->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (ctx->mesh.lines)
        this->view->drawArrays(e, e->va_lines, GL_LINES, false);

    if (ctx->mesh.lineLabels)
        this->view->drawElementLabels(e, e->lines);

    if (ctx->mesh.nodes || ctx->mesh.nodeLabels) {
        if (e->getAllElementsVisible())
            this->view->drawVerticesPerEntity(e);
        else
            this->view->drawVerticesPerElement(e, e->lines);
    }

    if (ctx->mesh.tangents)
        this->view->drawTangents(e->lines);

    if (select) {
        glPopName();
        glPopName();
    }
}

//

View::DrawMeshGFace::DrawMeshGFace(View * view) : view(view) {}

void
View::DrawMeshGFace::operator()(GFace * f)
{
    if (!f->getVisibility())
        return;

    auto ctx = CTX::instance();

    bool select =
        (this->view->renderMode() == View::GMSH_SELECT && f->model() == GModel::current());
    if (select) {
        glPushName(2);
        glPushName(f->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    this->view->drawArrays(f,
                           f->va_lines,
                           GL_LINES,
                           ctx->mesh.light && ctx->mesh.lightLines,
                           ctx->mesh.surfaceFaces,
                           ctx->color.mesh.line);

    if (ctx->mesh.lightTwoSide)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    this->view->drawArrays(f, f->va_triangles, GL_TRIANGLES, ctx->mesh.light);

    if (ctx->mesh.surfaceLabels) {
        if (ctx->mesh.triangles)
            this->view->drawElementLabels(f,
                                          f->triangles,
                                          ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        if (ctx->mesh.quadrangles)
            this->view->drawElementLabels(f,
                                          f->quadrangles,
                                          ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        this->view->drawElementLabels(f, f->polygons, ctx->mesh.surfaceFaces, ctx->color.mesh.line);
    }

    if (ctx->mesh.nodes || ctx->mesh.nodeLabels) {
        if (f->getAllElementsVisible())
            this->view->drawVerticesPerEntity(f);
        else {
            if (ctx->mesh.triangles)
                this->view->drawVerticesPerElement(f, f->triangles);
            if (ctx->mesh.quadrangles)
                this->view->drawVerticesPerElement(f, f->quadrangles);
            this->view->drawVerticesPerElement(f, f->polygons);
        }
    }

    if (ctx->mesh.normals) {
        if (ctx->mesh.triangles)
            this->view->drawNormals(f->triangles);
        if (ctx->mesh.quadrangles)
            this->view->drawNormals(f->quadrangles);
        this->view->drawNormals(f->polygons);
    }

    if (ctx->mesh.dual) {
        if (ctx->mesh.triangles)
            drawBarycentricDual(f->triangles);
        if (ctx->mesh.quadrangles)
            drawBarycentricDual(f->quadrangles);
        drawBarycentricDual(f->polygons);
    }
    else if (ctx->mesh.voronoi) {
        if (ctx->mesh.triangles)
            drawVoronoiDual(f->triangles);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}

//

View::DrawMeshGRegion::DrawMeshGRegion(View * view) : view(view) {}

void
View::DrawMeshGRegion::operator()(GRegion * rgn)
{
    if (!rgn->getVisibility())
        return;

    auto ctx = CTX::instance();

    bool select =
        (this->view->renderMode() == View::GMSH_SELECT && rgn->model() == GModel::current());
    if (select) {
        glPushName(3);
        glPushName(rgn->tag());
    }

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    this->view->drawArrays(rgn,
                           rgn->va_lines,
                           GL_LINES,
                           ctx->mesh.light && (ctx->mesh.lightLines > 1),
                           ctx->mesh.volumeFaces,
                           ctx->color.mesh.line);

    if (ctx->mesh.lightTwoSide)
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    this->view->drawArrays(rgn, rgn->va_triangles, GL_TRIANGLES, ctx->mesh.light);

    if (ctx->mesh.volumeLabels) {
        if (ctx->mesh.tetrahedra)
            this->view->drawElementLabels(rgn,
                                          rgn->tetrahedra,
                                          ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        if (ctx->mesh.hexahedra)
            this->view->drawElementLabels(rgn,
                                          rgn->hexahedra,
                                          ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        if (ctx->mesh.prisms)
            this->view->drawElementLabels(rgn,
                                          rgn->prisms,
                                          ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        if (ctx->mesh.pyramids)
            this->view->drawElementLabels(rgn,
                                          rgn->pyramids,
                                          ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        if (ctx->mesh.trihedra)
            this->view->drawElementLabels(rgn,
                                          rgn->trihedra,
                                          ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                          ctx->color.mesh.line);
        this->view->drawElementLabels(rgn,
                                      rgn->polyhedra,
                                      ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces,
                                      ctx->color.mesh.line);
    }

    if (ctx->mesh.nodes || ctx->mesh.nodeLabels) {
        if (rgn->getAllElementsVisible())
            this->view->drawVerticesPerEntity(rgn);
        else {
            if (ctx->mesh.tetrahedra)
                this->view->drawVerticesPerElement(rgn, rgn->tetrahedra);
            if (ctx->mesh.hexahedra)
                this->view->drawVerticesPerElement(rgn, rgn->hexahedra);
            if (ctx->mesh.prisms)
                this->view->drawVerticesPerElement(rgn, rgn->prisms);
            if (ctx->mesh.pyramids)
                this->view->drawVerticesPerElement(rgn, rgn->pyramids);
            if (ctx->mesh.trihedra)
                this->view->drawVerticesPerElement(rgn, rgn->trihedra);
            this->view->drawVerticesPerElement(rgn, rgn->polyhedra);
        }
    }

    if (ctx->mesh.dual) {
        if (ctx->mesh.tetrahedra)
            drawBarycentricDual(rgn->tetrahedra);
        if (ctx->mesh.hexahedra)
            drawBarycentricDual(rgn->hexahedra);
        if (ctx->mesh.prisms)
            drawBarycentricDual(rgn->prisms);
        if (ctx->mesh.pyramids)
            drawBarycentricDual(rgn->pyramids);
        if (ctx->mesh.trihedra)
            drawBarycentricDual(rgn->trihedra);
        drawBarycentricDual(rgn->polyhedra);
    }

    if (ctx->mesh.voronoi) {
        if (ctx->mesh.tetrahedra)
            drawVoronoiDual(rgn->tetrahedra);
    }

    if (select) {
        glPopName();
        glPopName();
    }
}

//

static int
needPolygonOffset()
{
    GModel * m = GModel::current();
    auto ctx = CTX::instance();
    if (m->getMeshStatus() == 2 &&
        (ctx->mesh.surfaceEdges || ctx->geom.curves || ctx->geom.surfaces))
        return 1;
    if (m->getMeshStatus() == 3 && (ctx->mesh.surfaceEdges || ctx->mesh.volumeEdges))
        return 1;
    /*
    for (std::size_t i = 0; i < PView::list.size(); i++) {
        PViewOptions * opt = PView::list[i]->getOptions();
        if (opt->visible && opt->showElement)
            return 1;
    }
    */
    return 0;
}

View::View(MainWindow * main_wnd) : main_window(main_wnd), width(0), height(0), _transform(nullptr)
{
    auto ctx = CTX::instance();
    // initialize from temp values in global context
    for (int i = 0; i < 3; i++) {
        this->r[i] = ctx->tmpRotation[i];
        this->t[i] = ctx->tmpTranslation[i];
        this->s[i] = ctx->tmpScale[i];
    }
    for (int i = 0; i < 4; i++) {
        this->quaternion[i] = ctx->tmpQuaternion[i];
    }

    this->render_mode = GMSH_RENDER;
    this->vxmin = this->vymin = this->vxmax = this->vymax = 0.;
    this->pixel_equiv_x = this->pixel_equiv_y = 0.;

    // cannot create it here: needs valid opengl context
    this->quadric = nullptr;
    this->display_lists = 0;
}

View::~View()
{
    invalidateQuadricsAndDisplayLists();
}

View::RenderMode
View::renderMode() const
{
    return this->render_mode;
}

void
View::hide(GModel * m)
{
    this->hidden_models.insert(m);
}

void
View::show(GModel * m)
{
    auto it = this->hidden_models.find(m);
    if (it != this->hidden_models.end())
        this->hidden_models.erase(it);
}

void
View::showAll()
{
    this->hidden_models.clear();
}

// std::array<double, 3>
// View::getScale() const
//{
//     return this->s;
// }
//
// double
// View::getPixelEquivX() const
//{
//     return this->pixel_equiv_x;
// }
//
// double
// View::getPixelEquivY() const
//{
//     return this->pixel_equiv_y;
// }

int
View::getWidth() const
{
    return this->width;
}

int
View::getHeight() const
{
    return this->height;
}

std::array<int, 4>
View::getViewport()
{
    return { 0, 0, HIDPI(this->width), HIDPI(this->height) };
}

void
View::viewport2World(double vp[3], double xyz[3]) const
{
    GLint viewport[4];
    GLdouble model[16], proj[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    gluUnProject(vp[0], vp[1], vp[2], model, proj, viewport, &xyz[0], &xyz[1], &xyz[2]);
}

bool
View::isVisible(GModel * m) const
{
    return (this->hidden_models.find(m) == this->hidden_models.end());
}

void
View::initializeGL()
{
}

void
View::resizeGL(int w, int h)
{
    this->width = w;
    this->height = h;
}

void
View::paintGL()
{
    auto viewport = getViewport();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    auto clr_bg = CTX::instance()->color.bg;
    GLclampf r = CTX::instance()->unpackRed(clr_bg) / 255.;
    GLclampf g = CTX::instance()->unpackGreen(clr_bg) / 255.;
    GLclampf b = CTX::instance()->unpackBlue(clr_bg) / 255.;
    glClearColor(r, g, b, 0.);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    draw3D();
    draw2D();
}

void
View::draw3D()
{
    createQuadricsAndDisplayLists();

    auto ctx = CTX::instance();
    glPolygonOffset((float) ctx->polygonOffsetFactor, (float) ctx->polygonOffsetUnits);
    if (ctx->polygonOffsetFactor || ctx->polygonOffsetUnits)
        ctx->polygonOffset = ctx->polygonOffsetAlways ? 1 : needPolygonOffset();
    else
        ctx->polygonOffset = 0;

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    initProjection();
    initRenderModel();

    if (!ctx->camera)
        initPosition(true);
    drawAxes();
    drawGeom();
    drawBackgroundImage(true);
    drawMesh();
    drawGraph2D(true);
}

void
View::initProjection(int xpick, int ypick, int wpick, int hpick)
{
    auto ctx = CTX::instance();

    double Va = (double) this->height / (double) this->width;
    double Wa = (ctx->max[1] - ctx->min[1]) / (ctx->max[0] - ctx->min[0]);

    // compute the viewport in World coordinates (with margins)
    if (Va > Wa) {
        this->vxmin = ctx->min[0];
        this->vxmax = ctx->max[0];
        this->vymin = 0.5 * (ctx->min[1] + ctx->max[1] - Va * (ctx->max[0] - ctx->min[0]));
        this->vymax = 0.5 * (ctx->min[1] + ctx->max[1] + Va * (ctx->max[0] - ctx->min[0]));
    }
    else {
        this->vxmin = 0.5 * (ctx->min[0] + ctx->max[0] - (ctx->max[1] - ctx->min[1]) / Va);
        this->vxmax = 0.5 * (ctx->min[0] + ctx->max[0] + (ctx->max[1] - ctx->min[1]) / Va);
        this->vymin = ctx->min[1];
        this->vymax = ctx->max[1];
    }
    double fact = ctx->displayBorderFactor;
    double xborder = fact * (this->vxmax - this->vxmin);
    double yborder = fact * (this->vymax - this->vymin);
    this->vxmin -= xborder;
    this->vxmax += xborder;
    this->vymin -= yborder;
    this->vymax += yborder;

    // Put the origin of World coordinates at center of viewport
    // (this is necessary for the scaling to be applied at center of viewport
    // instead of at initial position of center of gravity)
    this->vxmin -= ctx->cg[0];
    this->vxmax -= ctx->cg[0];
    this->vymin -= ctx->cg[1];
    this->vymax -= ctx->cg[1];

    // store what one pixel represents in world coordinates
    this->pixel_equiv_x = (this->vxmax - this->vxmin) / this->width;
    this->pixel_equiv_y = (this->vymax - this->vymin) / this->height;

    // no initial translation of the model
    this->t_init[0] = this->t_init[1] = this->t_init[2] = 0.;

    // set up the near and far clipping planes so that the box is large enough to
    // manipulate the model and zoom, but not too big (otherwise the z-buffer
    // resolution e.g. with Mesa can become insufficient)
    double zmax = std::max(fabs(ctx->min[2]), fabs(ctx->max[2]));
    if (zmax < ctx->lc)
        zmax = ctx->lc;

    // if we use the camera mode
    if (ctx->camera) {
        glDisable(GL_DEPTH_TEST);
        glPushMatrix();
        glLoadIdentity();
        auto w = HIDPI<double>(this->width);
        auto h = HIDPI<double>(this->height);
        double ratio = w / h;
        double dx = 1.5 * std::tan(this->camera.radians) * w * ratio;
        double dy = 1.5 * std::tan(this->camera.radians) * w;
        double dz = -w * 1.25;
        glBegin(GL_QUADS);
        glColor4ubv((GLubyte *) &ctx->color.bg);
        glVertex3i((int) -dx, (int) -dy, (int) dz);
        glVertex3i((int) dx, (int) -dy, (int) dz);
        glColor4ubv((GLubyte *) &ctx->color.bgGrad);
        glVertex3i((int) dx, (int) dy, (int) dz);
        glVertex3i((int) -dx, (int) dy, (int) dz);
        glEnd();
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }
    else {
        double clip_near, clip_far;
        if (ctx->ortho) {
            clip_near = -zmax * this->s[2] * ctx->clipFactor;
            clip_far = -clip_near;
        }
        else {
            clip_near = 0.75 * ctx->clipFactor * zmax;
            clip_far = 75. * ctx->clipFactor * zmax;
        }
        // setup projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // restrict picking to a rectangular region around xpick,ypick
        auto viewport = getViewport();
        if (this->render_mode == GMSH_SELECT)
            gluPickMatrix((GLdouble) xpick,
                          (GLdouble) (this->height - ypick),
                          (GLdouble) wpick,
                          (GLdouble) hpick,
                          (GLint *) viewport.data());

        // draw background if not in selection mode
        if (this->render_mode != GMSH_SELECT)
            drawBackground(viewport, clip_near, clip_far);

        if (ctx->ortho) {
            glOrtho(this->vxmin, this->vxmax, this->vymin, this->vymax, clip_near, clip_far);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }
        else {
            // recenter the model such that the perspective is always at the center of
            // gravity (we should maybe add an option to choose this, as we do for the
            // rotation center)
            this->t_init[0] = ctx->cg[0];
            this->t_init[1] = ctx->cg[1];
            this->vxmin -= this->t_init[0];
            this->vxmax -= this->t_init[0];
            this->vymin -= this->t_init[1];
            this->vymax -= this->t_init[1];
            glFrustum(this->vxmin, this->vxmax, this->vymin, this->vymax, clip_near, clip_far);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            double coef = (clip_far / clip_near) / 3.;
            glTranslated(-coef * this->t_init[0], -coef * this->t_init[1], -coef * clip_near);
            glScaled(coef, coef, coef);
        }
    }
}

void
View::initRenderModel()
{
    // TODO: move drawContext::initRenderModel() here
}

void
View::initPosition(bool save_matrices)
{
    auto ctx = CTX::instance();
    // NB: Those operations are applied to the model in the view coordinates
    // (in opposite order)
    glScaled(this->s[0], this->s[1], this->s[2]);
    glTranslated(this->t[0] - ctx->cg[0], this->t[1] - ctx->cg[1], this->t[2] - ctx->cg[2]);
    if (ctx->rotationCenterCg)
        glTranslated(ctx->cg[0], ctx->cg[1], ctx->cg[2]);
    else
        glTranslated(ctx->rotationCenter[0], ctx->rotationCenter[1], ctx->rotationCenter[2]);

    buildRotationMatrix();
    glMultMatrixd(rot);

    if (ctx->rotationCenterCg)
        glTranslated(-ctx->cg[0], -ctx->cg[1], -ctx->cg[2]);
    else
        glTranslated(-ctx->rotationCenter[0], -ctx->rotationCenter[1], -ctx->rotationCenter[2]);

    // store the projection and modelview matrices at this precise moment (so that
    // we can use them at any later time, even if the context has changed, i.e.,
    // even if we are out of draw())
    if (save_matrices) {
        glGetDoublev(GL_PROJECTION_MATRIX, this->proj);
        glGetDoublev(GL_MODELVIEW_MATRIX, this->model);
    }

    for (int i = 0; i < 6; i++)
        glClipPlane((GLenum) (GL_CLIP_PLANE0 + i), ctx->clipPlane[i]);
}

void
View::invalidateQuadricsAndDisplayLists()
{
    if (this->quadric) {
        gluDeleteQuadric(this->quadric);
        this->quadric = nullptr;
    }
    if (this->display_lists) {
        glDeleteLists(this->display_lists, 3);
        this->display_lists = 0;
    }
}

void
View::createQuadricsAndDisplayLists()
{
    if (!this->quadric)
        this->quadric = gluNewQuadric();
    if (!this->quadric) {
        Msg::Error("Could not create quadric");
        return;
    }

    if (!this->display_lists)
        this->display_lists = glGenLists(3);
    if (!this->display_lists) {
        Msg::Error("Could not generate display lists");
        return;
    }

    // display list 0 (sphere)
    glNewList(this->display_lists + 0, GL_COMPILE);
    gluSphere(this->quadric,
              1.,
              CTX::instance()->quadricSubdivisions,
              CTX::instance()->quadricSubdivisions);
    glEndList();

    // display list 1 (arrow)
    glNewList(this->display_lists + 1, GL_COMPILE);
    glTranslated(0., 0., CTX::instance()->arrowRelStemLength);
    if (CTX::instance()->arrowRelHeadRadius > 0 && CTX::instance()->arrowRelStemLength < 1)
        gluCylinder(this->quadric,
                    CTX::instance()->arrowRelHeadRadius,
                    0.,
                    (1. - CTX::instance()->arrowRelStemLength),
                    CTX::instance()->quadricSubdivisions,
                    1);
    if (CTX::instance()->arrowRelHeadRadius > CTX::instance()->arrowRelStemRadius)
        gluDisk(this->quadric,
                CTX::instance()->arrowRelStemRadius,
                CTX::instance()->arrowRelHeadRadius,
                CTX::instance()->quadricSubdivisions,
                1);
    else
        gluDisk(this->quadric,
                CTX::instance()->arrowRelHeadRadius,
                CTX::instance()->arrowRelStemRadius,
                CTX::instance()->quadricSubdivisions,
                1);
    glTranslated(0., 0., -CTX::instance()->arrowRelStemLength);
    if (CTX::instance()->arrowRelStemRadius > 0 && CTX::instance()->arrowRelStemLength > 0) {
        gluCylinder(this->quadric,
                    CTX::instance()->arrowRelStemRadius,
                    CTX::instance()->arrowRelStemRadius,
                    CTX::instance()->arrowRelStemLength,
                    CTX::instance()->quadricSubdivisions,
                    1);
        gluDisk(this->quadric,
                0,
                CTX::instance()->arrowRelStemRadius,
                CTX::instance()->quadricSubdivisions,
                1);
    }
    glEndList();

    // display list 2 (disk)
    glNewList(this->display_lists + 2, GL_COMPILE);
    gluDisk(this->quadric, 0, 1, CTX::instance()->quadricSubdivisions, 1);
    glEndList();
}

void
View::buildRotationMatrix()
{
    auto ctx = CTX::instance();
    if (ctx->useTrackball) {
        build_rotmatrix(this->rot, this->quaternion);
        setEulerAnglesFromRotationMatrix();
    }
    else {
        double x = this->r[0] * M_PI / 180.;
        double y = this->r[1] * M_PI / 180.;
        double z = this->r[2] * M_PI / 180.;
        double A = cos(x);
        double B = sin(x);
        double C = cos(y);
        double D = sin(y);
        double E = cos(z);
        double F = sin(z);
        double AD = A * D;
        double BD = B * D;
        this->rot[0] = C * E;
        this->rot[1] = BD * E + A * F;
        this->rot[2] = -AD * E + B * F;
        this->rot[3] = 0.;
        this->rot[4] = -C * F;
        this->rot[5] = -BD * F + A * E;
        this->rot[6] = AD * F + B * E;
        this->rot[7] = 0.;
        this->rot[8] = D;
        this->rot[9] = -B * C;
        this->rot[10] = A * C;
        this->rot[11] = 0.;
        this->rot[12] = 0.;
        this->rot[13] = 0.;
        this->rot[14] = 0.;
        this->rot[15] = 1.;
        setQuaternionFromEulerAngles();
    }
}

void
View::addQuaternion(double p1x, double p1y, double p2x, double p2y)
{
    double quat[4];
    trackball(quat, p1x, p1y, p2x, p2y);
    add_quats(quat, this->quaternion, this->quaternion);
    if (CTX::instance()->camera)
        this->camera.rotate(quat);
}

void
View::addQuaternionFromAxisAndAngle(double axis[3], double angle)
{
    double a = angle * M_PI / 180.;
    double quat[4];
    axis_to_quat(axis, a, quat);
    add_quats(quat, this->quaternion, this->quaternion);
}

void
View::setQuaternion(double q0, double q1, double q2, double q3)
{
    this->quaternion[0] = q0;
    this->quaternion[1] = q1;
    this->quaternion[2] = q2;
    this->quaternion[3] = q3;
}

void
View::setQuaternionFromEulerAngles()
{
    double x = this->r[0] * M_PI / 180.;
    double y = this->r[1] * M_PI / 180.;
    double z = this->r[2] * M_PI / 180.;
    double xx[3] = { 1., 0., 0. };
    double yy[3] = { 0., 1., 0. };
    double zz[3] = { 0., 0., 1. };
    double q1[4], q2[4], q3[4], tmp[4];
    axis_to_quat(xx, -x, q1);
    axis_to_quat(yy, -y, q2);
    axis_to_quat(zz, -z, q3);
    add_quats(q1, q2, tmp);
    add_quats(tmp, q3, this->quaternion);
}

void
View::setEulerAnglesFromRotationMatrix()
{
    this->r[1] = asin(this->rot[8]); // Calculate Y-axis angle
    double C = cos(this->r[1]);
    this->r[1] *= 180. / M_PI;
    // Gimball lock?
    if (fabs(C) > 0.005) {
        double tmpx = this->rot[10] / C; // No, so get X-axis angle
        double tmpy = -this->rot[9] / C;
        this->r[0] = atan2(tmpy, tmpx) * 180. / M_PI;
        tmpx = this->rot[0] / C; // Get Z-axis angle
        tmpy = -this->rot[4] / C;
        this->r[2] = atan2(tmpy, tmpx) * 180. / M_PI;
    }
    else {
        // Gimball lock has occurred
        this->r[0] = 0.; // Set X-axis angle to zero
        double tmpx = this->rot[5]; // And calculate Z-axis angle
        double tmpy = this->rot[1];
        this->r[2] = atan2(tmpy, tmpx) * 180. / M_PI;
    }
    // return only positive angles in [0,360]
    if (this->r[0] < 0.)
        this->r[0] += 360.;
    if (this->r[1] < 0.)
        this->r[1] += 360.;
    if (this->r[2] < 0.)
        this->r[2] += 360.;
}

void
View::drawGeom()
{
    auto ctx = CTX::instance();
    if (!ctx->geom.draw)
        return;

    // // draw any transient geometry stuff
    // if (drawGeomTransient)
    //     (*drawGeomTransient)(this);

    for (int i = 0; i < 6; i++)
        if (ctx->geom.clip & (1 << i))
            glEnable((GLenum) (GL_CLIP_PLANE0 + i));
        else
            glDisable((GLenum) (GL_CLIP_PLANE0 + i));

    for (std::size_t i = 0; i < GModel::list.size(); i++) {
        GModel * m = GModel::list[i];
        if (m->getVisibility() && isVisible(m)) {
            std::for_each(m->firstVertex(), m->lastVertex(), DrawGVertex(this));
            std::for_each(m->firstEdge(), m->lastEdge(), DrawGEdge(this));
            std::for_each(m->firstFace(), m->lastFace(), DrawGFace(this));
            std::for_each(m->firstRegion(), m->lastRegion(), DrawGRegion(this));
        }
    }

    for (int i = 0; i < 6; i++)
        glDisable((GLenum) (GL_CLIP_PLANE0 + i));
}

void
View::drawArrays(GEntity * e,
                 VertexArray * va,
                 GLint type,
                 bool use_normal_array,
                 int force_color,
                 unsigned int color)
{
    if (!va || !va->getNumVertices())
        return;

    auto ctx = CTX::instance();
    // If we want to be enable picking of individual elements we need to
    // draw each one separately
    bool select =
        (this->render_mode == GMSH_SELECT && ctx->pickElements && e->model() == GModel::current());
    if (select) {
        if (va->getNumElementPointers() == va->getNumVertices()) {
            for (int i = 0; i < va->getNumVertices(); i += va->getNumVerticesPerElement()) {
                glPushName(va->getNumVerticesPerElement());
                glPushName(i);
                glBegin(type);
                for (int j = 0; j < va->getNumVerticesPerElement(); j++)
                    glVertex3fv(va->getVertexArray(3 * (i + j)));
                glEnd();
                glPopName();
                glPopName();
            }
            return;
        }
    }

    glVertexPointer(3, GL_FLOAT, 0, va->getVertexArray());
    glEnableClientState(GL_VERTEX_ARRAY);

    if (use_normal_array) {
        glEnable(GL_LIGHTING);
        glNormalPointer(NORMAL_GLTYPE, 0, va->getNormalArray());
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    else
        glDisableClientState(GL_NORMAL_ARRAY);

    if (force_color) {
        glDisableClientState(GL_COLOR_ARRAY);
        glColor4ubv((GLubyte *) &color);
    }
    else if (ctx->pickElements || (!e->getSelection() && (ctx->mesh.colorCarousel == 0 ||
                                                          ctx->mesh.colorCarousel == 3))) {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, va->getColorArray());
        glEnableClientState(GL_COLOR_ARRAY);
    }
    else {
        glDisableClientState(GL_COLOR_ARRAY);
        color = getColorByEntity(e);
        glColor4ubv((GLubyte *) &color);
    }

    if (va->getNumVerticesPerElement() > 2 && ctx->polygonOffset)
        glEnable(GL_POLYGON_OFFSET_FILL);

    glDrawArrays(type, 0, va->getNumVertices());

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void
View::drawVertexLabel(GEntity * e, MVertex * v, int partition)
{
    if (!v->getVisibility())
        return;

    auto ctx = CTX::instance();
    int np = e->physicals.size();
    int physical = np ? e->physicals[np - 1] : 0;
    char str[256];
    if (ctx->mesh.labelType == 4)
        snprintf(str, 256, "(%.16g,%.16g,%.16g)", v->x(), v->y(), v->z());
    else if (ctx->mesh.labelType == 3) {
        if (partition < 0)
            snprintf(str, 256, "NA");
        else
            snprintf(str, 256, "%d", partition);
    }
    else if (ctx->mesh.labelType == 2)
        snprintf(str, 256, "%d", physical);
    else if (ctx->mesh.labelType == 1)
        snprintf(str, 256, "%d", e->tag());
    else
        snprintf(str, 256, "%lu", v->getNum());

    if (ctx->mesh.colorCarousel == 0 || ctx->mesh.volumeFaces || ctx->mesh.surfaceFaces) {
        // by element type
        if (v->getPolynomialOrder() > 1)
            glColor4ubv((GLubyte *) &ctx->color.mesh.nodeSup);
        else
            glColor4ubv((GLubyte *) &ctx->color.mesh.node);
    }
    else {
        unsigned int col = getColorByEntity(e);
        glColor4ubv((GLubyte *) &col);
    }
    double offset = (0.5 * ctx->mesh.nodeSize + 0.1 * ctx->glFontSize) * this->pixel_equiv_x;
    drawString(str,
               v->x() + offset / this->s[0],
               v->y() + offset / this->s[1],
               v->z() + offset / this->s[2]);
}

void
View::drawVerticesPerEntity(GEntity * e)
{
    auto ctx = CTX::instance();
    if (ctx->mesh.nodes) {
        if (ctx->mesh.nodeType) {
            for (std::size_t i = 0; i < e->mesh_vertices.size(); i++) {
                MVertex * v = e->mesh_vertices[i];
                if (!v->getVisibility())
                    continue;
                if (ctx->mesh.colorCarousel == 0 || ctx->mesh.volumeFaces ||
                    ctx->mesh.surfaceFaces) {
                    // by element type
                    if (v->getPolynomialOrder() > 1)
                        glColor4ubv((GLubyte *) &ctx->color.mesh.nodeSup);
                    else
                        glColor4ubv((GLubyte *) &ctx->color.mesh.node);
                }
                else {
                    unsigned int col = getColorByEntity(e);
                    glColor4ubv((GLubyte *) &col);
                }
                drawSphere(ctx->mesh.nodeSize, v->x(), v->y(), v->z(), ctx->mesh.light);
            }
        }
        else {
            glBegin(GL_POINTS);
            for (std::size_t i = 0; i < e->mesh_vertices.size(); i++) {
                MVertex * v = e->mesh_vertices[i];
                if (!v->getVisibility())
                    continue;
                if (ctx->mesh.colorCarousel == 0 || ctx->mesh.volumeFaces ||
                    ctx->mesh.surfaceFaces) {
                    // by element type
                    if (v->getPolynomialOrder() > 1)
                        glColor4ubv((GLubyte *) &ctx->color.mesh.nodeSup);
                    else
                        glColor4ubv((GLubyte *) &ctx->color.mesh.node);
                }
                else {
                    unsigned int col = getColorByEntity(e);
                    glColor4ubv((GLubyte *) &col);
                }
                glVertex3d(v->x(), v->y(), v->z());
            }
            glEnd();
        }
    }
    if (ctx->mesh.nodeLabels) {
        int labelStep = ctx->mesh.labelSampling;
        if (labelStep <= 0)
            labelStep = 1;
        for (std::size_t i = 0; i < e->mesh_vertices.size(); i++)
            if (i % labelStep == 0)
                drawVertexLabel(e, e->mesh_vertices[i]);
    }
}

static void
beginFakeTransparency()
{
}

static void
endFakeTransparency()
{
}

void
View::drawMesh()
{
    auto ctx = CTX::instance();

    if (!ctx->mesh.draw)
        return;

    /*
    // make sure to flag any model-dependent post-processing view as
    // changed if the underlying mesh has, before resetting the changed
    // flag
    if(CTX::instance()->mesh.changed) {
        for(std::size_t i = 0; i < GModel::list.size(); i++)
            for(std::size_t j = 0; j < PView::list.size(); j++)
                if(PView::list[j]->getData()->hasModel(GModel::list[i]))
                    PView::list[j]->setChanged(true);
    }
    */

    glPointSize((float) ctx->mesh.nodeSize);
    // gl2psPointSize((float) (ctx->mesh.nodeSize * ctx->print.epsPointSizeFactor));

    glLineWidth((float) ctx->mesh.lineWidth);
    // gl2psLineWidth((float) (ctx->mesh.lineWidth * ctx->print.epsLineWidthFactor));

    if (!ctx->clipWholeElements) {
        for (int i = 0; i < 6; i++)
            if (ctx->mesh.clip & (1 << i))
                glEnable((GLenum) (GL_CLIP_PLANE0 + i));
            else
                glDisable((GLenum) (GL_CLIP_PLANE0 + i));
    }

    for (auto & m : GModel::list) {
        bool changed = m->fillVertexArrays();
        if (changed)
            Msg::Debug("mesh vertex arrays have changed");
        if (m->getVisibility() && isVisible(m)) {
            int status = m->getMeshStatus();
            if (status >= 0)
                std::for_each(m->firstVertex(), m->lastVertex(), DrawMeshGVertex(this));
            if (status >= 1)
                std::for_each(m->firstEdge(), m->lastEdge(), DrawMeshGEdge(this));
            if (status >= 2) {
                beginFakeTransparency();
                std::for_each(m->firstFace(), m->lastFace(), DrawMeshGFace(this));
                endFakeTransparency();
            }
            if (status >= 3)
                std::for_each(m->firstRegion(), m->lastRegion(), DrawMeshGRegion(this));
        }
    }

    ctx->mesh.changed = 0;

    for (int i = 0; i < 6; i++)
        glDisable((GLenum) (GL_CLIP_PLANE0 + i));
}

//

void
View::drawAxes()
{
    // TODO: move drawContext::drawAxes() here
}

void
View::drawGraph2D(bool in_model_coordinates)
{
    // TODO: move drawContext::drawGraph2d here
}

void
View::drawBackground(const std::array<int, 4> & viewport, double clip_near, double clip_far)
{
    auto ctx = CTX::instance();
    if ((ctx->bgGradient || ctx->bgImageFileName.size()) &&
        (!ctx->printing || ctx->print.background)) {
        glDisable(GL_DEPTH_TEST);
        glPushMatrix();
        glLoadIdentity();
        // the z values and the translation are only needed for GL2PS, which does
        // not understand "no depth test" (hence we must make sure that we draw
        // the background behind the rest of the scene)
        glOrtho((double) viewport[0],
                (double) viewport[2],
                (double) viewport[1],
                (double) viewport[3],
                clip_near,
                clip_far);
        glTranslated(0., 0., -0.99 * clip_far);
        drawBackgroundGradient();
        // hack for GL2PS (to make sure that the image is in front of the
        // gradient)
        glTranslated(0., 0., 0.01 * clip_far);
        drawBackgroundImage(false);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }
}

void
View::drawBackgroundGradientVertical()
{
    auto ctx = CTX::instance();
    auto viewport = getViewport();
    glBegin(GL_QUADS);
    glColor4ubv((GLubyte *) &ctx->color.bg);
    glVertex2i(viewport[0], viewport[1]);
    glVertex2i(viewport[2], viewport[1]);
    glColor4ubv((GLubyte *) &ctx->color.bgGrad);
    glVertex2i(viewport[2], viewport[3]);
    glVertex2i(viewport[0], viewport[3]);
    glEnd();
}

void
View::drawBackgroundGradientHorizontal()
{
    auto ctx = CTX::instance();
    auto viewport = getViewport();
    glBegin(GL_QUADS);
    glColor4ubv((GLubyte *) &ctx->color.bg);
    glVertex2i(viewport[2], viewport[1]);
    glVertex2i(viewport[2], viewport[3]);
    glColor4ubv((GLubyte *) &ctx->color.bgGrad);
    glVertex2i(viewport[0], viewport[3]);
    glVertex2i(viewport[0], viewport[1]);
    glEnd();
}

void
View::drawBackgroundGradientRadial()
{
    auto ctx = CTX::instance();
    auto viewport = getViewport();
    double cx = 0.5 * (viewport[0] + viewport[2]);
    double cy = 0.5 * (viewport[1] + viewport[3]);
    double r = 0.5 * std::max(viewport[2] - viewport[0], viewport[3] - viewport[1]);
    glBegin(GL_TRIANGLE_FAN);
    glColor4ubv((GLubyte *) &ctx->color.bgGrad);
    glVertex2d(cx, cy);
    glColor4ubv((GLubyte *) &ctx->color.bg);
    glVertex2d(cx + r, cy);
    int ntheta = 36;
    for (int i = 1; i < ntheta + 1; i++) {
        double theta = i * 2 * M_PI / (double) ntheta;
        glVertex2d(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}

void
View::drawBackgroundGradient()
{
    auto ctx = CTX::instance();
    if (ctx->bgGradient == 0)
        ;
    else if (ctx->bgGradient == 1)
        drawBackgroundGradientVertical();
    else if (ctx->bgGradient == 2)
        drawBackgroundGradientHorizontal();
    else if (ctx->bgGradient == 3)
        drawBackgroundGradientRadial();
}

void
View::drawBackgroundImage(bool three_d)
{
    // TODO: move drawContext::drawBackgroundImage(bool threeD) here
}

void
View::draw2D()
{
    glDisable(GL_DEPTH_TEST);
    for (int i = 0; i < 6; i++)
        glDisable((GLenum) (GL_CLIP_PLANE0 + i));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    auto viewport = getViewport();
    // in pixels, so we can draw some 3D glyphs
    glOrtho((double) viewport[0],
            (double) viewport[2],
            (double) viewport[1],
            (double) viewport[3],
            -100.,
            100.);

    auto ctx = CTX::instance();
    // hack to make the 2D primitives appear "in front" in GL2PS
    glTranslated(0., 0., ctx->clipFactor > 1. ? 1. / ctx->clipFactor : ctx->clipFactor);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    drawGraph2D(false);
    drawText2D();
    if (ctx->post.draw && !ctx->stereo)
        drawScales();
    if (ctx->smallAxes)
        drawSmallAxes();
}

void
View::drawText2D()
{
    // TODO: move drawContext::drawText2d() here
}

void
View::drawScales()
{
    // TODO: move drawContext::drawScales() here
}

void
View::drawSmallAxes()
{
    auto ctx = CTX::instance();

    double l = HIDPI(ctx->smallAxesSize);
    double o = ctx->glFontSize / 5;

    double cx = HIDPI(ctx->smallAxesPos[0]);
    double cy = HIDPI(ctx->smallAxesPos[1]);
    fix2dCoordinates(&cx, &cy);

    double xx, xy, yx, yy, zx, zy;

    if (ctx->camera) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(this->camera.position.x,
                  this->camera.position.y,
                  this->camera.position.z,
                  this->camera.target.x,
                  this->camera.target.y,
                  this->camera.target.z,
                  this->camera.up.x,
                  this->camera.up.y,
                  this->camera.up.z);
        glPushMatrix();
        glPopMatrix();
        float fvViewMatrix[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, fvViewMatrix);
        glLoadIdentity();
        xx = l * fvViewMatrix[0];
        xy = l * fvViewMatrix[1];
        yx = l * fvViewMatrix[4];
        yy = l * fvViewMatrix[5];
        zx = l * fvViewMatrix[8];
        zy = l * fvViewMatrix[9];
    }
    else {
        xx = l * this->rot[0];
        xy = l * this->rot[1];
        yx = l * this->rot[4];
        yy = l * this->rot[5];
        zx = l * this->rot[8];
        zy = l * this->rot[9];
    }
    glLineWidth(HIDPI<float>(ctx->lineWidth));
    gl2psLineWidth(HIDPI<float>(ctx->lineWidth * ctx->print.epsLineWidthFactor));

    GLubyte red[4] = { 188, 39, 26, 0 };
    glColor4ubv(red);
    glBegin(GL_LINES);
    glVertex2d(cx, cy);
    glVertex2d(cx + xx, cy + xy);
    glEnd();
    drawString("X", cx + xx + o, cy + xy + o, 0.);

    GLubyte green[4] = { 65, 147, 41, 0 };
    glColor4ubv(green);
    glBegin(GL_LINES);
    glVertex2d(cx, cy);
    glVertex2d(cx + yx, cy + yy);
    glEnd();
    drawString("Y", cx + yx + o, cy + yy + o, 0.);

    GLubyte blue[4] = { 0, 0, 200, 0 };
    glColor4ubv(blue);
    glBegin(GL_LINES);
    glVertex2d(cx, cy);
    glVertex2d(cx + zx, cy + zy);
    glEnd();
    drawString("Z", cx + zx + o, cy + zy + o, 0.);
}

int
View::fix2dCoordinates(double * x, double * y)
{
    int ret = (*x > 99999 && *y > 99999) ? 3 : (*y > 99999) ? 2 : (*x > 99999) ? 1 : 0;

    auto viewport = getViewport();
    if (*x < 0) // measure from right border
        *x = viewport[2] + *x;
    else if (*x > 99999) // by convention, x-centered
        *x = viewport[2] / 2;

    if (*y < 0) // measure from bottom border
        *y = -(*y);
    else if (*y > 99999) // by convention, y-centered
        *y = viewport[3] / 2.;
    else
        *y = viewport[3] - *y;
    return ret;
}

void
View::drawSphere(double size, double x, double y, double z, int light)
{
    double ss = size * this->pixel_equiv_x / this->s[0]; // size is in pixels
    if (light)
        glEnable(GL_LIGHTING);
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(ss, ss, ss);
    glCallList(this->display_lists + 0);
    glPopMatrix();
    glDisable(GL_LIGHTING);
}

void
View::drawSphere(double R, double x, double y, double z, int n1, int n2, int light)
{
    if (light)
        glEnable(GL_LIGHTING);
    glPushMatrix();
    glTranslated(x, y, z);
    gluSphere(this->quadric, R, n1, n2);
    glPopMatrix();
    glDisable(GL_LIGHTING);
}

void
View::drawCylinder(double width, double * x, double * y, double * z, int light)
{
    if (light)
        glEnable(GL_LIGHTING);

    double dx = x[1] - x[0];
    double dy = y[1] - y[0];
    double dz = z[1] - z[0];
    double const length = std::sqrt(dx * dx + dy * dy + dz * dz);
    double radius = width * this->pixel_equiv_x / s[0];
    double zdir[3] = { 0., 0., 1. };
    double vdir[3] = { dx / length, dy / length, dz / length };
    double axis[3], phi;
    prodve(zdir, vdir, axis);
    double const cosphi = prosca(zdir, vdir);
    if (!norme(axis)) {
        axis[0] = 0.;
        axis[1] = 1.;
        axis[2] = 0.;
    }
    phi = 180. * myacos(cosphi) / M_PI;

    glPushMatrix();
    glTranslated(x[0], y[0], z[0]);
    glRotated(phi, axis[0], axis[1], axis[2]);
    gluCylinder(this->quadric, radius, radius, length, CTX::instance()->quadricSubdivisions, 1);
    glPopMatrix();

    glDisable(GL_LIGHTING);
}

void
View::drawString(const std::string & s,
                 double x,
                 double y,
                 double z,
                 const std::string & font_name,
                 int font_enum,
                 int font_size,
                 int align,
                 int line_num)
{
    if (s.empty())
        return;
    auto ctx = CTX::instance();
    if (ctx->printing && !ctx->print.text)
        return;

    if (s.size() > 8 && s.substr(0, 7) == "file://") {
        // FIXME
        // drawImage(s.substr(7), x, y, z, align);
        return;
    }

    glRasterPos3d(x, y, z);
    GLboolean valid;
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
    if (valid == GL_FALSE)
        return; // the primitive is culled

    if (align > 0 || line_num) {
        // NOTE: need a way to figure out rendered text width and height
        // TODO: implement this
    }

    if (!ctx->printing) {
        auto world_pos = QVector3D(x, y, z);
        this->renderText(world_pos, QString(s.c_str()));
    }
    else {
        // NOTE: rendering text is not implemented for printing
    }
}

void
View::drawString(const std::string & s, double x, double y, double z, int line_num)
{
    auto ctx = CTX::instance();
    drawString(s, x, y, z, ctx->glFont, ctx->glFontEnum, ctx->glFontSize, 0, line_num);
}

void
View::drawString(const std::string & s, double x, double y, double z, double style, int line_num)
{
    unsigned int bits = (unsigned int) style;

    if (!bits) {
        // use defaults
        drawString(s, x, y, z, line_num);
    }
    else {
        int size = (bits & 0xff);
        int font = (bits >> 8 & 0xff);
        int align = (bits >> 16 & 0xff);
        int font_enum = -1;
        std::string font_name = "Helvetica";
        if (!size)
            size = CTX::instance()->glFontSize;
        drawString(s, x, y, z, font_name, font_enum, size, align, line_num);
    }
}

void
View::renderText(const QVector3D & text_pos_world, const QString & text)
{
    GLdouble model[4][4], proj[4][4];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);
    GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

    project(text_pos_world.x(),
            text_pos_world.y(),
            text_pos_world.z(),
            &model[0][0],
            &proj[0][0],
            &view[0],
            &textPosX,
            &textPosY,
            &textPosZ);

    // y is inverted
    textPosY = HIDPI(this->height) - textPosY;

    float curr_clr[4];
    glGetFloatv(GL_CURRENT_COLOR, curr_clr);
    QPainter painter(this);
    painter.setPen(QColor::fromRgbF(curr_clr[0], curr_clr[1], curr_clr[2]));
    // TODO: Allow users to select the font
    painter.setFont(QFont("Helvetica", 12));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(textPosX / devicePixelRatio(), textPosY / devicePixelRatio(), text);
    painter.end();
}

GLint
View::project(GLdouble objx,
              GLdouble objy,
              GLdouble objz,
              const GLdouble model[16],
              const GLdouble proj[16],
              const GLint viewport[4],
              GLdouble * winx,
              GLdouble * winy,
              GLdouble * winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;
    transformPoint(out, model, in);
    transformPoint(in, proj, out);

    if (in[3] == 0.0)
        return GL_FALSE;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

    *winz = (1 + in[2]) / 2;
    return GL_TRUE;
}

void
View::transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row, col) m[col * 4 + row]
    out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
    out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
    out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
    out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

static void
drawSimpleVector(int arrow,
                 int fill,
                 double x,
                 double y,
                 double z,
                 double dx,
                 double dy,
                 double dz,
                 double d,
                 int light)
{
    double n[3], t[3], u[3];

    n[0] = dx / d;
    n[1] = dy / d;
    n[2] = dz / d;

    if ((fabs(n[0]) >= fabs(n[1]) && fabs(n[0]) >= fabs(n[2])) ||
        (fabs(n[1]) >= fabs(n[0]) && fabs(n[1]) >= fabs(n[2]))) {
        t[0] = n[1];
        t[1] = -n[0];
        t[2] = 0.;
    }
    else {
        t[0] = 0.;
        t[1] = n[2];
        t[2] = -n[1];
    }

    double l = sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
    t[0] /= l;
    t[1] /= l;
    t[2] /= l;

    u[0] = n[1] * t[2] - n[2] * t[1];
    u[1] = n[2] * t[0] - n[0] * t[2];
    u[2] = n[0] * t[1] - n[1] * t[0];

    l = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    u[0] /= l;
    u[1] /= l;
    u[2] /= l;

    double b = CTX::instance()->arrowRelHeadRadius * d;

    if (arrow) {
        double f1 = CTX::instance()->arrowRelStemLength;
        double f2 = (1 - 2. * CTX::instance()->arrowRelStemRadius) * f1; // hack :-)

        if (fill) {
            glBegin(GL_LINES);
            glVertex3d(x, y, z);
            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);
            glEnd();

            if (light && fill)
                glEnable(GL_LIGHTING);
            glBegin(GL_TRIANGLES);
            if (light)
                glNormal3dv(u);
            glVertex3d(x + dx, y + dy, z + dz);
            glVertex3d(x + f2 * dx + b * (t[0]),
                       y + f2 * dy + b * (t[1]),
                       z + f2 * dz + b * (t[2]));
            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);

            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);
            glVertex3d(x + f2 * dx + b * (-t[0]),
                       y + f2 * dy + b * (-t[1]),
                       z + f2 * dz + b * (-t[2]));
            glVertex3d(x + dx, y + dy, z + dz);

            if (light)
                glNormal3dv(t);
            glVertex3d(x + dx, y + dy, z + dz);
            glVertex3d(x + f2 * dx + b * (-u[0]),
                       y + f2 * dy + b * (-u[1]),
                       z + f2 * dz + b * (-u[2]));
            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);

            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);
            glVertex3d(x + f2 * dx + b * (u[0]),
                       y + f2 * dy + b * (u[1]),
                       z + f2 * dz + b * (u[2]));
            glVertex3d(x + dx, y + dy, z + dz);
            glEnd();
            glDisable(GL_LIGHTING);
        }
        else {
            glBegin(GL_LINE_STRIP);
            glVertex3d(x, y, z);
            glVertex3d(x + dx, y + dy, z + dz);
            glVertex3d(x + f2 * dx + b * (t[0]),
                       y + f2 * dy + b * (t[1]),
                       z + f2 * dz + b * (t[2]));
            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);
            glVertex3d(x + f2 * dx + b * (-t[0]),
                       y + f2 * dy + b * (-t[1]),
                       z + f2 * dz + b * (-t[2]));
            glVertex3d(x + dx, y + dy, z + dz);
            glVertex3d(x + f2 * dx + b * (-u[0]),
                       y + f2 * dy + b * (-u[1]),
                       z + f2 * dz + b * (-u[2]));
            glVertex3d(x + f1 * dx, y + f1 * dy, z + f1 * dz);
            glVertex3d(x + f2 * dx + b * (u[0]),
                       y + f2 * dy + b * (u[1]),
                       z + f2 * dz + b * (u[2]));
            glVertex3d(x + dx, y + dy, z + dz);
            glEnd();
        }
    }
    else {
        // simple pyramid
        if (fill) {
            double top[3] = { x + dx, y + dy, z + dz };
            double tp[3] = { x + b * t[0], y + b * t[1], z + b * t[2] };
            double tm[3] = { x - b * t[0], y - b * t[1], z - b * t[2] };
            double up[3] = { x + b * u[0], y + b * u[1], z + b * u[2] };
            double um[3] = { x - b * u[0], y - b * u[1], z - b * u[2] };
            double nn[3];

            if (light && fill)
                glEnable(GL_LIGHTING);
            glBegin(GL_TRIANGLES);
            if (light) {
                normal3points(tm[0], tm[1], tm[2], um[0], um[1], um[2], top[0], top[1], top[2], nn);
                glNormal3dv(nn);
            }
            glVertex3d(tm[0], tm[1], tm[2]);
            glVertex3d(um[0], um[1], um[2]);
            glVertex3d(top[0], top[1], top[2]);

            if (light) {
                normal3points(um[0], um[1], um[2], tp[0], tp[1], tp[2], top[0], top[1], top[2], nn);
                glNormal3dv(nn);
            }
            glVertex3d(um[0], um[1], um[2]);
            glVertex3d(tp[0], tp[1], tp[2]);
            glVertex3d(top[0], top[1], top[2]);

            if (light) {
                normal3points(tp[0], tp[1], tp[2], up[0], up[1], up[2], top[0], top[1], top[2], nn);
                glNormal3dv(nn);
            }
            glVertex3d(tp[0], tp[1], tp[2]);
            glVertex3d(up[0], up[1], up[2]);
            glVertex3d(top[0], top[1], top[2]);

            if (light) {
                normal3points(up[0], up[1], up[2], tm[0], tm[1], tm[2], top[0], top[1], top[2], nn);
                glNormal3dv(nn);
            }
            glVertex3d(up[0], up[1], up[2]);
            glVertex3d(tm[0], tm[1], tm[2]);
            glVertex3d(top[0], top[1], top[2]);
            glEnd();
            glDisable(GL_LIGHTING);
        }
        else {
            glBegin(GL_LINE_LOOP);
            glVertex3d(x + b * (t[0]), y + b * (t[1]), z + b * (t[2]));
            glVertex3d(x + b * (-u[0]), y + b * (-u[1]), z + b * (-u[2]));
            glVertex3d(x + b * (-t[0]), y + b * (-t[1]), z + b * (-t[2]));
            glVertex3d(x + b * (u[0]), y + b * (u[1]), z + b * (u[2]));
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(x + b * (t[0]), y + b * (t[1]), z + b * (t[2]));
            glVertex3d(x + dx, y + dy, z + dz);

            glVertex3d(x + b * (-u[0]), y + b * (-u[1]), z + b * (-u[2]));
            glVertex3d(x + dx, y + dy, z + dz);

            glVertex3d(x + b * (-t[0]), y + b * (-t[1]), z + b * (-t[2]));
            glVertex3d(x + dx, y + dy, z + dz);

            glVertex3d(x + b * (u[0]), y + b * (u[1]), z + b * (u[2]));
            glVertex3d(x + dx, y + dy, z + dz);
            glEnd();
        }
    }
}

void
View::drawArrow3D(double x,
                  double y,
                  double z,
                  double dx,
                  double dy,
                  double dz,
                  double length,
                  int light)
{
    double zdir[3] = { 0., 0., 1. };
    double vdir[3] = { dx / length, dy / length, dz / length };
    double axis[3];
    prodve(zdir, vdir, axis);
    double const cosphi = prosca(zdir, vdir);
    if (!norme(axis)) {
        axis[0] = 0.;
        axis[1] = 1.;
        axis[2] = 0.;
    }
    double phi = 180. * myacos(cosphi) / M_PI;

    if (light)
        glEnable(GL_LIGHTING);
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(length, length, length);
    glRotated(phi, axis[0], axis[1], axis[2]);
    glCallList(this->display_lists + 1);
    glPopMatrix();
    glDisable(GL_LIGHTING);
}

void
View::drawVector(int Type,
                 int Fill,
                 double x,
                 double y,
                 double z,
                 double dx,
                 double dy,
                 double dz,
                 int light)
{
    double length = sqrt(dx * dx + dy * dy + dz * dz);

    if (length == 0.0)
        return;

    switch (Type) {
    case 1:
        glBegin(GL_LINES);
        glVertex3d(x, y, z);
        glVertex3d(x + dx, y + dy, z + dz);
        glEnd();
        break;
    case 6:
        if (CTX::instance()->arrowRelHeadRadius) {
            glBegin(GL_POINTS);
            glVertex3d(x + dx, y + dy, z + dz);
            glEnd();
        }
        glBegin(GL_LINES);
        glVertex3d(x + dx, y + dy, z + dz);
        // color gradient
        glColor4ubv((GLubyte *) &CTX::instance()->color.bg);
        glVertex3d(x, y, z);
        glEnd();
        break;
    case 2:
        drawSimpleVector(1, Fill, x, y, z, dx, dy, dz, length, light);
        break;
    case 3:
        drawSimpleVector(0, Fill, x, y, z, dx, dy, dz, length, light);
        break;
    case 4:
    default:
        drawArrow3D(x, y, z, dx, dy, dz, length, light);
        break;
    }
}

void
View::drawEntityLabel(GEntity * e, double x, double y, double z, double offset)
{
    double xx = x + offset / this->s[0];
    double yy = y + offset / this->s[1];
    double zz = z + offset / this->s[2];

    auto ctx = CTX::instance();

    if (ctx->geom.labelType == 0) {
        std::vector<std::string> info = SplitString(e->getInfoString(false, true), '\n');
        for (int line = 0; line < (int) info.size(); line++)
            drawString(info[line].c_str(), xx, yy, zz, line);
        return;
    }

    char str[1024];
    if (ctx->geom.labelType == 1) {
        snprintf(str, 1024, "%d", e->tag());
    }
    else if (ctx->geom.labelType == 2) {
        strcpy(str, "");
        for (std::size_t i = 0; i < e->physicals.size(); i++) {
            char tmp[32];
            if (i)
                strcat(str, ", ");
            snprintf(tmp, 32, "%d", e->physicals[i]);
            strcat(str, tmp);
        }
    }
    else if (ctx->geom.labelType == 3) {
        strcpy(str, e->model()->getElementaryName(e->dim(), e->tag()).c_str());
    }
    else {
        strcpy(str, "");
        std::string name = "";
        for (std::size_t i = 0; i < e->physicals.size(); i++) {
            if (name.size())
                strcat(str, ", ");
            name = e->model()->getPhysicalName(e->dim(), std::abs(e->physicals[i]));
            if (name.size())
                strcat(str, name.c_str());
        }
    }

    drawString(str, xx, yy, zz);
}

void
View::transform(double & x, double & y, double & z)
{
    if (this->_transform)
        this->_transform->transform(x, y, z);
}

void
View::transformOneForm(double & x, double & y, double & z)
{
    if (this->_transform)
        this->_transform->transformOneForm(x, y, z);
}

void
View::transformTwoForm(double & x, double & y, double & z)
{
    if (this->_transform)
        this->_transform->transformTwoForm(x, y, z);
}
