TEMPLATE = lib
DEFINES += FW_QT
DEFINES += FW_USE_CUDA=1
DEFINES += FW_USE_GLEW=1
CONFIG(debug, debug|release): DEFINES += _DEBUG
QT += opengl
CONFIG += staticlib
INCLUDEPATH += $$PWD/framework
INCLUDEPATH += $$PWD/reconstruction_lib
INCLUDEPATH += $$PWD/framework/3rdparty/glew/include
LIBS += -lGL -lcuda
