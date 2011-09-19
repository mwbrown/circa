
#pragma once

#include <QWidget>

class GLWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

protected:
    void keyPressEvent(QKeyEvent *event);
    GLWidget *glWidget;
};
