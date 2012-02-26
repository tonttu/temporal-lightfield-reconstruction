HEADERS += gpu/Buffer.hpp \
    gpu/CudaModule.hpp \
    gpu/CudaCompiler.hpp \
    gpu/GLContext.hpp
SOURCES += gpu/Buffer.cpp \
    gpu/CudaModule.cpp \
    gpu/CudaCompiler.cpp \
    gpu/GLContext.cpp
LIBS += -lGL
