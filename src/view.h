#pragma once

#include <QOpenGLWidget>
#include <QMatrix4x4>

class MainWindow;

class View : public QOpenGLWidget {
public:
    explicit View(MainWindow * main_wnd);
    ~View() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    MainWindow * main_window;
    QMatrix4x4 projection;
};
