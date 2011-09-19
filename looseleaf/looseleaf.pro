
INCLUDEPATH += ../src

HEADERS = src/glwidget.h \
          src/scriptenv.h \
          src/window.h

SOURCES = src/glwidget.cpp \
          src/main.cpp \
          src/scriptenv.cpp \
          src/window.cpp

LIBS += -L../build -lcirca_t
CONFIG += debug

QT += opengl
