#ifndef RESIDUESWINDOW_H
#define RESIDUESWINDOW_H

#include <QDebug>
#include <QWidget>
#include <QGridLayout>

#include "residue.h"

// #include "trajectory.h"
// #include "utility.h"
// using namespace utility;

namespace Ui {
class ResiduesWindow;
}

class ResiduesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ResiduesWindow(QWidget *parent = nullptr);
    ~ResiduesWindow();

    // Trajectory *trajectory;

    QMap<int, Residue> residues;
    void InitUI();

private:
    Ui::ResiduesWindow *ui;
};

#endif // RESIDUESWINDOW_H
