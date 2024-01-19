#include "view.h"
#include "mainwindow.h"
#include <QOpenGLFunctions>

View::View(MainWindow * main_wnd) :
    main_window(main_wnd)
{

}

View::~View() {}

void View::initializeGL()
{
    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void View::resizeGL(int w, int h)
{
    // Update projection matrix and other size related settings:
    this->projection.setToIdentity();
    this->projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
}

void View::paintGL()
{
    // Draw the scene:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT);
}
