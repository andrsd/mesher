#include "view.h"
#include "mainwindow.h"
#include "gl2ps.h"
#include "Context.h"
#include "GModel.h"
#include "Trackball.h"
#include <QOpenGLFunctions>
#include <QPainter>

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

View::View(MainWindow * main_wnd) : main_window(main_wnd), width(0), height(0)
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
    // TODO: move drawContext::drawGeom() here
}

void
View::drawMesh()
{
    // TODO: move drawContext::drawMesh() here
}

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
