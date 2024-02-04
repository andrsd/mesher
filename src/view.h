#pragma once

#include <set>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include "Context.h"
#include "Camera.h"
#include "GEntity.h"
#include "MElement.h"
#include "MLine.h"
#include "MTriangle.h"
#include "MQuadrangle.h"
#include "MElementCut.h"
#include "MHexahedron.h"
#include "MPyramid.h"
#include "MPrism.h"
#include "MTrihedron.h"
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#if defined(HAVE_VISUDEV)
    #define NORMAL_GLTYPE GL_FLOAT
#else
    #define NORMAL_GLTYPE GL_BYTE
#endif

class MainWindow;
class GModel;
class GEntity;
class GVertex;
class GEdge;
class GFace;
class GRegion;
class MVertex;
class VertexArray;
class PView;
class QShortcut;
class SelectOthersDialog;
class SelectionInfoWidget;

unsigned int getColorByEntity(GEntity * e);
bool isElementVisible(MElement * ele);

class View : public QOpenGLWidget {
    Q_OBJECT
public:
    enum RenderMode { GMSH_RENDER = 1, GMSH_SELECT = 2, GMSH_FEEDBACK = 3 };

    enum SelectionType { NONE, HIGHLIGHT = 1, SELECTED = 2 };

    explicit View(MainWindow * main_wnd);
    ~View() override;

    RenderMode renderMode() const;

    void hide(GModel * m);
    void show(GModel * m);
    void showAll();
    bool isVisible(GModel * m) const;

    void setTranslation(const std::array<double, 3> & trans);
    void setScale(const std::array<double, 3> & scale);
    void setEulerAngles(const std::array<double, 3> & angles);
    void setQuaternionFromEulerAngles();
    void selectEntity(GEntity * entity);

    GEntity * getHighlightedEntity() const;

protected:
    class DrawGVertex {
    private:
        View * view;

    public:
        DrawGVertex(View * view);
        void operator()(GVertex * v);
    };

    class DrawGEdge {
    private:
        View * view;

    public:
        DrawGEdge(View * view);
        void operator()(GEdge * e);
    };

    class DrawGFace {
    private:
        View * view;
        void _drawVertexArray(VertexArray * va,
                              bool useNormalArray,
                              int forceColor = 0,
                              const QColor & color = QColor());

    public:
        DrawGFace(View * view);
        void operator()(GFace * f);
    };

    class DrawGRegion {
    private:
        View * view;

    public:
        DrawGRegion(View * view);
        void operator()(GRegion * rgn);
    };

    // GVertex drawing routines
    class DrawMeshGVertex {
    private:
        View * view;

    public:
        DrawMeshGVertex(View * view);
        void operator()(GVertex * v);
    };

    // GEdge drawing routines
    class DrawMeshGEdge {
    private:
        View * view;

    public:
        DrawMeshGEdge(View * view);
        void operator()(GEdge * e);
    };

    // GFace drawing routines
    class DrawMeshGFace {
    private:
        View * view;

    public:
        DrawMeshGFace(View * view);
        void operator()(GFace * f);
    };

    // GRegion drawing routines
    class DrawMeshGRegion {
    private:
        View * view;

    public:
        DrawMeshGRegion(View * view);
        void operator()(GRegion * r);
    };

    template <typename T>
    inline T
    HIDPI(T value) const
    {
        return devicePixelRatio() * value;
    }

    //
    class DrawTransform {
    public:
        DrawTransform() {}
        virtual ~DrawTransform() {}
        virtual void
        transform(double & x, double & y, double & z)
        {
        }
        virtual void
        transformOneForm(double & x, double & y, double & z)
        {
        }
        virtual void
        transformTwoForm(double & x, double & y, double & z)
        {
        }
        virtual void
        setMatrix(double mat[3][3], double tra[3])
        {
        }
    };

    //
    class MousePosition {
    public:
        double win[3]; // window coordinates
        double wnr[3]; // world coordinates BEFORE rotation
        double s[3]; // scaling state when the event was recorded
        double t[3]; // translation state when the event was recorded

        MousePosition()
        {
            for (int i = 0; i < 3; i++)
                win[i] = wnr[i] = s[i] = t[i] = 0.;
        }

        MousePosition(const MousePosition & instance)
        {
            for (int i = 0; i < 3; i++) {
                win[i] = instance.win[i];
                wnr[i] = instance.wnr[i];
                s[i] = instance.s[i];
                t[i] = instance.t[i];
            }
        }
        void
        set(View * view, const QPoint & pos)
        {
            for (int i = 0; i < 3; i++) {
                s[i] = view->s[i];
                t[i] = view->t[i];
            }
            win[0] = (double) pos.x();
            win[1] = (double) pos.y();
            win[2] = 0.;

            wnr[0] = (view->vxmin + win[0] / view->getWidthF() * (view->vxmax - view->vxmin)) /
                         view->s[0] -
                     view->t[0] + view->t_init[0] / view->s[0];
            wnr[1] = (view->vymax - win[1] / view->getHeightF() * (view->vymax - view->vymin)) /
                         view->s[1] -
                     view->t[1] + view->t_init[1] / view->s[1];
            wnr[2] = 0.;
        }

        void
        recenter(View * view)
        {
            // compute the equivalent translation to apply *after* the scaling so that
            // the scaling is done around the point which was clicked:
            view->t[0] = t[0] * (s[0] / view->s[0]) - wnr[0] * (1. - (s[0] / view->s[0]));
            view->t[1] = t[1] * (s[1] / view->s[1]) - wnr[1] * (1. - (s[1] / view->s[1]));
        }
    };

    class Hit {
    public:
        GLuint type, ient, depth, type2, ient2;
        Hit(GLuint t, GLuint i, GLuint d, GLuint t2 = 0, GLuint i2 = 0) :
            type(t),
            ient(i),
            depth(d),
            type2(t2),
            ient2(i2)
        {
        }
    };

    class HitDepthLessThan {
    public:
        bool
        operator()(const Hit & h1, const Hit & h2) const
        {
            return h1.depth < h2.depth;
        }
    };

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void draw3D();
    void initProjection(int xpick = 0, int ypick = 0, int wpick = 0, int hpick = 0);
    void initRenderModel();
    void initPosition(bool save_matrices);
    void invalidateQuadricsAndDisplayLists();
    void createQuadricsAndDisplayLists();
    void buildRotationMatrix();
    void addQuaternion(double p1x, double p1y, double p2x, double p2y);
    void addQuaternionFromAxisAndAngle(double axis[3], double angle);
    void setQuaternion(double q0, double q1, double q2, double q3);
    void setEulerAnglesFromRotationMatrix();
    void drawGeom();

    void drawArrays(GEntity * e,
                    VertexArray * va,
                    GLint type,
                    bool use_normal_array,
                    int force_color = 0,
                    unsigned int color = 0);
    void drawVertexLabel(GEntity * e, MVertex * v, int partition = -1);
    void drawVerticesPerEntity(GEntity * e);
    template <class T>
    void drawVerticesPerElement(GEntity * e, std::vector<T *> & elements);
    void drawMesh();

    void drawAxes();
    void drawGraph2D(bool in_model_coordinates);
    void drawBackground(const std::array<int, 4> & viewport, double clip_near, double clip_far);
    void drawBackgroundGradient();
    void drawBackgroundImage(bool three_d);
    void drawBackgroundGradientVertical();
    void drawBackgroundGradientHorizontal();
    void drawBackgroundGradientRadial();
    void draw2D();
    void drawText2D();
    void drawScales();
    void drawSmallAxes();
    int fix2dCoordinates(double * x, double * y);

    void drawSphere(double size, double x, double y, double z, int light);
    void drawSphere(double R, double x, double y, double z, int n1, int n2, int light);
    void drawCylinder(double width, double * x, double * y, double * z, int light);
    void drawBox(double xmin,
                 double ymin,
                 double zmin,
                 double xmax,
                 double ymax,
                 double zmax,
                 bool labels = true);
    void drawPlaneInBoundingBox(double xmin,
                                double ymin,
                                double zmin,
                                double xmax,
                                double ymax,
                                double zmax,
                                double a,
                                double b,
                                double c,
                                double d,
                                int shade = 0);

    template <class T>
    void drawElementLabels(GEntity * e,
                           std::vector<T *> & elements,
                           int force_color = 0,
                           unsigned int color = 0);
    template <class T>
    void drawNormals(std::vector<T *> & elements);
    template <class T>
    void drawTangents(std::vector<T *> & elements);

    void drawString(const std::string & s,
                    double x,
                    double y,
                    double z,
                    const std::string & font_name,
                    int font_enum,
                    int font_size,
                    int align,
                    int line_num = 0);
    void drawString(const std::string & s, double x, double y, double z, int line_num = 0);
    void
    drawString(const std::string & s, double x, double y, double z, double style, int line_num = 0);
    void drawArrow3D(double x,
                     double y,
                     double z,
                     double dx,
                     double dy,
                     double dz,
                     double length,
                     int light);
    void drawVector(int Type,
                    int Fill,
                    double x,
                    double y,
                    double z,
                    double dx,
                    double dy,
                    double dz,
                    int light);

    void drawEntityLabel(GEntity * e, double x, double y, double z, double offset);

    void viewport2World(double vp[3], double xyz[3]) const;
    void world2Viewport(double xyz[3], double vp[3]) const;
    std::array<int, 4> getViewport();
    std::array<double, 4> getViewportF();
    int getWidth() const;
    double getWidthF() const;
    int getHeight() const;
    double getHeightF() const;

    void renderText(const QVector3D & text_pos_world, const QString & text);

    GLint project(GLdouble objx,
                  GLdouble objy,
                  GLdouble objz,
                  const GLdouble model[16],
                  const GLdouble proj[16],
                  const GLint viewport[4],
                  GLdouble * winx,
                  GLdouble * winy,
                  GLdouble * winz);
    void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);

    void transform(double & x, double & y, double & z);
    void transformOneForm(double & x, double & y, double & z);
    void transformTwoForm(double & x, double & y, double & z);

    void drawAxis(double xmin,
                  double ymin,
                  double zmin,
                  double xmax,
                  double ymax,
                  double zmax,
                  int ntics,
                  int mikado);
    void drawAxes(int mode,
                  double tics[3],
                  std::string format[3],
                  std::string label[3],
                  double bb[6],
                  int mikado,
                  double value_bb[6]);
    void drawAxes(int mode,
                  double tics[3],
                  std::string format[3],
                  std::string label[3],
                  SBoundingBox3d & bb,
                  int mikado,
                  SBoundingBox3d & value_bb);
    int drawTics(int comp,
                 double n,
                 std::string & format,
                 std::string & label,
                 double p1[3],
                 double p2[3],
                 double perp[3],
                 int mikado,
                 double pixelfact,
                 double value_p1[3],
                 double value_p2[3]);

    bool select(int type,
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
                std::vector<PView *> & views);

    void zoomToPoint(const QPointF & pt, double zoomFactor);
    //
    void wheelEvent(QWheelEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void mouseDragEvent(QMouseEvent * event);
    void mouseHoverEvent(QMouseEvent * event);

signals:
    void selectionChanged();

protected slots:
    void onHighlight();
    void onSelectionChanged();

public slots:
    void onSelectOther();
    void onDeselectAll();

protected:
    MainWindow * main_window;

    DrawTransform * _transform;
    GLUquadricObj * quadric;
    GLuint display_lists;
    std::set<GModel *> hidden_models;

    Camera camera;
    // current Euler angles (in degrees)
    std::array<double, 3> r;
    // current translation and scale
    std::array<double, 3> t, s;
    // current quaternion used for "trackball" rotation
    double quaternion[4];
    GLsizei width;
    GLsizei height;
    // current rotation matrix
    double rot[16];
    // initial translation before applying modelview transform
    double t_init[3];
    // current viewport in real coordinates
    double vxmin, vxmax, vymin, vymax;
    // approx equiv model length of a pixel
    double pixel_equiv_x, pixel_equiv_y;
    // the modelview and projection matrix as they were  at the time of the last initPosition() call
    double model[16];
    double proj[16];
    bool draw_rotation_center;
    bool draw_mesh;
    // current rendering mode
    RenderMode render_mode;
    bool is_dragging;
    Qt::MouseButton event_btn;
    MousePosition _click, _curr, _prev;
    QTimer * hilight_timer;
    SelectOthersDialog * select_others_dlg;

    // entities that are found under the mouse cursor when it stops
    std::vector<GVertex *> hover_vertices;
    std::vector<GEdge *> hover_edges;
    std::vector<GFace *> hover_faces;
    std::vector<GRegion *> hover_regions;
    std::vector<MElement *> hover_elements;
    GEntity * highlighted_entity;
    std::set<GEntity *> selected_entities;
    SelectionInfoWidget * selection_info;
};

template <class T>
void
View::drawElementLabels(GEntity * e,
                        std::vector<T *> & elements,
                        int force_color,
                        unsigned int color)
{
    unsigned col = force_color ? color : getColorByEntity(e);
    glColor4ubv((GLubyte *) &col);

    auto ctx = CTX::instance();
    int labelStep = ctx->mesh.labelSampling;
    if (labelStep <= 0)
        labelStep = 1;

    for (std::size_t i = 0; i < elements.size(); i++) {
        MElement * ele = elements[i];
        if (!isElementVisible(ele))
            continue;
        if (i % labelStep == 0) {
            SPoint3 pc = ele->barycenter();
            char str[256];
            if (ctx->mesh.labelType == 4)
                snprintf(str, 256, "(%g,%g,%g)", pc.x(), pc.y(), pc.z());
            else if (ctx->mesh.labelType == 3)
                snprintf(str, 256, "%d", ele->getPartition());
            else if (ctx->mesh.labelType == 2) {
                int np = e->physicals.size();
                int p = np ? e->physicals[np - 1] : 0;
                snprintf(str, 256, "%d", p);
            }
            else if (ctx->mesh.labelType == 1)
                snprintf(str, 256, "%d", e->tag());
            else
                snprintf(str, 256, "%lu", ele->getNum());
            drawString(str, pc.x(), pc.y(), pc.z());
        }
    }
}

template <class T>
void
View::drawVerticesPerElement(GEntity * e, std::vector<T *> & elements)
{
    auto ctx = CTX::instance();
    for (std::size_t i = 0; i < elements.size(); i++) {
        MElement * ele = elements[i];
        for (std::size_t j = 0; j < ele->getNumVertices(); j++) {
            MVertex * v = ele->getVertex(j);
            // FIXME isElementVisible() can be slow: we should also use a
            // vertex array for drawing vertices...
            if (isElementVisible(ele) && v->getVisibility()) {
                if (ctx->mesh.nodes) {
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
                    if (ctx->mesh.nodeType)
                        drawSphere(ctx->mesh.nodeSize, v->x(), v->y(), v->z(), ctx->mesh.light);
                    else {
                        glBegin(GL_POINTS);
                        glVertex3d(v->x(), v->y(), v->z());
                        glEnd();
                    }
                }
                if (ctx->mesh.nodeLabels)
                    drawVertexLabel(v->onWhat() ? v->onWhat() : e, v);
            }
        }
    }
}

template <class T>
void
View::drawNormals(std::vector<T *> & elements)
{
    auto ctx = CTX::instance();
    glColor4ubv((GLubyte *) &ctx->color.mesh.normals);
    for (std::size_t i = 0; i < elements.size(); i++) {
        MElement * ele = elements[i];
        if (!isElementVisible(ele))
            continue;
        SVector3 n = ele->getFace(0).normal();
        for (int j = 0; j < 3; j++)
            n[j] *= ctx->mesh.normals * this->pixel_equiv_x / this->s[j];
        SPoint3 pc = ele->barycenter();
        drawVector(ctx->vectorType, 0, pc.x(), pc.y(), pc.z(), n[0], n[1], n[2], ctx->mesh.light);
    }
}

template <class T>
void
View::drawTangents(std::vector<T *> & elements)
{
    auto ctx = CTX::instance();
    glColor4ubv((GLubyte *) &ctx->color.mesh.tangents);
    for (std::size_t i = 0; i < elements.size(); i++) {
        MElement * ele = elements[i];
        if (!isElementVisible(ele))
            continue;
        SVector3 t = ele->getEdge(0).tangent();
        for (int j = 0; j < 3; j++)
            t[j] *= ctx->mesh.tangents * this->pixel_equiv_x / this->s[j];
        SPoint3 pc = ele->barycenter();
        drawVector(ctx->vectorType, 0, pc.x(), pc.y(), pc.z(), t[0], t[1], t[2], ctx->mesh.light);
    }
}
