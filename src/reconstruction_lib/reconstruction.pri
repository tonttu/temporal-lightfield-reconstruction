HEADERS += reconstruction/ReconstructionCudaKernels.hpp \
    reconstruction/Reconstruction.hpp
SOURCES += reconstruction/ReconstructionOur.cpp \
    reconstruction/ReconstructionCuda.cpp \
    reconstruction/ReconstructionTreeBuilder.cpp \
    reconstruction/Reconstruction.cpp

OTHER_FILES += reconstruction/ReconstructionCudaKernels.cu
