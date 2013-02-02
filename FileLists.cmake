SET(FLUID2D_HEADERS
    ${FLUID2D_SRC_DIR}/FluidCharacter.h)
    
SET(FLUID2D_SOURCES
    ${FLUID2D_SRC_DIR}/FluidCharacter.cpp
    ${FLUID2D_SRC_DIR}/main.cpp)
    
SET(FLUID2D_SRC_FILES
    ${FLUID2D_HEADERS}
    ${FLUID2D_SOURCES})
    
SET(FLUID2D_CONFIG_FILES
    ${FLUID2D_SRC_DIR}/CMakeLists.txt
    ${FLUID2D_SRC_DIR}/FileLists.cmake
    ${FLUID2D_SRC_DIR}/LibLists.cmake)