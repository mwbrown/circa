
INCLUDEPATH += ../src

HEADERS = src/glwidget.h \
          src/window.h

SOURCES = src/glwidget.cpp \
          src/window.cpp \
          src/main.cpp
LIBS += -L../build -lcirca_t

QT += opengl
