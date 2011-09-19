// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <QtGui>

#include "circa.h"

#include "glwidget.h"
#include "window.h"

using namespace circa;

circa::HandleWrapper<Window> window_t;

Window::Window()
{
    glWidget = new GLWidget;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);
    setLayout(mainLayout);

    setWindowTitle("Looseleaf");
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
    Window* window = new Window();
    window->show();
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
