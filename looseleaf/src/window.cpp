// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <QtGui>
#include <QtOpenGL>

#include "circa.h"

#include "MouseState.h"
#include "Window.h"

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

    // Place a pointer to this Window in the EvalContext, so that the script can
    // access us.
    // TODO: This should use a Window handle, it doesn't b/c this object is owned
    // elsewhere.
    set_opaque_pointer(glWidget->scriptEnv.context.inputArguments.append(), this);

}
void Window::tick()
{
    glWidget->updateGL();
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

CA_FUNCTION(env__window_size)
{
    Window* window = get_opaque_pointer(CONTEXT->inputArguments[0]);
    QSize size = window->size();
    set_point(OUTPUT, size.x, size.y);
}

CA_FUNCTION(env__mouse)
{
    Window* window = get_opaque_pointer(CONTEXT->inputArguments[0]);
    MouseState* mouseState = &window->glWidget->mouseState;
    set_point(OUTPUT, mouseState->x, mouseState->y);
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

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);

    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_CULL_FACE);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scriptEnv.tick();
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
     
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1000.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
}
