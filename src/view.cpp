#include "view.h"
#include "mainwindow.h"
#include "selectothersdialog.h"
#include "gl2ps.h"
#include "GModel.h"
#include "VertexArray.h"
#include "Trackball.h"
#include "StringUtils.h"
#include "PView.h"
#include <QOpenGLFunctions>
#include <QPainter>
#include <QWheelEvent>
#include <QSettings>
#include <QShortcut>
#include "selectioninfowidget.h"

static int
needPolygonOffset()
{
    GModel * m = GModel::current();
    auto settings = MainWindow::getSettings();
    auto show_curves = settings->value("visibility/geom/curves").toBool();
    auto show_surfaces = settings->value("visibility/geom/surfaces").toBool();
    auto show_surface_edges = settings->value("visibility/mesh/2d_edges").toBool();
    auto show_volume_edges = settings->value("visibility/mesh/3d_edges").toBool();

    if (m->getMeshStatus() == 2 && (show_surface_edges || show_curves || show_surfaces))
        return 1;
    if (m->getMeshStatus() == 3 && (show_surface_edges || show_volume_edges))
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

// returns the element at a given position in a vertex array (element pointers
// are not always stored: returning 0 is not an error)
static MElement *
getElement(GEntity * e, int va_type, int index)
{
    switch (va_type) {
    case 2:
        if (e->va_lines && index < e->va_lines->getNumElementPointers())
            return *e->va_lines->getElementPointerArray(index);
        break;
    case 3:
        if (e->va_triangles && index < e->va_triangles->getNumElementPointers())
            return *e->va_triangles->getElementPointerArray(index);
        break;
    }
    return nullptr;
}

View::View(MainWindow * main_wnd) :
    main_window(main_wnd),
    width(0),
    height(0),
    _transform(nullptr),
    is_dragging(false),
    highlighted_entity(nullptr),
    selection_info(nullptr)
{
    setMouseTracking(true);

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

    this->hilight_timer = new QTimer;
    this->hilight_timer->setSingleShot(true);
    connect(this->hilight_timer, &QTimer::timeout, this, &View::onHighlight);

    this->select_others_dlg = new SelectOthersDialog(this);

    this->selection_info = new SelectionInfoWidget(this);
    this->selection_info->setVisible(true);
    this->selection_info->setText("");

    connect(this, &View::selectionChanged, this, &View::onSelectionChanged);
}

View::~View()
{
    invalidateQuadricsAndDisplayLists();
    delete this->hilight_timer;
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

int
View::getWidth() const
{
    return this->width;
}

double
View::getWidthF() const
{
    return (double) this->width;
}

int
View::getHeight() const
{
    return this->height;
}

double
View::getHeightF() const
{
    return (double) this->height;
}

std::array<int, 4>
View::getViewport()
{
    return { 0, 0, HIDPI(this->width), HIDPI(this->height) };
}

std::array<double, 4>
View::getViewportF()
{
    return { 0., 0., HIDPI<double>(this->width), HIDPI<double>(this->height) };
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

void
View::world2Viewport(double xyz[3], double vp[3]) const
{
    GLint viewport[4];
    GLdouble model[16], proj[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    gluProject(xyz[0], xyz[1], xyz[2], model, proj, viewport, &vp[0], &vp[1], &vp[2]);
}

bool
View::isVisible(GModel * m) const
{
    return (this->hidden_models.find(m) == this->hidden_models.end());
}

void
View::setTranslation(const std::array<double, 3> & trans)
{
    this->t = trans;
}

void
View::setScale(const std::array<double, 3> & scale)
{
    this->s = scale;
}

void
View::setEulerAngles(const std::array<double, 3> & angles)
{
    this->r = angles;
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
            gluPickMatrix(HIDPI<GLdouble>(xpick),
                          HIDPI<GLdouble>(this->height - ypick),
                          HIDPI<GLdouble>(wpick),
                          HIDPI<GLdouble>(hpick),
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
    auto ctx = CTX::instance();

    glPushMatrix();
    glLoadIdentity();
    glScaled(this->s[0], this->s[1], this->s[2]);
    glTranslated(this->t[0], this->t[1], this->t[2]);

    for (int i = 0; i < 6; i++) {
        if (ctx->light[i]) {
            GLfloat position[4] = { (GLfloat) ctx->lightPosition[i][0],
                                    (GLfloat) ctx->lightPosition[i][1],
                                    (GLfloat) ctx->lightPosition[i][2],
                                    (GLfloat) ctx->lightPosition[i][3] };
            glLightfv((GLenum) (GL_LIGHT0 + i), GL_POSITION, position);

            GLfloat r = (GLfloat) (ctx->unpackRed(ctx->color.ambientLight[i]) / 255.);
            GLfloat g = (GLfloat) (ctx->unpackGreen(ctx->color.ambientLight[i]) / 255.);
            GLfloat b = (GLfloat) (ctx->unpackBlue(ctx->color.ambientLight[i]) / 255.);
            GLfloat ambient[4] = { r, g, b, 1.0F };
            glLightfv((GLenum) (GL_LIGHT0 + i), GL_AMBIENT, ambient);

            r = (GLfloat) (ctx->unpackRed(ctx->color.diffuseLight[i]) / 255.);
            g = (GLfloat) (ctx->unpackGreen(ctx->color.diffuseLight[i]) / 255.);
            b = (GLfloat) (ctx->unpackBlue(ctx->color.diffuseLight[i]) / 255.);
            GLfloat diffuse[4] = { r, g, b, 1.0F };
            glLightfv((GLenum) (GL_LIGHT0 + i), GL_DIFFUSE, diffuse);

            r = (GLfloat) (ctx->unpackRed(ctx->color.specularLight[i]) / 255.);
            g = (GLfloat) (ctx->unpackGreen(ctx->color.specularLight[i]) / 255.);
            b = (GLfloat) (ctx->unpackBlue(ctx->color.specularLight[i]) / 255.);
            GLfloat specular[4] = { r, g, b, 1.0F };
            glLightfv((GLenum) (GL_LIGHT0 + i), GL_SPECULAR, specular);

            glEnable((GLenum) (GL_LIGHT0 + i));
        }
        else {
            glDisable((GLenum) (GL_LIGHT0 + i));
        }
    }

    glPopMatrix();

    // ambient and diffuse material colors track glColor automatically
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    // "white"-only specular material reflection color
    GLfloat spec[4] = { (GLfloat) ctx->shine, (GLfloat) ctx->shine, (GLfloat) ctx->shine, 1.0F };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    // specular exponent in [0,128] (larger means more "focused"
    // reflection)
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat) ctx->shineExponent);

    glShadeModel(GL_SMOOTH);

    // Normalize the normals automatically. Using glEnable(GL_RESCALE_NORMAL)
    // instead of glEnable(GL_NORMALIZE) (since we initially specify unit normals)
    // is more efficient, but will only work with isotropic scalings (and we allow
    // anistotropic scalings in myZoom...). Note that GL_RESCALE_NORMAL is only
    // available in GL_VERSION_1_2.
#if defined(WIN32)
    glEnable(GL_NORMALIZE);
#else
    glEnable(GL_RESCALE_NORMAL);
#endif

    // lighting is enabled/disabled for each particular primitive later
    glDisable(GL_LIGHTING);
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

    auto settings = MainWindow::getSettings();
    auto quadric_subdivisions = settings->value("appearance/quadric_subdivs").toInt();

    // display list 0 (sphere)
    glNewList(this->display_lists + 0, GL_COMPILE);
    gluSphere(this->quadric, 1., quadric_subdivisions, quadric_subdivisions);
    glEndList();

    // display list 1 (arrow)
    glNewList(this->display_lists + 1, GL_COMPILE);
    glTranslated(0., 0., CTX::instance()->arrowRelStemLength);
    if (CTX::instance()->arrowRelHeadRadius > 0 && CTX::instance()->arrowRelStemLength < 1)
        gluCylinder(this->quadric,
                    CTX::instance()->arrowRelHeadRadius,
                    0.,
                    (1. - CTX::instance()->arrowRelStemLength),
                    quadric_subdivisions,
                    1);
    if (CTX::instance()->arrowRelHeadRadius > CTX::instance()->arrowRelStemRadius)
        gluDisk(this->quadric,
                CTX::instance()->arrowRelStemRadius,
                CTX::instance()->arrowRelHeadRadius,
                quadric_subdivisions,
                1);
    else
        gluDisk(this->quadric,
                CTX::instance()->arrowRelHeadRadius,
                CTX::instance()->arrowRelStemRadius,
                quadric_subdivisions,
                1);
    glTranslated(0., 0., -CTX::instance()->arrowRelStemLength);
    if (CTX::instance()->arrowRelStemRadius > 0 && CTX::instance()->arrowRelStemLength > 0) {
        gluCylinder(this->quadric,
                    CTX::instance()->arrowRelStemRadius,
                    CTX::instance()->arrowRelStemRadius,
                    CTX::instance()->arrowRelStemLength,
                    quadric_subdivisions,
                    1);
        gluDisk(this->quadric, 0, CTX::instance()->arrowRelStemRadius, quadric_subdivisions, 1);
    }
    glEndList();

    // display list 2 (disk)
    glNewList(this->display_lists + 2, GL_COMPILE);
    gluDisk(this->quadric, 0, 1, quadric_subdivisions, 1);
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
    auto settings = MainWindow::getSettings();
    auto show_surface_faces = settings->value("visibility/mesh/2d_faces").toBool();
    auto show_volume_faces = settings->value("visibility/mesh/3d_faces").toBool();

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

    if (ctx->mesh.colorCarousel == 0 || show_volume_faces || show_surface_faces) {
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
    auto settings = MainWindow::getSettings();
    auto show_nodes = settings->value("visibility/mesh/nodes").toBool();
    auto show_node_labels = settings->value("visibility/mesh/node_labels").toBool();
    auto show_surface_faces = settings->value("visibility/mesh/2d_faces").toBool();
    auto show_volume_faces = settings->value("visibility/mesh/3d_faces").toBool();

    if (show_nodes) {
        if (ctx->mesh.nodeType) {
            for (std::size_t i = 0; i < e->mesh_vertices.size(); i++) {
                MVertex * v = e->mesh_vertices[i];
                if (!v->getVisibility())
                    continue;
                if (ctx->mesh.colorCarousel == 0 || show_volume_faces || show_surface_faces) {
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
                if (ctx->mesh.colorCarousel == 0 || show_volume_faces || show_surface_faces) {
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
    if (show_node_labels) {
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

    auto settings = MainWindow::getSettings();
    auto quadric_subdivisions = settings->value("appearance/quadric_subdivs").toInt();

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
    gluCylinder(this->quadric, radius, radius, length, quadric_subdivisions, 1);
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

    auto settings = MainWindow::getSettings();
    auto label_type = settings->value("visibility/geo/label_type").toInt();
    if (label_type == 0) {
        std::vector<std::string> info = SplitString(e->getInfoString(false, true), '\n');
        for (int line = 0; line < (int) info.size(); line++)
            drawString(info[line].c_str(), xx, yy, zz, line);
        return;
    }

    char str[1024];
    if (label_type == 1) {
        snprintf(str, 1024, "%d", e->tag());
    }
    else if (label_type == 2) {
        strcpy(str, "");
        for (std::size_t i = 0; i < e->physicals.size(); i++) {
            char tmp[32];
            if (i)
                strcat(str, ", ");
            snprintf(tmp, 32, "%d", e->physicals[i]);
            strcat(str, tmp);
        }
    }
    else if (label_type == 3) {
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

void
View::drawBox(double xmin,
              double ymin,
              double zmin,
              double xmax,
              double ymax,
              double zmax,
              bool labels)
{
    glBegin(GL_LINE_LOOP);
    glVertex3d(xmin, ymin, zmin);
    glVertex3d(xmax, ymin, zmin);
    glVertex3d(xmax, ymax, zmin);
    glVertex3d(xmin, ymax, zmin);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3d(xmin, ymin, zmax);
    glVertex3d(xmax, ymin, zmax);
    glVertex3d(xmax, ymax, zmax);
    glVertex3d(xmin, ymax, zmax);
    glEnd();
    glBegin(GL_LINES);
    glVertex3d(xmin, ymin, zmin);
    glVertex3d(xmin, ymin, zmax);
    glVertex3d(xmax, ymin, zmin);
    glVertex3d(xmax, ymin, zmax);
    glVertex3d(xmax, ymax, zmin);
    glVertex3d(xmax, ymax, zmax);
    glVertex3d(xmin, ymax, zmin);
    glVertex3d(xmin, ymax, zmax);
    glEnd();
    if (labels) {
        char label[256];
        double offset = 0.3 * CTX::instance()->glFontSize * pixel_equiv_x;
        snprintf(label, 256, "(%g,%g,%g)", xmin, ymin, zmin);
        drawString(label, xmin + offset / s[0], ymin + offset / s[1], zmin + offset / s[2]);
        snprintf(label, 256, "(%g,%g,%g)", xmax, ymax, zmax);
        drawString(label, xmax + offset / s[0], ymax + offset / s[1], zmax + offset / s[2]);
    }
}

namespace {
class point {
public:
    double x, y, z;
    bool valid;
    point() : x(0.), y(0.), z(0.), valid(false) {}
    point(double xi, double yi, double zi) : x(xi), y(yi), z(zi), valid(true) {}
};

class plane {
private:
    double _a, _b, _c, _d;

public:
    plane(double a, double b, double c, double d) : _a(a), _b(b), _c(c), _d(d) {}
    double
    val(point & p)
    {
        return _a * p.x + _b * p.y + _c * p.z + _d;
    };
    point
    intersect(point & p1, point & p2)
    {
        double v1 = val(p1), v2 = val(p2);
        if (fabs(v1) < 1.e-12) {
            if (fabs(v2) < 1.e-12)
                return point();
            else
                return point(p1.x, p1.y, p1.z);
        }
        else if (fabs(v2) < 1.e-12) {
            return point(p2.x, p2.y, p2.z);
        }
        else if (v1 * v2 < 0.) {
            double coef = -v1 / (v2 - v1);
            return point(coef * (p2.x - p1.x) + p1.x,
                         coef * (p2.y - p1.y) + p1.y,
                         coef * (p2.z - p1.z) + p1.z);
        }
        else
            return point();
    }
};
} // namespace

void
View::drawPlaneInBoundingBox(double xmin,
                             double ymin,
                             double zmin,
                             double xmax,
                             double ymax,
                             double zmax,
                             double a,
                             double b,
                             double c,
                             double d,
                             int shade)
{
    plane pl(a, b, c, d);
    point p1(xmin, ymin, zmin), p2(xmax, ymin, zmin);
    point p3(xmax, ymax, zmin), p4(xmin, ymax, zmin);
    point p5(xmin, ymin, zmax), p6(xmax, ymin, zmax);
    point p7(xmax, ymax, zmax), p8(xmin, ymax, zmax);

    point edge[12];
    edge[0] = pl.intersect(p1, p2);
    edge[1] = pl.intersect(p1, p4);
    edge[2] = pl.intersect(p1, p5);
    edge[3] = pl.intersect(p2, p3);
    edge[4] = pl.intersect(p2, p6);
    edge[5] = pl.intersect(p3, p4);
    edge[6] = pl.intersect(p3, p7);
    edge[7] = pl.intersect(p4, p8);
    edge[8] = pl.intersect(p5, p6);
    edge[9] = pl.intersect(p5, p8);
    edge[10] = pl.intersect(p6, p7);
    edge[11] = pl.intersect(p7, p8);

    int face[6][4] = { { 0, 2, 4, 8 },  { 0, 1, 3, 5 },  { 1, 2, 7, 9 },
                       { 3, 4, 6, 10 }, { 5, 6, 7, 11 }, { 8, 9, 10, 11 } };

    double n[3] = { a, b, c }, ll = 50;
    norme(n);
    if (CTX::instance()->arrowRelStemRadius)
        ll = CTX::instance()->lineWidth / CTX::instance()->arrowRelStemRadius;
    n[0] *= ll * pixel_equiv_x / s[0];
    n[1] *= ll * pixel_equiv_x / s[1];
    n[2] *= ll * pixel_equiv_x / s[2];
    double length = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);

    int n_shade = 0;
    point p_shade[24];

    for (int i = 0; i < 6; i++) {
        int nb = 0;
        point p[4];
        for (int j = 0; j < 4; j++) {
            if (edge[face[i][j]].valid == true)
                p[nb++] = edge[face[i][j]];
        }
        if (nb > 1) {
            for (int j = 1; j < nb; j++) {
                double xx[2] = { p[j].x, p[j - 1].x };
                double yy[2] = { p[j].y, p[j - 1].y };
                double zz[2] = { p[j].z, p[j - 1].z };
                drawCylinder(CTX::instance()->lineWidth, xx, yy, zz, 1);
            }
            for (int j = 0; j < nb; j++) {
                drawArrow3D(p[j].x, p[j].y, p[j].z, n[0], n[1], n[2], length, 1);
                if (shade) {
                    p_shade[n_shade].x = p[j].x;
                    p_shade[n_shade].y = p[j].y;
                    p_shade[n_shade].z = p[j].z;
                    n_shade++;
                }
            }
        }
    }

    if (shade) {
        // disable two-side lighting beacuse polygon can overlap itself
        GLboolean twoside;
        glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE, &twoside);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
        glEnable(GL_LIGHTING);
        glBegin(GL_POLYGON);
        glNormal3d(n[0], n[1], n[2]);
        for (int j = 0; j < n_shade; j++) {
            glVertex3d(p_shade[j].x, p_shade[j].y, p_shade[j].z);
        }
        glEnd();
        glDisable(GL_LIGHTING);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, twoside);
    }
}

void
View::zoomToPoint(const QPointF & pt, double zoomFactor)
{
    double wnr[2];
    double prev_s[3];
    double prev_t[3];

    for (int i = 0; i < 3; i++) {
        prev_s[i] = this->s[i];
        prev_t[i] = this->t[i];
    }

    wnr[0] = (this->vxmin + pt.x() / getWidthF() * (this->vxmax - this->vxmin)) / this->s[0] -
             this->t[0] + this->t_init[0] / this->s[0];
    wnr[1] = (this->vymax - pt.y() / getHeightF() * (this->vymax - this->vymin)) / this->s[1] -
             this->t[1] + this->t_init[1] / this->s[1];

    this->s[0] *= zoomFactor;
    this->s[1] = this->s[0];
    this->s[2] = this->s[0];

    // compute the equivalent translation to apply *after* the scaling so that
    // the scaling is done around the point which was clicked
    this->t[0] = prev_t[0] * (prev_s[0] / this->s[0]) - wnr[0] * (1. - (prev_s[0] / this->s[0]));
    this->t[1] = prev_t[1] * (prev_s[1] / this->s[1]) - wnr[1] * (1. - (prev_s[1] / this->s[1]));
}

void
View::wheelEvent(QWheelEvent * event)
{
    double dy = event->pixelDelta().y();
    auto h = getHeightF();
    double fact = (5. * CTX::instance()->zoomFactor * fabs(dy) + h) / h;
    bool direction = (CTX::instance()->mouseInvertZoom) ? (dy <= 0) : (dy > 0);
    fact = (direction ? fact : 1. / fact);
    if (CTX::instance()->camera) {
        this->camera.zoom(fact);
        this->camera.update();
        update();
    }
    else {
        zoomToPoint(event->position(), fact);
        update();
    }
}

void
View::mousePressEvent(QMouseEvent * event)
{
    this->is_dragging = true;
    this->event_btn = event->button();

    if (event->buttons() & (Qt::RightButton | Qt::MiddleButton)) {
        CTX::instance()->drawRotationCenter = 1;
        if (CTX::instance()->fastRedraw) {
            CTX::instance()->mesh.draw = 0;
            CTX::instance()->post.draw = 0;
        }
        update();
    }

    this->_curr.set(this, event->pos());
    this->_click.set(this, event->pos());
    this->_prev.set(this, event->pos());
}

void
View::mouseMoveEvent(QMouseEvent * event)
{
    GModel * m = GModel::current();
    if (this->highlighted_entity)
        this->highlighted_entity->setSelection(NONE);
    for (auto & e : this->selected_entities)
        e->setSelection(SELECTED);
    update();

    if (this->is_dragging)
        mouseDragEvent(event);
    else
        mouseHoverEvent(event);
}

void
View::mouseDragEvent(QMouseEvent * event)
{
    this->_curr.set(this, event->pos());
    double dx = this->_curr.win[0] - this->_prev.win[0];
    double dy = this->_curr.win[1] - this->_prev.win[1];
    if (this->event_btn == Qt::MiddleButton) {
        // panning
        this->t[0] += (this->_curr.wnr[0] - this->_click.wnr[0]);
        this->t[1] += (this->_curr.wnr[1] - this->_click.wnr[1]);
        this->t[2] = 0.;
        update();
    }
    else if (this->event_btn == Qt::RightButton) {
        // rotate
        if (CTX::instance()->useTrackball) {
            auto w = getWidthF();
            auto h = getHeightF();
            addQuaternion((2. * this->_prev.win[0] - w) / w,
                          (h - 2. * this->_prev.win[1]) / h,
                          (2. * this->_curr.win[0] - w) / w,
                          (h - 2. * this->_curr.win[1]) / h);
        }
        else {
            this->r[1] += ((fabs(dx) > fabs(dy)) ? 180. * dx / getWidthF() : 0.);
            this->r[0] += ((fabs(dx) > fabs(dy)) ? 0. : 180. * dy / getHeightF());
        }
        update();
    }
    this->_prev.set(this, event->pos());
}

void
View::mouseHoverEvent(QMouseEvent * event)
{
    // hover
    this->_curr.set(this, event->pos());
    this->hilight_timer->start(100);
}

void
View::mouseReleaseEvent(QMouseEvent * event)
{
    this->is_dragging = false;
    this->_curr.set(this, event->pos());
    if ((this->_curr.win[0] == this->_click.win[0]) &&
        (this->_curr.win[1] == this->_click.win[1]) && this->highlighted_entity) {
        if (this->selected_entities.find(this->highlighted_entity) ==
            this->selected_entities.end()) {
            selectEntity(this->highlighted_entity);
        }
        else {
            this->highlighted_entity->setSelection(HIGHLIGHT);
            this->selected_entities.erase(this->highlighted_entity);
            emit selectionChanged();
        }
    }
    CTX::instance()->drawRotationCenter = 0;
    CTX::instance()->mesh.draw = 1;
    CTX::instance()->post.draw = 1;
    update();
    this->_prev.set(this, event->pos());
}

void
View::selectEntity(GEntity * entity)
{
    this->selected_entities.insert(entity);
    entity->setSelection(SELECTED);
    emit selectionChanged();
}

void
View::onHighlight()
{
    std::vector<SPoint2> points;
    std::vector<PView *> views;
    bool res = select(ENT_ALL,
                      true,
                      CTX::instance()->mouseHoverMeshes,
                      false,
                      (int) _curr.win[0],
                      (int) _curr.win[1],
                      5,
                      5,
                      this->hover_vertices,
                      this->hover_edges,
                      this->hover_faces,
                      this->hover_regions,
                      this->hover_elements,
                      points,
                      views);
    if (res) {
        if (!this->hover_vertices.empty()) {
            this->hover_vertices[0]->setSelection(HIGHLIGHT);
            this->highlighted_entity = this->hover_vertices[0];
        }
        else if (!this->hover_edges.empty()) {
            this->hover_edges[0]->setSelection(HIGHLIGHT);
            this->highlighted_entity = this->hover_edges[0];
        }
        else if (!this->hover_faces.empty()) {
            this->hover_faces[0]->setSelection(HIGHLIGHT);
            this->highlighted_entity = this->hover_faces[0];
        }
        else if (!this->hover_regions.empty()) {
            this->hover_regions[0]->setSelection(HIGHLIGHT);
            this->highlighted_entity = this->hover_regions[0];
        }
        else
            this->highlighted_entity = nullptr;
        update();
    }
    else
        this->highlighted_entity = nullptr;
}

bool
View::select(int type,
             bool multiple,
             bool mesh,
             bool post,
             int x,
             int y,
             int w,
             int h,
             std::vector<GVertex *> & vertices,
             std::vector<GEdge *> & edges,
             std::vector<GFace *> & faces,
             std::vector<GRegion *> & regions,
             std::vector<MElement *> & elements,
             std::vector<SPoint2> & points,
             std::vector<PView *> & views)
{
    vertices.clear();
    edges.clear();
    faces.clear();
    regions.clear();
    elements.clear();
    points.clear();
    views.clear();

    // in our case the selection buffer size is equal to between 5 and 7 times the
    // maximum number of possible hits
    GModel * m = GModel::current();
    int eles = (mesh && CTX::instance()->pickElements) ? 4 * m->getNumMeshElements() : 0;
    int nviews = PView::list.size() * 100;
    int size = 7 * (m->getNumVertices() + m->getNumEdges() + m->getNumFaces() + m->getNumRegions() +
                    eles) +
               nviews;
    if (size == 0)
        return false; // the model is empty, don't bother!

    // allocate selection buffer
    size += 1000; // just to make sure
    GLuint * selection_buffer = new GLuint[size];
    glSelectBuffer(size, selection_buffer);

    // do one rendering pass in select mode
    this->render_mode = GMSH_SELECT;
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushMatrix();

    // 3d stuff
    initProjection(x, y, w, h);
    initPosition(false);
    drawGeom();
    if (mesh)
        drawMesh();
    // if(post) drawPost();
    // drawGraph2D(true);

    auto viewport = getViewportF();
    // 2d stuff
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(HIDPI<GLdouble>(x),
                  viewport[3] - HIDPI<GLdouble>(y),
                  HIDPI<GLdouble>(w),
                  HIDPI<GLdouble>(h),
                  (GLint *) viewport.data());
    // in pixels, so we can draw some 3D glyphs
    glOrtho(viewport[0], viewport[2], viewport[1], viewport[3], HIDPI(-100.), HIDPI(100.));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawGraph2D(false);
    drawText2D();

    glPopMatrix();

    GLint num_hits = glRenderMode(GL_RENDER);
    this->render_mode = GMSH_RENDER;

    if (!num_hits) {
        // no hits
        delete[] selection_buffer;
        return false;
    }
    else if (num_hits < 0) {
        // overflow
        delete[] selection_buffer;
        Msg::Warning("Too many entities selected");
        return false;
    }

    // decode the hits
    std::vector<Hit> hits;
    GLuint * ptr = selection_buffer;
    for (int i = 0; i < num_hits; i++) {
        // in Gmsh 'names' should always be 0, 2 or 4:
        // * names == 0 means that there is nothing on the stack
        // * if names == 2, the first name is the type of the entity (0 for point, 1
        //   for edge, 2 for face or 3 for volume) and the second is the entity
        //   number;
        // * if names == 4, the first name is the type of the entity, the second is
        //   the entity number, the third is the type of vertex array (2 for line, 3
        //   for triangle, 4 for quad) and the fourth is the index of the element in
        //   the vertex array
        GLuint names = *ptr++;
        GLuint mindepth = *ptr++;
        GLuint maxdepth = *ptr++;
        if (names == 2) {
            GLuint depth = maxdepth + 0 * mindepth; // could do something with mindepth
            GLuint type = *ptr++;
            GLuint ient = *ptr++;
            hits.push_back(Hit(type, ient, depth));
        }
        else if (names == 4) {
            GLuint depth = maxdepth + 0 * mindepth; // could do something with mindepth
            GLuint type = *ptr++;
            GLuint ient = *ptr++;
            GLuint type2 = *ptr++;
            GLuint ient2 = *ptr++;
            hits.push_back(Hit(type, ient, depth, type2, ient2));
        }
    }

    delete[] selection_buffer;

    if (!hits.size()) {
        // no entities
        return false;
    }

    // sort hits to get closest entities first
    std::sort(hits.begin(), hits.end(), HitDepthLessThan());

    // filter result: if type == ENT_NONE, return the closest entity of "lowest
    // dimension" (point < line < surface < volume). Otherwise, return the closest
    // entity of type "type"
    GLuint typmin = 10;
    for (std::size_t i = 0; i < hits.size(); i++)
        typmin = std::min(typmin, hits[i].type);

    for (std::size_t i = 0; i < hits.size(); i++) {
        if ((type == ENT_ALL) || (type == ENT_NONE && hits[i].type == typmin) ||
            (type == ENT_POINT && hits[i].type == 0) || (type == ENT_CURVE && hits[i].type == 1) ||
            (type == ENT_SURFACE && hits[i].type == 2) ||
            (type == ENT_VOLUME && hits[i].type == 3)) {
            switch (hits[i].type) {
            case 0: {
                GVertex * v = m->getVertexByTag(hits[i].ient);
                if (!v) {
                    Msg::Error("Problem in point selection processing");
                    return false;
                }
                vertices.push_back(v);
                if (!multiple)
                    return true;
            } break;
            case 1: {
                GEdge * e = m->getEdgeByTag(hits[i].ient);
                if (!e) {
                    Msg::Error("Problem in line selection processing");
                    return false;
                }
                if (hits[i].type2) {
                    MElement * ele = getElement(e, hits[i].type2, hits[i].ient2);
                    if (ele)
                        elements.push_back(ele);
                }
                edges.push_back(e);
                if (!multiple)
                    return true;
            } break;
            case 2: {
                GFace * f = m->getFaceByTag(hits[i].ient);
                if (!f) {
                    Msg::Error("Problem in surface selection processing");
                    return false;
                }
                if (hits[i].type2) {
                    MElement * ele = getElement(f, hits[i].type2, hits[i].ient2);
                    if (ele)
                        elements.push_back(ele);
                }
                faces.push_back(f);
                if (!multiple)
                    return true;
            } break;
            case 3: {
                GRegion * r = m->getRegionByTag(hits[i].ient);
                if (!r) {
                    Msg::Error("Problem in volume selection processing");
                    return false;
                }
                if (hits[i].type2) {
                    MElement * ele = getElement(r, hits[i].type2, hits[i].ient2);
                    if (ele)
                        elements.push_back(ele);
                }
                regions.push_back(r);
                if (!multiple)
                    return true;
            } break;
            case 4: {
                // NOTE: we are not using 2D graphs
                // int tag = hits[i].ient;
                // SPoint2 p = getGraph2dDataPointForTag(tag);
                // points.push_back(p);
                // if (!multiple)
                //     return true;
            } break;
            case 5: {
                // NOTE: we are not using PViews
                // int tag = hits[i].ient;
                // if (tag >= 0 && tag < (int) PView::list.size())
                //     views.push_back(PView::list[tag]);
                // if (!multiple)
                //     return true;
            } break;
            }
        }
    }

    if (vertices.size() || edges.size() || faces.size() || regions.size() || elements.size() ||
        points.size() || views.size())
        return true;
    return false;
}

void
View::onSelectOther()
{
    if (this->select_others_dlg->isVisible()) {
        this->select_others_dlg->hide();
    }
    else {
        std::vector<GEntity *> entities;
        for (auto & e : this->hover_vertices)
            entities.push_back(e);
        for (auto & e : this->hover_edges)
            entities.push_back(e);
        for (auto & e : this->hover_faces)
            entities.push_back(e);
        for (auto & e : this->hover_regions)
            entities.push_back(e);
        this->select_others_dlg->setEntities(entities);

        auto pos = QCursor::pos();
        pos += QPoint(30, 20);
        this->select_others_dlg->move(pos);
        this->select_others_dlg->show();
    }
}

void
View::onDeselectAll()
{
    for (auto & e : this->selected_entities)
        e->setSelection(NONE);
    this->selected_entities.clear();
    emit selectionChanged();
    update();
}

void
View::onSelectionChanged()
{
    QString text;
    if (this->selected_entities.size() == 0) {
        text = " ";
    }
    else if (this->selected_entities.size() == 1) {
        auto ent = *this->selected_entities.begin();
        text = QString("%1 %2").arg(ent->getTypeString().c_str(), QString::number(ent->tag()));
    }
    else {
        text = QString("Selected entities: %1").arg(this->selected_entities.size());
    }

    this->selection_info->setText(text);
    this->selection_info->adjustSize();
    auto geom = this->geometry();
    this->selection_info->move(geom.width() - this->selection_info->width() - 10 - 80,
                               geom.height() - this->selection_info->height() - 5);
    this->update();
}
