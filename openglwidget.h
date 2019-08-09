#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <iostream>
#include <fstream>
#include <sstream>

#include <QtMath>
#include <QDebug>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMouseEvent>
// #include <QRandomGenerator>
#include <QTime>

#include "trajectory.h"
#include "utility.h"
using namespace utility;

#define MAX_PITCH 89.0f
#define MIN_RADIUS 1.0f
#define MAX_RADIUS 1000.0f

class OpenGLWidget : public QOpenGLWidget, QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent);

    // background color
    QColor BackgroundColor;

    // trajectory
    Trajectory trajectory;
    GLsizei PointsCount;

    PlaybackData playback;
    FrameRateData FrameRate;

    // SSAO and lighting
    ScreenSpaceAmbientOcclusion SSAO;
    Light light;

    // material
    QVector4D factors = {-30, -70, -90, 100};
    float shininess = 8;

    // outline
    OutlineData outline;
    void SetOutlineColor(bool flag = false);

    // texture toggles
    bool KernelTextureFlag = false;
    bool NoiseTextureFlag = false;
    bool OutlinelTextureFlag = false;
    void ToggleKernelTexture();
    void ToggleNoiseTexture();
    void ToggleOutlinelTexture();

    // test drawing mode
    bool TestDrawingMode = false;
    QVector<int> samplers
    {
        TextureIndex::CENTER,
        TextureIndex::NORMAL,
        TextureIndex::ALBEDO,
        TextureIndex::COLOUR,
        TextureIndex::SSAO_OCCLUSION,
        TextureIndex::SSAO_BLUR,
        TextureIndex::SILHOUETTE
    };
    int SamplerIndex = 4;

    // shift key status
    bool ShiftDown;

protected:
    // gl
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // mouse events
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    // clear color
    void SetClearColor(QColor color);

    QVector<GLuint> programs;
    GLuint createProgram(std::string vertPath, std::string geomPath, std::string fragPath);
    GLuint createShader(std::string source, GLenum type);
    GLuint addProgram(int index);
    void removeProgram(int index);

    QVector<GLuint> VAOs;
    GLuint addVAO(int index);
    void removeVAO(int index);

    QVector<GLuint> FBOs;
    GLuint addFBO(int index);

    QVector<GLuint> textures;
    GLuint addTexture(int index);

    QVector<GLuint> RBOs;
    GLuint addRBO(int index);

    // MVP matrix
    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 projection;

    CameraData camera;

    MouseEventData mouse;

    // drawing
    void DrawPoints();
    void DrawImpostors();

    void DrawOcclusion();
    void DrawBlur();
    void DrawLighting();

    void DrawSilhouette();
    void DrawOutline();

    void DrawLightMarker();

    void DrawQuad();

signals:
    // void FrameRateSignal(float fps);
    void FrameRateSignal();
    void NextStepSignal();
    void MouseHoverSignal(Atom atom);
    void MouseNotHoverSignal();
    void ColorSchemeSignal(bool flag);
};

#endif // OPENGLWIDGET_H
