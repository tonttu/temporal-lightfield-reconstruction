include(../src.pri)

TEMPLATE = app

HEADERS += App.hpp
SOURCES += App.cpp

LIBS += -L../framework -L../reconstruction_lib -lreconstruction_lib -lframework -lGL

linux-*:PRE_TARGETDEPS += ../*/*.a
