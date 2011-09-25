
#include <QApplication>
#include <QDesktopWidget>

#include <QDir>
#include <QFileInfo>

#include "circa.h"

#include "backgroundscript.h"
#include "window.h"
#include "scriptenv.h"

using namespace circa;

// Setup functions that are implemented elsewhere:
void window_setup(Branch* branch);

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

    circa_initialize();
    circa_use_default_filesystem_interface();

    Branch kernel;
    BackgroundScript kernelRunner(&kernel);

    // Install Looseleaf bindings
    load_script_term(&kernel, "src/window.ca");
    window_setup(&kernel);

    include_script(&kernel, "../libs/opengl/opengl.ca");
    include_script(&kernel, "runtime/main.ca");

    Branch* filesBranch = create_branch_unevaluated(&kernel, "files");
    set_files_branch_global(filesBranch);

    if (has_static_errors(&kernel))
        return -1;

    kernelRunner.start();

    int result = app.exec();

    clear_branch(&kernel);
    circa_shutdown();

    return result;
}
