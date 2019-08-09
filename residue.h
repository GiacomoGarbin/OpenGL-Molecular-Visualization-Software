#ifndef RESIDUE_H
#define RESIDUE_H

#include <QString>
#include <QVector>
#include <QVector3D>

#include <Eigen/Dense>

#include "utility.h"
using namespace utility;

class Residue
{
public:
    Residue();

    int number;

    // QChar chain;
    QString chain;
    QString name;
    int sequence;

    QVector<int> atoms;

    // QVector<Eigen::Vector3f> RMSDs;
    QVector<float> RMSDs;
    float MinRMSD;
    float MaxRMSD;

    float RMSF;

    QString PackedData();

    friend std::ostream& operator<<(std::ostream& os, const Residue& residue);
};

#endif // RESIDUE_H
