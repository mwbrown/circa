
#include <QApplication>
#include <QDesktopWidget>

#include "window.h"
#include "scriptenv.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    initialize_script_env();

    int result = app.exec();

    destroy_script_env();

    return result;
}
