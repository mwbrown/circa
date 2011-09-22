
#pragma once

#include <QTimer>
#include <QWidget>
#include <QGLWidget>

#include "scriptenv.h"

class GLWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();
    void setHidden();
    void tick();
    void loadScript(const char* filename);

protected:
    void keyPressEvent(QKeyEvent *event);
    GLWidget *glWidget;
    QTimer updateTimer;
};

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent = 0);
    ~GLWidget();

    ScriptEnv scriptEnv;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

};