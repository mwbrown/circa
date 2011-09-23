
#include <QApplication>
#include <QDesktopWidget>

#include <QDir>
#include <QFileInfo>

#include "circa.h"

#include "window.h"
#include "scriptenv.h"

using namespace circa;

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

        if (steps++ > 100) {
            printf("Couldn't find 'runtime' directory\n");
            return -1;
        }
    } 

    initialize_script_env();

    Window rootWindow;
    Branch* mainScript = rootWindow.loadScript("runtime/main.ca");

    if (circa::print_static_errors_formatted(*mainScript, std::cout))
        return -1;

    rootWindow.tick();

    int result = app.exec();

    destroy_script_env();

    return result;
}
