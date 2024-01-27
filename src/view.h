#pragma once

#include <QOpenGLWidget>
#include <QMatrix4x4>
#include "Camera.h"
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

class MainWindow;

class View : public QOpenGLWidget {
public:
    explicit View(MainWindow * main_wnd);
    ~View() override;

protected:
    template <typename T>
    inline T
    HIDPI(T value) const
    {
        return devicePixelRatio() * value;
    }

    enum RenderMode { GMSH_RENDER = 1, GMSH_SELECT = 2, GMSH_FEEDBACK = 3 };

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
    void setQuaternionFromEulerAngles();
    void setEulerAnglesFromRotationMatrix();
    void drawGeom();
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

    void viewport2World(double vp[3], double xyz[3]) const;
    std::array<int, 4> getViewport();
    int getWidth() const;
    int getHeight() const;

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

    MainWindow * main_window;

    GLUquadricObj * quadric;
    GLuint display_lists;

    Camera camera;
    // current Euler angles (in degrees)
    double r[3];
    // current translation and scale
    double t[3], s[3];
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
    // current rendering mode
    RenderMode render_mode;
};
