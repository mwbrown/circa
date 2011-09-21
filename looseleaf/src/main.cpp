
#include <QApplication>
#include <QDesktopWidget>

#include <QDir>
#include <QFileInfo>

#include "window.h"
#include "scriptenv.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Walk upwards till we find the "runtime" directory. This way things will work
    // fine when we are launched from an app bundle.
    int steps = 0;
    while (!QFileInfo("runtime").exists()) {

        QDir parent = QDir::current();
        parent.cdUp();

        QDir::setCurrent(parent.path());

        if (steps++ > 100)
            break;
    } 

    initialize_script_env();

    Window rootWindow;
    rootWindow.loadScript("runtime/main.ca");
    rootWindow.tick();

    int result = app.exec();

    destroy_script_env();

    return result;
}
