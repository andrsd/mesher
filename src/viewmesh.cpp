#include "view.h"
#include "GModel.h"
#include "GVertex.h"

template <class T>
static void
drawBarycentricDual(std::vector<T *> & elements)
{
    glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0F0F);
    // gl2psEnable(GL2PS_LINE_STIPPLE);
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
    // gl2psDisable(GL2PS_LINE_STIPPLE);
}

template <class T>
static void
drawVoronoiDual(std::vector<T *> & elements)
{
    glColor4ubv((GLubyte *) &CTX::instance()->color.fg);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0F0F);
    // gl2psEnable(GL2PS_LINE_STIPPLE);
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
    // gl2psDisable(GL2PS_LINE_STIPPLE);
}

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
