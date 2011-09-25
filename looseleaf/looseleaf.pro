
INCLUDEPATH += ../src

HEADERS = src/ScriptEnv.h \
          src/Window.h \
          src/BackgroundScript.h \
          src/MouseState.h

SOURCES = src/main.cpp \
          src/ScriptEnv.cpp \
          src/Window.cpp

LIBS += -L../build -lcirca_d
CONFIG += debug

QT += opengl
