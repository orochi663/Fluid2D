# Qt
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
INCLUDE(${QT_USE_FILE})

SET(FLUID2D_LIBRARIES
    ${QT_LIBRARIES}
    CellarWorkbench
    MediaWorkbench
    PropRoom2D
    Scaena
)
    
SET(FLUID2D_INCLUDE_DIRS
    ${FLUID2D_SRC_DIR}
    ${FLUID2D_INSTALL_PREFIX}/include/CellarWorkbench
    ${FLUID2D_INSTALL_PREFIX}/include/MediaWorkbench
    ${FLUID2D_INSTALL_PREFIX}/include/PropRoom2D
    ${FLUID2D_INSTALL_PREFIX}/include/Scaena)
