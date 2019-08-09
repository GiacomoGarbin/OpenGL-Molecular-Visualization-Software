#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T20:17:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FinalProject
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    openglwidget.cpp \
    trajectory.cpp \
    atom.cpp \
    residue.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    trajectory.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    trajectory.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    trajectory.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    trajectory.cpp \
    residueswindow.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    residueswindow.cpp \
    trajectory.cpp \
    atom.cpp \
    main.cpp \
    mainwindow.cpp \
    openglwidget.cpp \
    residue.cpp \
    residueswindow.cpp \
    trajectory.cpp

HEADERS += \
        mainwindow.h \
    openglwidget.h \
    trajectory.h \
    atom.h \
    residue.h \
    utility.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    trajectory.h \
    utility.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    trajectory.h \
    utility.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    trajectory.h \
    utility.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    trajectory.h \
    utility.h \
    residueswindow.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    residueswindow.h \
    trajectory.h \
    utility.h \
    atom.h \
    mainwindow.h \
    openglwidget.h \
    residue.h \
    residueswindow.h \
    trajectory.h \
    utility.h

FORMS += \
        mainwindow.ui \
    residueswindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += ../../eigen-eigen-323c052e1731

DISTFILES += \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/outline_frag.glsl \
    shaders/outline_geom.glsl \
    shaders/outline_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/lighting_frag.glsl \
    shaders/lighting_geom.glsl \
    shaders/lighting_vert.glsl \
    shaders/outline_frag.glsl \
    shaders/outline_geom.glsl \
    shaders/outline_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/lighting_frag.glsl \
    shaders/lighting_geom.glsl \
    shaders/lighting_vert.glsl \
    shaders/outline_frag.glsl \
    shaders/outline_geom.glsl \
    shaders/outline_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/SSAO_occlusion_frag.glsl \
    shaders/SSAO_occlusion_geom.glsl \
    shaders/SSAO_occlusion_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/lighting_frag.glsl \
    shaders/lighting_geom.glsl \
    shaders/lighting_vert.glsl \
    shaders/outline_frag.glsl \
    shaders/outline_geom.glsl \
    shaders/outline_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/SSAO_blur_frag.glsl \
    shaders/SSAO_blur_geom.glsl \
    shaders/SSAO_blur_vert.glsl \
    shaders/SSAO_occlusion_frag.glsl \
    shaders/SSAO_occlusion_geom.glsl \
    shaders/SSAO_occlusion_vert.glsl \
    shaders/impostor_frag.glsl \
    shaders/impostor_geom.glsl \
    shaders/impostor_vert.glsl \
    shaders/lighting_frag.glsl \
    shaders/lighting_geom.glsl \
    shaders/lighting_vert.glsl \
    shaders/marker_frag.glsl \
    shaders/marker_vert.glsl \
    shaders/outline_frag.glsl \
    shaders/outline_geom.glsl \
    shaders/outline_vert.glsl \
    shaders/point_frag.glsl \
    shaders/point_vert.glsl \
    shaders/quad_frag.glsl \
    shaders/quad_geom.glsl \
    shaders/quad_vert.glsl \
    shaders/silhouette_frag.glsl \
    shaders/silhouette_geom.glsl \
    shaders/silhouette_vert.glsl \
    shaders/SSAO_blur_frag.glsl \
    shaders/SSAO_blur_geom.glsl \
    shaders/SSAO_blur_vert.glsl \
    shaders/SSAO_occlusion_frag.glsl \
    shaders/SSAO_occlusion_geom.glsl \
    shaders/SSAO_occlusion_vert.glsl
