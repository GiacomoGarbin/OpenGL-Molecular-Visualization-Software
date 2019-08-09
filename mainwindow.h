#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include <QtMath>
#include <QDebug>
#include <QPushButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>

#include "residueswindow.h"

#include "utility.h"
using namespace utility;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;

    OpenGLWidget *gl;

    void InitUI();
    void AtomGroup();
    void ResidueGroup();
    void TrajectoryGroup();
    // void progress();
    void OutlineGroup();
    void playback();

    void ColorLerp();

    void SSAOTab();
    void LightTab();
    void MaterialTab();

    void framerate();

    ResiduesWindow window;
    void ResiduesWindowInit();
};

#endif // MAINWINDOW_H
