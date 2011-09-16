
#include <QtGui>
#include <QtOpenGL>

#include "glwidget.h"

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
