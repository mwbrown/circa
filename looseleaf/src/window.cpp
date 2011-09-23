// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <QtGui>
#include <QtOpenGL>

#include "circa.h"

#include "window.h"

using namespace circa;

circa::HandleWrapper<Window> window_t;

const int updateInterval = 16;

Window::Window()
{
    glWidget = new GLWidget;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);
    setLayout(mainLayout);

    QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(tick()));
    updateTimer.start(updateInterval);

    setWindowTitle("Looseleaf");
}
void Window::tick()
{
    glWidget->updateGL();
    glWidget->scriptEnv.tick();
}
circa::Branch* Window::loadScript(const char* filename)
{
    return glWidget->scriptEnv.loadScript(filename);
}

void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

CA_FUNCTION(create_window)
{
    std::cout << "create_window: " << STRING_INPUT(0) << std::endl;

    Window* window = new Window();
    window->show();
    window->loadScript(STRING_INPUT(0));
    window_t.set(OUTPUT, window);
}

CA_FUNCTION(Window__resize)
{
    float x,y;
    get_point(INPUT(1), &x, &y);
    window_t.get(INPUT(0))->resize(QSize(x,y));
}

void window_setup(Branch* branch)
{
    window_t.initialize(branch, "Window");
    install_function(branch, create_window, "create_window");
    install_function(branch, Window__resize, "Window.resize");
}

GLWidget::GLWidget(QWidget* parent)
  : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
    //qglClearColor();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);

    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
}

void GLWidget::resizeGL()
{
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
}
